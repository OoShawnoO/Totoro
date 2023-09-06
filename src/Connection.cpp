#include <sys/epoll.h>
#include <fcntl.h>
#include "Connection.h"

namespace totoro {

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

    int Connection::EpollAdd(SocketID _sock) {
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

    int Connection::EpollMod(SocketID _sock,uint32_t ev) {
        epoll_event event{};
        event.data.fd = _sock;
        event.events = ev | EPOLLRDHUP;
        event.events = edgeTriggle ? event.events | EPOLLET : event.events;
        event.events = oneShot ? event.events | EPOLLONESHOT : event.events;
        return epoll_ctl(epollId,EPOLL_CTL_MOD,_sock,&event);
    }

    int Connection::EpollDel(SocketID _sock) {
        return epoll_ctl(epollId,EPOLL_CTL_DEL,_sock,nullptr);
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

    void Connection::Run() {
        while(status != None){
            int ret;
            switch (status) {
                case Read : {
                    ret = ReadCallback();
                    if(ret == 1){
                        status = AfterRead;
                    }else if(ret == 0){
                        status = None;
                    }else{
                        LOG_ERROR("Connection","ReadCallback failed");
                        status = Error;
                    }
                    break;
                }
                case Write : {
                    ret = WriteCallback();
                    if(ret == 1){
                        status = AfterWrite;
                    }else if(ret == 0){
                        status = None;
                    }else{
                        LOG_ERROR("Connection","WriteCallback failed");
                        status = Error;
                    }
                    break;
                }
                case AfterRead : {
                    ret = AfterReadCallback();
                    if(ret == 1){
                        status = Write;
                    }else if(ret == 0){
                        status = None;
                    }else{
                        LOG_ERROR("Connection","AfterReadCallback failed");
                        status = Error;
                    }
                    break;
                }
                case AfterWrite : {
                    ret = AfterWriteCallback();
                    if(ret >= 0){
                        status = None;
                    }else{
                        LOG_ERROR("Connection","AfterWriteCallback failed");
                        status = Error;
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

    Connection::~Connection() {
        Connection::Close();
    }

    void Connection::SetWorkSock(SocketID _sock) {
        workSock = _sock;
    }
}

