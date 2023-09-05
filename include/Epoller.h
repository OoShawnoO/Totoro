#ifndef TOTOROSERVER_EPOLLER_H
#define TOTOROSERVER_EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include "Connection.h"
#include "Pool.h"
#include "ThreadPool.h"

namespace totoro {

    template<class T = Connection>
    class Epoller {
        EpollID id                  {BAD_FILE_DESCRIPTOR};
        epoll_event events[4096]    {};
        bool edgeTriggle            {false};
        bool oneShot                {true};
        bool noneBlock              {true};
        bool& isStop                ;
        Pool<T> connectionPool      {1024};
        ThreadPool<T> threadPool    {4,8196};
        std::unordered_map<int,std::shared_ptr<T>> connectionMap;

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
    public:
        Epoller(const std::string& ip,short port,bool& _isStop,bool _et = false,
                bool _oneShot = true,bool _noneBlock = false):
        isStop(_isStop),edgeTriggle(_et),
        oneShot(_oneShot),noneBlock(_noneBlock){
            if((id =epoll_create(1234)) < 0){
                LOG_ERROR("Epoller","create epoll failed");
                exit(-1);
            }
        }

        ~Epoller(){
            close(id);
        }

        bool AddConnection(TCPSocket& tcpSocket){
            std::shared_ptr<T> conn;
            if(!connectionPool.acquire(conn)){
                tcpSocket.Close();
                LOG_ERROR("Epoller","connection pool acquire failed");
                return false;
            }
            conn->Init(tcpSocket,id);
            if(EpollAdd(tcpSocket.Sock()) < 0){
                return false;
            }
            connectionMap[tcpSocket.Sock()] = conn;
            return true;
        };

        void DelConnection(std::shared_ptr<T>& conn){
            EpollDel(conn->Sock());
            connectionMap.erase(conn->Sock());
            conn->Close();
            connectionPool.release(conn);
        };

        static void Poll(Epoller<T>* _epoller,int timeOut){
            auto& epoller = *_epoller;
            int ret,index,cur;
            TCPSocket tcpSocket{};
            typename std::unordered_map<int,std::shared_ptr<T>>::iterator mapIterator;
            while(!epoller.isStop){
                if((ret = epoll_wait(epoller.id,epoller.events,4096,timeOut)) < 0){
                    if(errno == EINTR) continue;
                    LOG_ERROR("Epoller","epoll wait failed");
                    exit(-1);
                }
                for(index = 0;index < ret;index++ ){
                    cur = epoller.events[index].data.fd;

                    if((mapIterator = epoller.connectionMap.find(cur)) == epoller.connectionMap.end()){
                        LOG_ERROR("Epoller","un-added socket fd come in");
                        exit(-1);
                    }
                    auto& connection = mapIterator->second;
                    auto event = epoller.events[index].events;
                    if(event & EPOLLRDHUP){
                        LOG_INFO("Epoller","client close");
                        epoller.DelConnection(connection);
                    }else if(event & EPOLLERR){
                        LOG_ERROR("Epoller","epoll error");
                        epoller.DelConnection(connection);
                    }else if(event & EPOLLIN){
                        connection->RegisterNextEvent(Connection::Read,false);
                        epoller.threadPool.Add(connection);
                    }else if(event & EPOLLOUT){
                        connection->RegisterNextEvent(Connection::Write,false);
                        epoller.threadPool.Add(connection);
                    }else{
                        LOG_ERROR("Epoller","unknown error");
                        epoller.DelConnection(connection);
                    }
                }
            }
        }
    };

} // totoro

#endif //TOTOROSERVER_EPOLLER_H
