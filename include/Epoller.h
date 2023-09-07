#ifndef TOTOROSERVER_EPOLLER_H
#define TOTOROSERVER_EPOLLER_H

#include <sys/epoll.h>      /* epoll */
#include <fcntl.h>          /* fcntl */
#include "Connection.h"     /* Connection */
#include "Pool.h"           /* Pool */
#include "ThreadPool.h"     /* ThreadPool */

static const std::string EpollerChan = "Epoller";
namespace totoro {
    /**
     * @brief 负责连接资源获取、事件监听与分发 \n Responsible for connecting resource acquisition, event monitoring, and distribution
     * @tparam T Connection类或继承自Connection类 \n Connection or derived from Connection.
     */
    template<class T = Connection>
    class Epoller {
        EpollID id                                                      {BAD_FILE_DESCRIPTOR};
        epoll_event events[4096]                                        {};
        bool edgeTriggle                                                {false};
        bool oneShot                                                    {true};
        bool noneBlock                                                  {true};
        bool& isStop                                                    ;
        Pool<T> connectionPool                                          {1024};
        ThreadPool<T> threadPool                                        {4,8196};
        std::atomic<int> currentConnectCount                            {0};
        std::unordered_map<SocketID ,std::pair<sockaddr_in,sockaddr_in>> candidateMap;
        std::unordered_map<SocketID ,std::shared_ptr<T>> connectionMap  ;

        void NoneBlock(SocketID socketId) {
            int option = fcntl(socketId,F_GETFL);
            int newOption = option | O_NONBLOCK;
            fcntl(socketId,newOption);
        }
        int EpollAdd(SocketID socketId,bool isListen = false) {
            epoll_event ev{};
            ev.data.fd = socketId;
            ev.events = EPOLLIN | EPOLLRDHUP;
            ev.events = (edgeTriggle ? ev.events | EPOLLET : ev.events);
            ev.events = (oneShot && !isListen ? ev.events | EPOLLONESHOT : ev.events);
            if(noneBlock) NoneBlock(socketId);
            return epoll_ctl(id,EPOLL_CTL_ADD,socketId,&ev);
        }
        int EpollMod(SocketID socketId, uint32_t ev) {
            epoll_event event{};
            event.data.fd = socketId;
            event.events = ev | EPOLLRDHUP;
            event.events = edgeTriggle ? event.events | EPOLLET : event.events;
            event.events = oneShot ? event.events | EPOLLONESHOT : event.events;
            return epoll_ctl(id,EPOLL_CTL_MOD,socketId,&event);
        }
        int EpollDel(SocketID socketId) {
            return epoll_ctl(id,EPOLL_CTL_DEL,socketId,nullptr);
        }

        bool getMapIterator(SocketID cur,typename std::unordered_map<int,std::shared_ptr<T>>::iterator& mapIterator){
            if((mapIterator = connectionMap.find(cur)) == connectionMap.end()){
                std::shared_ptr<T> conn;
                typename std::unordered_map<SocketID,std::pair<sockaddr_in,sockaddr_in>>::iterator iter;
                if(!connectionPool.acquire(conn)){
                    close(cur);
                    LOG_ERROR(EpollerChan,"connection pool acquire failed");
                    return false;
                }
                if((iter = candidateMap.find(cur)) == candidateMap.end()){
                    close(cur);
                    LOG_ERROR(EpollerChan,"not found candidate addr");
                    return false;
                }
                int ret = conn->Init(cur,iter->second.first,iter->second.second,id,edgeTriggle,oneShot);
                if(ret == -2){
                    conn->Close();
                    connectionPool.release(conn);
                    return false;
                }
                candidateMap.erase(iter);
                std::pair<typename std::unordered_map<SocketID,std::shared_ptr<T>>::iterator,bool> result;
                if(ret > 0){
                    if(EpollAdd(ret) < 0){
                        LOG_ERROR(EpollerChan,"epoll add new failed");
                        conn->Close();
                        connectionPool.release(conn);
                        return false;
                    }
                    result = connectionMap.insert({ret,conn});
                    if(!result.second){
                        LOG_ERROR(EpollerChan,"connectionMap already have key");
                        conn->Close();
                        connectionPool.release(conn);
                        return false;
                    }
                }
                result = connectionMap.insert({cur,conn});
                if(!result.second){
                    LOG_ERROR(EpollerChan,"connectionMap already have key");
                    conn->Close();
                    connectionPool.release(conn);
                    return false;
                }
                currentConnectCount++;
                mapIterator = result.first;
            }
            return true;
        }

        void poll(int timeOut){
            int ret,index,cur;
            TCPSocket tcpSocket{};
            typename std::unordered_map<int,std::shared_ptr<T>>::iterator mapIterator;
            while(!isStop){
                if((ret = epoll_wait(id,events,4096,timeOut)) < 0){
                    if(errno == EINTR) continue;
                    LOG_ERROR(EpollerChan,"epoll wait failed");
                    exit(-1);
                }
                for(index = 0;index < ret;index++ ){
                    cur = events[index].data.fd;
                    if(!getMapIterator(cur,mapIterator)) continue;
                    auto& connection = mapIterator->second;
                    connection->SetWorkSock(cur);
                    auto event = events[index].events;
                    if(event & EPOLLRDHUP){
                        LOG_INFO(EpollerChan,std::to_string(cur));
                        LOG_INFO(EpollerChan,"client close");
                        DelConnection(connection);
                    }else if(event & EPOLLERR){
                        LOG_ERROR(EpollerChan,"epoll error");
                        DelConnection(connection);
                    }else if(event & EPOLLIN){
                        connection->RegisterNextEvent(cur,Connection::Read,false);
                        threadPool.Add(connection);
                    }else if(event & EPOLLOUT){
                        connection->RegisterNextEvent(cur,Connection::Write,false);
                        threadPool.Add(connection);
                    }else{
                        LOG_ERROR(EpollerChan,"unknown error");
                        DelConnection(connection);
                    }
                }
            }
        }
    public:
        explicit Epoller(bool& _isStop,bool _et = false,
                bool _oneShot = true,bool _noneBlock = false):
        isStop(_isStop),edgeTriggle(_et),
        oneShot(_oneShot),noneBlock(_noneBlock){
            if((id =epoll_create(1234)) < 0){
                LOG_ERROR(EpollerChan,"create epoll failed");
                exit(-1);
            }
        }

        ~Epoller(){
            close(id);
        }

        bool AddConnection(TCPSocket& tcpSocket){
            if(EpollAdd(tcpSocket.Sock()) < 0){
                return false;
            }
            candidateMap[tcpSocket.Sock()] = {tcpSocket.Addr(),tcpSocket.DestAddr()};
            return true;
        };

        void DelConnection(std::shared_ptr<T>& conn){
            EpollDel(conn->Sock());
            int sock = conn->Sock();
            int ret = conn->Close();
            connectionPool.release(conn);
            connectionMap.erase(sock);
            if(ret >= 0) {
                EpollDel(ret);
                connectionMap.erase(ret);
            }
            currentConnectCount--;
        };

        static void Poll(Epoller<T>* _epoller,int timeOut){
            auto& epoller = *_epoller;
            epoller.poll(timeOut);
        }

        int CurrentConnectionCount() const {
            return currentConnectCount;
        }
    };

} // totoro

#endif //TOTOROSERVER_EPOLLER_H
