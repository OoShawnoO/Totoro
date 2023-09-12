#include <netdb.h>
#include "ForwardHttpProxy.h"

const std::string ForwardHttpProxyChan = "ForwardHttpProxy";
namespace totoro {
    int ForwardHttpProxy::Init(const ConnectionInitParameter &connectionInitParameter) {
        return Connection::Init(connectionInitParameter);
    }

    int ForwardHttpProxy::Close() {
        return ProxyBase::Close();
    }

    int ForwardHttpProxy::ReadCallback() {
        return ProxyBase::ReadCallback();
    }

    int ForwardHttpProxy::AfterReadCallback() {
        return ProxyBase::AfterReadCallback();
    }

    int ForwardHttpProxy::WriteCallback() {
        return ProxyBase::WriteCallback();
    }

    int ForwardHttpProxy::AfterWriteCallback() {
        return ProxyBase::AfterWriteCallback();
    }

    int ForwardHttpProxy::MainReadCallback() {
        if(forwardSocket.Sock() == BAD_FILE_DESCRIPTOR){
            return HttpBase::ReadCallback();
        }
        if(!TCPSocket::RecvAll(data)) return -1;
        return 1;
    }

    int ForwardHttpProxy::MainAfterReadCallback() {
        if(forwardSocket.Sock() == BAD_FILE_DESCRIPTOR){
            auto host = requestHeader.GetFields().find("Host");
            if(host == requestHeader.GetFields().end() || host->second.empty()){
                LOG_ERROR(ForwardHttpProxyChan,"request host not found");
                return -1;
            }
            std::string hostAddrStr = host->second[0];
            auto pos = hostAddrStr.find(':');
            std::string ip;
            unsigned short port;
            if(pos != std::string::npos){
                ip = hostAddrStr.substr(0,pos);
                port = std::stoi(hostAddrStr.substr(pos + 1));
            }else{
                ip =  hostAddrStr;
                port = 80;
            }

            auto hostEntry = gethostbyname(ip.c_str());
            if(!hostEntry){
                LOG_ERROR(ForwardHttpProxyChan,"can't parse ip address:"  + ip);
                return -1;
            }

            forwardSocket.Init();
            bool connected = false;
            for(int i=0;hostEntry->h_addr_list[i];i++){
                ip = inet_ntoa(*(struct in_addr*)hostEntry->h_addr_list[i]);
                if(forwardSocket.Connect(ip,port)){
                    connected = true;
                    break;
                }
            }

            if(!connected){
                LOG_ERROR(ForwardHttpProxyChan,"can't connect destination address");
                return -1;
            }

            if(!AddForward()){
                LOG_ERROR(ForwardHttpProxyChan,"can't add forward");
                return -1;
            }
        }

        return ProxyBase::MainAfterReadCallback();
    }

    int ForwardHttpProxy::ForwardReadCallback() {
        return ProxyBase::ForwardReadCallback();
    }

    int ForwardHttpProxy::ForwardAfterReadCallback() {
        return ProxyBase::ForwardAfterReadCallback();
    }

    int ForwardHttpProxy::ForwardWriteCallback() {
        return ProxyBase::ForwardWriteCallback();
    }

    int ForwardHttpProxy::ForwardAfterWriteCallback() {
        return ProxyBase::ForwardAfterWriteCallback();
    }

    int ForwardHttpProxy::MainWriteCallback() {
        return ProxyBase::MainWriteCallback();
    }

    int ForwardHttpProxy::MainAfterWriteCallback() {
        return ProxyBase::MainAfterWriteCallback();
    }
} // totoro