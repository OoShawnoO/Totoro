#include <iostream>
#include "ProxyBase.h"
static const std::string ProxyBaseChan = "ProxyBase";
namespace totoro {
    int ProxyBase::MainReadCallback() {
        return Connection::ReadCallback();
    }

    int ProxyBase::MainAfterReadCallback() {
        int ret = forwardSocket.SendAll(data);
        RegisterNextEvent(forwardSocket.Sock(),Read,true);
        if(ret == 0) return 0;
        else if(ret < 0) return -1;
        else return 2;
    }

    int ProxyBase::ForwardReadCallback() {
        if(!forwardSocket.RecvAll(data)) return -1;
        return 1;
    }

    int ProxyBase::ForwardAfterReadCallback() {
        return 1;
    }

    int ProxyBase::ForwardWriteCallback() {
        return 1;
    }

    int ProxyBase::ForwardAfterWriteCallback() {
        RegisterNextEvent(sock,Write,true);
        return 1;
    }

    int ProxyBase::MainWriteCallback() {
        return Connection::WriteCallback();
    }

    int ProxyBase::MainAfterWriteCallback() {
        RegisterNextEvent(forwardSocket.Sock(),Read,true);
        return Connection::AfterWriteCallback();
    }

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

    int ProxyBase::Close() {
        int proxyFd = forwardSocket.Sock();
        forwardSocket.Close();
        Connection::Close();
        return proxyFd;
    }

    bool ProxyBase::AddForward() {
        if(!forwardCandidateMap->insert({forwardSocket.Sock(), sock}).second){
            LOG_ERROR(ProxyBaseChan,"unable to add forward pair");
            return false;
        }
        if(EpollAdd(forwardSocket.Sock()) < 0){
            LOG_ERROR(ProxyBaseChan, strerror(errno));
            return false;
        }
        return true;
    }
} // totoro