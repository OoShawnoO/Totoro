#include <sys/epoll.h>
#include <fcntl.h>
#include <cstring>
#include "core/Connection.h"

const std::string ConnectionChan = "Totoro";
namespace totoro {
    /* Init Impl */
    int Connection::Init(const ConnectionInitParameter &connectionInitParameter) {
        epoll_id = connectionInitParameter.epoll_id;
        oneshot = connectionInitParameter.oneshot;
        edge_trigger = connectionInitParameter.edge_trigger;
        TcpSocket::Init(
                connectionInitParameter.sock,
                *connectionInitParameter.local_address,
                *connectionInitParameter.destination_address
        );
        return 0;
    }

    /* About Epoll Impl */
    int Connection::EpollAdd(SOCKET _sock) const {
        epoll_event ev{};
        ev.data.fd = _sock;
        ev.events = EPOLLIN | EPOLLRDHUP;
        ev.events = (edge_trigger ? ev.events | EPOLLET : ev.events);
        ev.events = (oneshot ? ev.events | EPOLLONESHOT : ev.events);
        int option = fcntl(_sock, F_GETFL);
        int newOption = option | O_NONBLOCK;
        fcntl(_sock, F_SETFL, newOption);
        return epoll_ctl(epoll_id, EPOLL_CTL_ADD, _sock, &ev);
    }

    int Connection::EpollMod(SOCKET _sock, uint32_t ev) const {
        epoll_event event{};
        event.data.fd = _sock;
        event.events = ev | EPOLLRDHUP;
        event.events = edge_trigger ? event.events | EPOLLET : event.events;
        event.events = oneshot ? event.events | EPOLLONESHOT : event.events;
        return epoll_ctl(epoll_id, EPOLL_CTL_MOD, _sock, &event);
    }

    int Connection::EpollDel(SOCKET _sock) const {
        return epoll_ctl(epoll_id, EPOLL_CTL_DEL, _sock, nullptr);
    }

    /* Public Impl */
    Connection::~Connection() {
        Connection::Close();
    }

    void Connection::RegisterNextEvent(SOCKET _sock, Connection::Status nextStatus, bool isMod = false) {
        status = nextStatus;
        if (!isMod) return;
        switch (status) {
            case Read : {
                if (EpollMod(_sock, EPOLLIN) < 0) { MOLE_ERROR(ConnectionChan, strerror(errno)); }
                break;
            }
            case Write : {
                if (EpollMod(_sock, EPOLLOUT) < 0) { MOLE_ERROR(ConnectionChan, strerror(errno)); }
                break;
            }
            default: {
            }
        }
    }

    void Connection::Handler() {
        std::string data;
        RecvUntil(data,"\r\n\r\n");
        MOLE_INFO(ConnectionChan,data);
//        Submit(
//                "HTTP/1.1 200\n"
//                "Server: Totoro\n"
//                "Date: Thu, 22 Nov 2018 05:41:01 GMT\n"
//                "Content-Type: application/json;charset=UTF-8\n"
//                "Connection: keep-alive\n"
//                "Content-Length: 139\r\n"
//                "\r\n"
//                "{\"code\":0,\"data\":{\"requestId\":\"0000400004391542865263601\",\"ts\":1542865261748,\"groups\":[{\"impId\":0,\"ads\":[]}],\"emptyStatusCode\":1501010301}}"
//        );

//        Submit(
//                "HTTP/1.1 200\n"
//                "Server: Totoro\n"
//                "Date: Thu, 22 Nov 2018 05:41:01 GMT\n"
//                "Content-Type: text/plain;charset=UTF-8\n"
//                "Connection: keep-alive\n"
//                "Content-Length: 429\r\n"
//                "\r\n"
//        );
//        SubmitFile("main.cpp");
    }

    void Connection::Run() {
        while(status != None) {
            switch (status) {
                case Read : Handler();
                case Write : {
                    bool shutdown_flag = false;
                    if(status == Shutdown) { shutdown_flag = true; }
                    while(!send_queue.empty()) {
                        auto& item = send_queue.front();
                        if(item.second) {
                            if(!SendFile(item.first)) {
                                RegisterNextEvent(sock,Write,true);
                                break;
                            }else{
                                send_queue.pop_front();
                            }
                        }else{
                            size_t had_send;
                            if(item.first.size() != (had_send = SendAll(item.first))) {
                                item.first = item.first.substr(had_send);
                                RegisterNextEvent(sock,Write,true);
                                break;
                            }else{
                                send_queue.pop_front();
                            }
                        }
                    }

                    if(send_queue.empty()) RegisterNextEvent(sock,Read,true);
                    status = None;
                    if(shutdown_flag) { status = Shutdown; continue; }
                }
                case None : {
                    break;
                }
                case Shutdown : {
                    ShutDown();
                    status = None;
                    break;
                }
            }
        }
    }

    int Connection::Close() {
        status = None;
        epoll_id = BAD_SOCKET;
        return TcpSocket::Close();
    }

    int Connection::ShutDown() {
        shutdown(sock, SHUT_RDWR);
        RegisterNextEvent(sock, Read, true);
        return 1;
    }

    void Connection::Submit(std::string &&data) {
        if(!send_queue.empty() && !send_queue.back().second) {
            send_queue.back().first.append(data);
        }
        send_queue.emplace_back(std::move(data),false);
    }

    void Connection::SubmitFile(const std::string& file_name) {
        send_queue.emplace_back(file_name,true);
    }

}

