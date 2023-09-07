#include <iostream>
#include "ProxyBase.h"
static const std::string ProxyBaseChan = "ProxyBase";
namespace totoro {
    /* Public Impl */
    int ProxyBase::MainReadCallback() {
        return Connection::ReadCallback();
    }

    int ProxyBase::MainAfterReadCallback() {
        return forwardSocket.SendAll(data)>=0 ? 0 : -1;
    }

    int ProxyBase::MainWriteCallback() {
        return Connection::WriteCallback();
    }

    int ProxyBase::MainAfterWriteCallback() {
        return Connection::AfterWriteCallback();
    }

    int ProxyBase::ForwardReadCallback() {
        if(forwardSocket.RecvAll(data)) return 1;
        return -1;
    }

    int ProxyBase::ForwardAfterReadCallback() {
        return 1;
    }

    int ProxyBase::ForwardWriteCallback() {
        RegisterNextEvent(sock,Write,true);
        return 1;
    }

    int ProxyBase::ForwardAfterWriteCallback() {
        return 1;
    }
    /* Private Impl */
    int ProxyBase::ReadCallback() {
        if(workSock == forwardSocket.Sock()){
            return ForwardReadCallback();
        }
        return MainReadCallback();
    }

    int ProxyBase::AfterReadCallback() {
        if(workSock == forwardSocket.Sock()){
            return ForwardAfterReadCallback();
        }
        return MainAfterReadCallback();
    }

    int ProxyBase::WriteCallback() {
        if(workSock == forwardSocket.Sock()){
            return ForwardWriteCallback();
        }
        return MainWriteCallback();
    }

    int ProxyBase::AfterWriteCallback() {
        if(workSock == forwardSocket.Sock()){
            return ForwardAfterWriteCallback();
        }
        return MainAfterWriteCallback();
    }
    /* Connection Public Impl */
    int ProxyBase::Init(SocketID sock, sockaddr_in myAddr, sockaddr_in destAddr, EpollID epollId,
                      bool edgeTriggle,bool oneShot) {
        Connection::Init(sock, myAddr, destAddr, epollId, edgeTriggle, oneShot);
        if(forwardSocket.Sock() == BAD_FILE_DESCRIPTOR){
            auto addressJson = Configure::Get()["PROXY"][std::to_string(ntohs(myAddr.sin_port))];
            int index = rand() % addressJson.size();
            std::string ip = addressJson[index]["ip"];
            short port = addressJson[index]["port"];

            forwardSocket.Init();
            if(!forwardSocket.Connect(ip,port)){
                LOG_ERROR(ProxyBaseChan,"can't connect destination address");
                return -2;
            }
        }
        return forwardSocket.Sock();
    }

    int ProxyBase::Close() {
        int proxyFd = forwardSocket.Sock();
        forwardSocket.Close();
        Connection::Close();
        return proxyFd;
    }
} // totoro