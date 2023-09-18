#include <sys/epoll.h>
#include <fcntl.h>
#include "core/Connection.h"

const std::string ConnectionChan = "Connection";
namespace totoro {
    /* Init Impl */
    int Connection::Init(const ConnectionInitParameter& connectionInitParameter) {
        filter = connectionInitParameter.filter;
        epollId = connectionInitParameter.epollId;
        oneShot = connectionInitParameter.oneShot;
        edgeTriggle = connectionInitParameter.edgeTriggle;
        forwardCandidateMap = connectionInitParameter.forwardCandidateMap;
        closeChan = connectionInitParameter.closeChan;
        Socket::Init(connectionInitParameter.sock,connectionInitParameter.myAddr,
                     connectionInitParameter.destAddr);
        return 0;
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
                if(EpollMod(_sock,EPOLLIN) < 0){
                    LOG_ERROR(ConnectionChan, strerror(errno));
                }
                break;
            }
            case Write : {
                if(EpollMod(_sock,EPOLLOUT) < 0){
                    LOG_ERROR(ConnectionChan, strerror(errno));
                }
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
            CallbackReturnType ret;
            switch (status) {
                case Read : {
                    ret = ReadCallback();
                    switch(ret){
                        case SUCCESS : status = AfterRead; break;
                        case AGAIN : {
                            lastStatus = status;
                            RegisterNextEvent(workSock,Read,true);
                            status = None;
                            break;
                        }
                        case FAILED : {
                            LOG_ERROR(ConnectionChan,"ReadCallback failed");
                            status = Error;
                            break;
                        }
                        case SHUTDOWN : {
                            ShutDown();
                            status = None;
                            break;
                        }
                        case INTERRUPT : {
                            status = None;
                            break;
                        }
                    }
                    break;
                }
                case AfterRead : {
                    ret = AfterReadCallback();
                    switch(ret){
                        case SUCCESS : status = Write; break;
                        case AGAIN : {
                            lastStatus = status;
                            RegisterNextEvent(workSock, Read, true);
                            status = None;
                            break;
                        }
                        case FAILED : {
                            LOG_ERROR(ConnectionChan,"AfterReadCallback failed");
                            status = Error;
                            break;
                        }
                        case SHUTDOWN : {
                            ShutDown();
                            status = None;
                            break;
                        }
                        case INTERRUPT : {
                            status = None;
                            break;
                        }
                    }
                    break;
                }
                case Write : {
                    ret = WriteCallback();
                    switch(ret){
                        case SUCCESS : status = AfterWrite; break;
                        case AGAIN : {
                            lastStatus = status;
                            RegisterNextEvent(workSock, Write, true);
                            status = None;
                            break;
                        }
                        case FAILED : {
                            LOG_ERROR(ConnectionChan,"WriteCallback failed");
                            status = Error;
                            break;
                        }
                        case SHUTDOWN : {
                            ShutDown();
                            status = None;
                            break;
                        }
                        case INTERRUPT : {
                            status = None;
                            break;
                        }
                    }
                    break;
                }
                case AfterWrite : {
                    ret = AfterWriteCallback();
                    switch(ret) {
                        case SUCCESS : {
                            RegisterNextEvent(sock,Read,true);
                            status = None;
                            break;
                        }
                        case AGAIN : {
                            lastStatus = status;
                            RegisterNextEvent(workSock, Write, true);
                            status = None;
                            break;
                        }
                        case FAILED : {
                            LOG_ERROR(ConnectionChan,"WriteCallback failed");
                            status = Error;
                            break;
                        }
                        case SHUTDOWN : {
                            ShutDown();
                            status = None;
                            break;
                        }
                        case INTERRUPT : {
                            status = None;
                            break;
                        }
                    }
                    break;
                }
                case Error :
                    ShutDown();
                    status = None;
                case None : {
                    return;
                }
            }
        }
    }

    int Connection::Close() {
        status = None;
        filter = nullptr;
        forwardCandidateMap = nullptr;
        lastStatus = None;
        epollId = BAD_FILE_DESCRIPTOR;
        workSock = BAD_FILE_DESCRIPTOR;
        LOG_TRACE(ConnectionChan,std::to_string(sock) + " closed");
        return TCPSocket::Close();
    }

    void Connection::SetWorkSock(SocketID _sock) {
        workSock = _sock;
    }

    Connection::CallbackReturnType Connection::ReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType Connection::AfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType Connection::WriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType Connection::AfterWriteCallback() {
        return SUCCESS;
    }

    int Connection::ShutDown() {
        if(closeChan){
            closeChan->push(sock);
        }
        shutdown(sock,SHUT_RDWR);
        RegisterNextEvent(sock,Read,true);
        return 1;
    }

    bool Connection::BanAddr(const std::string& banIp) {
        if(!filter) return false;
        in_addr_t addr = inet_addr(banIp.c_str());
        return filter->AddBan(addr);
    }

    bool Connection::AllowAddr(const std::string& allowIp) {
        if(!filter) return false;
        in_addr_t addr = inet_addr(allowIp.c_str());
        return filter->AddAllow(addr);
    }
}

