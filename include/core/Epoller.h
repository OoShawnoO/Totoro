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
    protected:
        using ConnectionMap = std::unordered_map<SocketID ,std::shared_ptr<T>>;
        using ConnectionMapIterator = typename ConnectionMap::iterator;
        using ForwardCandidateMapIterator = typename ForwardCandidateMap::iterator;
        using InsertConnectionMapResult = std::pair<ConnectionMapIterator,bool>;

        EpollID id                                                      {BAD_FILE_DESCRIPTOR};
        epoll_event events[4096]                                        {};
        bool edgeTriggle                                                {false};
        bool oneShot                                                    {true};
        bool noneBlock                                                  {true};
        bool& isStop                                                    ;
        IPFilter* filter                                                {nullptr};
        Pool<T> connectionPool                                          {1024};
        ThreadPool<T> threadPool                                        {4,8196};
        std::atomic<int> currentConnectCount                            {0};
        ConnectionInitParameter connectionInitParameter                 {};
        ConnectionMap connectionMap                                     {};
        ForwardCandidateMap forwardCandidateMap                         {};

        int NoneBlock(SocketID socketId) {
            int option = fcntl(socketId,F_GETFL);
            int newOption = option | O_NONBLOCK;
            return fcntl(socketId,newOption);
        }
        int EpollAdd(SocketID socketId,bool isListen = false) {
            epoll_event ev{};
            ev.data.fd = socketId;
            ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
            ev.events = (edgeTriggle ? ev.events | EPOLLET : ev.events);
            ev.events = (oneShot && !isListen ? ev.events | EPOLLONESHOT : ev.events);
            if(noneBlock) {
                if(NoneBlock(socketId) < 0){
                    LOG_ERROR(EpollerChan, "set socket none block failed:" + std::string(strerror(errno)));
                    return -1;
                }
            }
            return epoll_ctl(id,EPOLL_CTL_ADD,socketId,&ev);
        }
        int EpollMod(SocketID socketId, uint32_t ev) {
            epoll_event event{};
            event.data.fd = socketId;
            event.events = ev | EPOLLRDHUP | EPOLLHUP;
            event.events = edgeTriggle ? event.events | EPOLLET : event.events;
            event.events = oneShot ? event.events | EPOLLONESHOT : event.events;
            return epoll_ctl(id,EPOLL_CTL_MOD,socketId,&event);
        }
        int EpollDel(SocketID socketId) {
            return epoll_ctl(id,EPOLL_CTL_DEL,socketId,nullptr);
        }

        virtual bool getMapIterator(SocketID cur,ConnectionMapIterator & mapIterator){
            if((mapIterator = connectionMap.find(cur)) == connectionMap.end()){

                std::shared_ptr<T> conn;
                sockaddr_in myAddr{},destAddr{};
                socklen_t addrLen = sizeof(myAddr);

                if(!connectionPool.acquire(conn)){
                    close(cur);
                    LOG_ERROR(EpollerChan,"connection pool acquire failed");
                    return false;
                }

                if(getsockname(cur,(sockaddr*)&myAddr,&addrLen) < 0){
                    close(cur);
                    LOG_ERROR(EpollerChan,"can't get sock my address");
                    return false;
                }
                if(getpeername(cur,(sockaddr*)&destAddr,&addrLen) < 0){
                    close(cur);
                    LOG_ERROR(EpollerChan,"can't get sock dest address");
                    return false;
                }

                connectionInitParameter.sock = cur;
                connectionInitParameter.myAddr = myAddr;
                connectionInitParameter.destAddr = destAddr;
                int ret = conn->Init(connectionInitParameter);
                if(ret < 0){
                    conn->Close();
                    connectionPool.release(conn);
                    return false;
                }

                InsertConnectionMapResult result;

                result = connectionMap.insert({cur,conn});
                if(!result.second){
                    LOG_ERROR(EpollerChan,"connectionMap already have key");
                    conn->Close();
                    connectionPool.release(conn);
                    return false;
                }
                currentConnectCount++;
                mapIterator = result.first;
                LOG_TRACE(EpollerChan,std::to_string(cur) + " new connection added");
            }
            return true;
        }

        void poll(int timeOut){
            int ret,index,cur;
            ConnectionMapIterator mapIterator;

            connectionInitParameter.epollId = id;
            connectionInitParameter.filter = filter;
            connectionInitParameter.oneShot = oneShot;
            connectionInitParameter.edgeTriggle = edgeTriggle;
            connectionInitParameter.forwardCandidateMap = &forwardCandidateMap;

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
                        LOG_TRACE(EpollerChan,std::to_string(cur) + " connection close");
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
        explicit Epoller(
                bool&       _isStop,
                IPFilter*   _filter                           = nullptr,
                bool        _et                               = false,
                bool        _oneShot                          = true,
                bool        _noneBlock                        = true
        ):isStop(_isStop),filter(_filter),edgeTriggle(_et),
        oneShot(_oneShot),noneBlock(_noneBlock){
            if((id =epoll_create(1234)) < 0){
                LOG_ERROR(EpollerChan,"create epoll failed");
                exit(-1);
            }
            connectionMap.reserve(4096);
            forwardCandidateMap.reserve(4096);
        }

        ~Epoller(){
            close(id);
        }

        bool AddConnection(TCPSocket& tcpSocket){
            if(EpollAdd(tcpSocket.Sock()) < 0){
                LOG_ERROR(EpollerChan,"add connection failed");
                return false;
            }
            return true;
        };

        virtual void DelConnection(std::shared_ptr<T>& conn){
            if(EpollDel(conn->Sock())<0){
                LOG_ERROR(EpollerChan, strerror(errno));
            }
            int sock = conn->Sock();
            conn->Close();
            connectionPool.release(conn);
            connectionMap.erase(sock);
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
