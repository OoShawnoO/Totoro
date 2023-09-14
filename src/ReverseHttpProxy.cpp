#include "ReverseHttpProxy.h"

const std::string ReverseHttpProxyChan = "ReverseHttpProxy";

namespace totoro {
    const std::pair<std::string, unsigned short> &ReverseHttpProxy::GetReverseProxyServer(const std::string &port) {
        static std::unordered_map<std::string,std::vector<std::pair<std::string,unsigned short>>> candidateServers;
        static std::unordered_map<std::string,int> candidateServersConnectionNum;
        static bool isInit = true;
        if(isInit){
            auto reverseProxyMap = Configure::Get()["HTTP_REVERSE_PROXY"];
            for(auto items = reverseProxyMap.cbegin(); items != reverseProxyMap.cend(); items++) {
                std::vector<std::pair<std::string,unsigned short>> servers;
                for(const auto& item : items.value()){
                    servers.emplace_back(item["ip"],item["port"]);
                }
                auto ret = candidateServers.insert({items.key(),std::move(servers)});
                if(!ret.second){
                    LOG_ERROR(ReverseHttpProxyChan,"candidate server insert failed");
                    exit(-1);
                }
            }
            isInit = false;
        }
        auto candidateServersIter = candidateServers.end();
        if((candidateServersIter = candidateServers.find(port)) == candidateServers.end()){
            LOG_ERROR(ReverseHttpProxyChan,"candidate server find port failed");
            exit(-1);
        }
        auto candidateNum = (candidateServersConnectionNum[port]++)%candidateServersIter->second.size();
        return candidateServersIter->second[candidateNum];
    }

    int ReverseHttpProxy::Init(const ConnectionInitParameter &connectionInitParameter) {
        Connection::Init(connectionInitParameter);
        auto port = std::to_string(ntohs(myAddr.sin_port));
        const auto& addr = GetReverseProxyServer(port);
        forwardSocket.Init();
        if(!forwardSocket.Connect(addr.first,addr.second)){
            LOG_ERROR(ReverseHttpProxyChan,"can't connect destination address");
            return -1;
        }
        if(!AddForward()){
            LOG_ERROR(ReverseHttpProxyChan,"can't add forward");
            return -1;
        }
        return 0;
    }

    int ReverseHttpProxy::Close() {
        return ProxyBase::Close();
    }

    int ReverseHttpProxy::ReadCallback() {
        return ProxyBase::ReadCallback();
    }

    int ReverseHttpProxy::AfterReadCallback() {
        return ProxyBase::AfterReadCallback();
    }

    int ReverseHttpProxy::WriteCallback() {
        return ProxyBase::WriteCallback();
    }

    int ReverseHttpProxy::AfterWriteCallback() {
        return ProxyBase::AfterWriteCallback();
    }

    int ReverseHttpProxy::MainReadCallback() {
        return ProxyBase::MainReadCallback();
    }

    int ReverseHttpProxy::MainAfterReadCallback() {
        return ProxyBase::MainAfterReadCallback();
    }

    int ReverseHttpProxy::ForwardReadCallback() {
        return ProxyBase::ForwardReadCallback();
    }

    int ReverseHttpProxy::ForwardAfterReadCallback() {
        return ProxyBase::ForwardAfterReadCallback();
    }

    int ReverseHttpProxy::ForwardWriteCallback() {
        return ProxyBase::ForwardWriteCallback();
    }

    int ReverseHttpProxy::ForwardAfterWriteCallback() {
        return ProxyBase::ForwardAfterWriteCallback();
    }

    int ReverseHttpProxy::MainWriteCallback() {
        return ProxyBase::MainWriteCallback();
    }

    int ReverseHttpProxy::MainAfterWriteCallback() {
        return ProxyBase::MainAfterWriteCallback();
    }
} // totoro