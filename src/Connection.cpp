#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cstring>
#include "Connection.h"

namespace totoro {

    void Connection::Init(TCPSocket& tcpSocket,EpollID _epollId,bool _edgeTriggle,bool _oneShot){
        TCPSocket::operator=(tcpSocket);
        epollId = _epollId;
        edgeTriggle = _edgeTriggle;
        oneShot = _oneShot;
    }

    int Connection::EpollAdd() {
        epoll_event ev{};
        ev.data.fd = sock;
        ev.events = EPOLLIN | EPOLLRDHUP;
        ev.events = (edgeTriggle ? ev.events | EPOLLET : ev.events);
        ev.events = (oneShot ? ev.events | EPOLLONESHOT : ev.events);
        int option = fcntl(sock,F_GETFL);
        int newOption = option | O_NONBLOCK;
        fcntl(sock,newOption);
        return epoll_ctl(epollId,EPOLL_CTL_ADD,sock,&ev);
    }

    int Connection::EpollMod(uint32_t ev) {
        epoll_event event{};
        event.data.fd = sock;
        event.events = ev | EPOLLRDHUP;
        event.events = edgeTriggle ? event.events | EPOLLET : event.events;
        event.events = oneShot ? event.events | EPOLLONESHOT : event.events;
        return epoll_ctl(epollId,EPOLL_CTL_MOD,sock,&event);
    }

    void Connection::RegisterNextEvent(Connection::Status nextStatus,bool isMod = false) {
        status = nextStatus;
        if(!isMod) return;
        switch(status){
            case Read : {
                EpollMod(EPOLLIN);
                break;
            }
            case Write : {
                EpollMod(EPOLLOUT);
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
        std::cout << data << std::endl;
        return 1;
    }

    int Connection::WriteCallback() {
        if(TCPSocket::SendAll(data)){
            return 1;
        }
        return -1;
    }

    int Connection::AfterWriteCallback() {
        RegisterNextEvent(Read,true);
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
                    if(AfterReadCallback()){
                        status = Write;
                    }else{
                        LOG_ERROR("Connection","AfterReadCallback failed");
                        status = Error;
                    }
                    break;
                }
                case AfterWrite : {
                    if(AfterWriteCallback()){
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

    void Connection::Close() {
        TCPSocket::Close();
        status = None;
    }

    Connection::~Connection() {
        Connection::Close();
    }
}

