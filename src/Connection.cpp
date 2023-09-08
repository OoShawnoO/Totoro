#include <sys/epoll.h>
#include <fcntl.h>
#include "Connection.h"

const std::string ConnectionChan = "Connection";
namespace totoro {
    /* Init Impl */
    void Connection::Init(TCPSocket& tcpSocket,EpollID _epollId,bool _edgeTriggle,bool _oneShot){
        TCPSocket::operator=(tcpSocket);
        epollId = _epollId;
        edgeTriggle = _edgeTriggle;
        oneShot = _oneShot;
    }

    int Connection::Init(SocketID sock,sockaddr_in myAddr, sockaddr_in destAddr, EpollID _epollId, bool _edgeTriggle, bool _oneShot) {
        Socket::Init(sock,myAddr,destAddr);
        epollId = _epollId;
        edgeTriggle = _edgeTriggle;
        oneShot = _oneShot;
        return -1;
    }
    /* About Epoll Impl */
    int Connection::EpollAdd(SocketID _sock) const {
        epoll_event ev{};
        ev.data.fd = _sock;
        ev.events = EPOLLIN | EPOLLRDHUP;
        ev.events = (edgeTriggle ? ev.events | EPOLLET : ev.events);
        ev.events = (oneShot ? ev.events | EPOLLONESHOT : ev.events);
        int option = fcntl(_sock,F_GETFL);
        int newOption = option | O_NONBLOCK;
        fcntl(_sock,newOption);
        return epoll_ctl(epollId,EPOLL_CTL_ADD,_sock,&ev);
    }

    int Connection::EpollMod(SocketID _sock,uint32_t ev) const {
        epoll_event event{};
        event.data.fd = _sock;
        event.events = ev | EPOLLRDHUP;
        event.events = edgeTriggle ? event.events | EPOLLET : event.events;
        event.events = oneShot ? event.events | EPOLLONESHOT : event.events;
        return epoll_ctl(epollId,EPOLL_CTL_MOD,_sock,&event);
    }

    int Connection::EpollDel(SocketID _sock) const {
        return epoll_ctl(epollId,EPOLL_CTL_DEL,_sock,nullptr);
    }
    /* Public Impl */
    Connection::~Connection() {
        Connection::Close();
    }

    void Connection::RegisterNextEvent(SocketID _sock,Connection::Status nextStatus,bool isMod = false) {
        status = nextStatus;
        if(!isMod) return;
        switch(status){
            case Read : {
                EpollMod(_sock,EPOLLIN);
                break;
            }
            case Write : {
                EpollMod(_sock,EPOLLOUT);
                break;
            }
            default:{}
        }
    }

    void Connection::Run() {
        if(lastStatus != None) {
            status = lastStatus;
            lastStatus = None;
        }
        while(status != None){
            int ret;
            switch (status) {
                case Read : {
                    ret = ReadCallback();
                    switch(ret){
                        case 1 : status = AfterRead; break;
                        case 0 : lastStatus = Read;status = None; break;
                        default: {
                            LOG_ERROR(ConnectionChan,"ReadCallback failed");
                            status = Error;
                        }
                    }
                    break;
                }
                case AfterRead : {
                    ret = AfterReadCallback();
                    switch(ret){
                        case 1 : status = Write; break;
                        case 0 : lastStatus = AfterRead;status = None; break;
                        default: {
                            LOG_ERROR(ConnectionChan,"AfterReadCallback failed");
                            status = Error;
                        }
                    }
                    break;
                }
                case Write : {
                    ret = WriteCallback();
                    switch(ret){
                        case 1 : status = AfterWrite; break;
                        case 0 : lastStatus = Write;status = None; break;
                        default: {
                            LOG_ERROR(ConnectionChan,"WriteCallback failed");
                            status = Error;
                        }
                    }
                    break;
                }
                case AfterWrite : {
                    ret = AfterWriteCallback();
                    switch(ret) {
                        case 0 : lastStatus = AfterWrite;
                        case 1 : status = None; break;
                        default:{
                            LOG_ERROR(ConnectionChan,"AfterWriteCallback failed");
                            status = Error;
                        }
                    }
                    break;
                }
                case Error :
                    Close();
                case None : {
                    return;
                }
            }
        }
    }

    int Connection::Close() {
        status = None;
        return TCPSocket::Close();
    }

    void Connection::SetWorkSock(SocketID _sock) {
        workSock = _sock;
    }

    /* Protected Impl */
    int Connection::ReadCallback() {
        if(TCPSocket::RecvAll(data)){
            return 1;
        }
        return -1;
    }

    int Connection::AfterReadCallback() {
        return 1;
    }

    int Connection::WriteCallback() {
        data.clear();
        data = "HTTP/1.1 200 OK\n"
               "Content-Type: text/html\n"
               "Content-Length: 6\n"
               "\n"
               "123456";
        if(TCPSocket::SendAll(data)){
            return 1;
        }
        return -1;
    }

    int Connection::AfterWriteCallback() {
        RegisterNextEvent(sock,Read,true);
        return 1;
    }
}

