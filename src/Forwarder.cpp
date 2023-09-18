#include <netdb.h>
#include "Forwarder.h"
const std::string HttpReverseForwarderChan = "HttpReverseForwarder";
const std::string HttpsReverseForwarderChan = "HttpsReverseForwarder";
const std::string HttpsForwardForwarderChan = "HttpsForwardForwarder";
const std::string HttpForwardForwarderChan = "HttpForwardForwarder";
namespace totoro {

    int HttpForwardForwarder::InitForwarder() {
        auto host = requestHeader.GetFields().find("Host");
        if(host == requestHeader.GetFields().end() || host->second.empty()){
            LOG_ERROR(HttpForwardForwarderChan,"host not found");
            return -1;
        }
        std::string ip,hostAddr = host->second[0];
        unsigned short port;
        auto pos = hostAddr.find(':');
        if(pos == std::string::npos){
            ip = hostAddr;
            port = 80;
        }else{
            ip = hostAddr.substr(0,pos);
            port = std::stoi(hostAddr.substr(pos+1));
        }
        auto hostEntry = gethostbyname(ip.c_str());
        if(!hostEntry){
            LOG_ERROR(HttpForwardForwarderChan,"can't parse ip address:" + ip);
            return -1;
        }
        forwarder.TCPSocket::Init();
        bool connected = false;
        std::string tempIP;
        for(int i=0;hostEntry->h_addr_list[i];i++){
            tempIP = inet_ntoa(*(struct in_addr*)hostEntry->h_addr_list[i]);
            if(forwarder.Connect(tempIP,port)){
                connected = true;
                break;
            }
        }
        if(!connected){
            LOG_ERROR(HttpForwardForwarderChan,"can't connect destination address");
            return -1;
        }
        if(!forwardCandidateMap->insert({forwarder.Sock(), sock}).second){
            LOG_ERROR(HttpForwardForwarderChan,"unable to add forward pair");
            return -1;
        }
        if(EpollAdd(forwarder.Sock()) < 0){
            LOG_ERROR(HttpForwardForwarderChan, strerror(errno));
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderReadCallback() {
        return forwarder.ParseResponse();
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderAfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderAfterWriteCallback() {
        RegisterNextEvent(sock,Write,true);
        return SUCCESS;
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::ReadCallback() {
        if(workSock == forwarder.Sock()){
            return ForwarderReadCallback();
        }
        return HttpBase::ReadCallback();
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::AfterReadCallback() {
        if(workSock == forwarder.Sock()){
            return ForwarderAfterReadCallback();
        }
        if(forwarder.Sock() == BAD_FILE_DESCRIPTOR){
            if(InitForwarder() < 0) return FAILED;
        }
        int ret = forwarder.SendAll(requestText);
        if(ret < 0){
            LOG_ERROR(HttpForwardForwarderChan,"forwarder send request failed");
            return FAILED;
        }else if(ret == 0){
            return AGAIN;
        }
        RegisterNextEvent(forwarder.Sock(),Read,true);
        return INTERRUPT;
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::WriteCallback() {
        if(workSock == forwarder.Sock()){
            return ForwarderWriteCallback();
        }
        int ret = SendAll(forwarder.responseText);
        if(ret < 0){
            LOG_ERROR(HttpForwardForwarderChan,"send response failed");
            return FAILED;
        }else if(ret == 0){
            return AGAIN;
        }
        return SUCCESS;
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::AfterWriteCallback() {
        if(workSock == forwarder.Sock()){
            return ForwarderAfterWriteCallback();
        }
        return HttpBase::AfterWriteCallback();
    }

    int HttpForwardForwarder::Close() {
        return Connection::Close();
    }

    int HttpsForwardForwarder::InitForwarder() {
        auto host = requestHeader.GetFields().find("Host");
        if(host == requestHeader.GetFields().end() || host->second.empty()){
            LOG_ERROR(HttpsForwardForwarderChan,"host not found");
            return -1;
        }
        std::string ip,hostAddr = host->second[0];
        unsigned short port;
        auto pos = hostAddr.find(':');
        if(pos == std::string::npos){
            ip = hostAddr;
            port = 443;
        }else{
            ip = hostAddr.substr(0,pos);
            port = std::stoi(hostAddr.substr(pos+1));
        }
        auto hostEntry = gethostbyname(ip.c_str());
        if(!hostEntry){
            LOG_ERROR(HttpsForwardForwarderChan,"can't parse ip address:" + ip);
            return -1;
        }
        forwarder.TCPSocket::Init();
        bool connected = false;
        std::string tempIP;
        for(int i=0;hostEntry->h_addr_list[i];i++){
            tempIP = inet_ntoa(*(struct in_addr*)hostEntry->h_addr_list[i]);
            if(forwarder.Connect(tempIP,port)){
                connected = true;
                break;
            }
        }
        if(!connected){
            LOG_ERROR(HttpsForwardForwarderChan,"can't connect destination address");
            return -1;
        }
        if(!forwardCandidateMap->insert({forwarder.Sock(), sock}).second){
            LOG_ERROR(HttpsForwardForwarderChan,"unable to add forward pair");
            return -1;
        }
        if(EpollAdd(forwarder.Sock()) < 0){
            LOG_ERROR(HttpsForwardForwarderChan, strerror(errno));
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderAfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderAfterWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ReadCallback() {
        return HttpsBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpsForwardForwarder::AfterReadCallback() {
        return HttpsBase::AfterReadCallback();
    }

    Connection::CallbackReturnType HttpsForwardForwarder::WriteCallback() {
        return HttpsBase::WriteCallback();
    }

    Connection::CallbackReturnType HttpsForwardForwarder::AfterWriteCallback() {
        return HttpsBase::AfterWriteCallback();
    }

    int HttpsForwardForwarder::Close() {
        return HttpsBase::Close();
    }

    int HttpReverseForwarder::InitForwarder() {
        return 0;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderReadCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderAfterReadCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderWriteCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderAfterWriteCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ReadCallback() {
        return HttpBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpReverseForwarder::AfterReadCallback() {
        return HttpBase::AfterReadCallback();
    }

    Connection::CallbackReturnType HttpReverseForwarder::WriteCallback() {
        return HttpBase::WriteCallback();
    }

    Connection::CallbackReturnType HttpReverseForwarder::AfterWriteCallback() {
        return HttpBase::AfterWriteCallback();
    }

    int HttpReverseForwarder::Close() {
        return Connection::Close();
    }

    int HttpsReverseForwarder::InitForwarder() {
        return 0;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderReadCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderAfterReadCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderWriteCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderAfterWriteCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ReadCallback() {
        return HttpsBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpsReverseForwarder::AfterReadCallback() {
        return HttpsBase::AfterReadCallback();
    }

    Connection::CallbackReturnType HttpsReverseForwarder::WriteCallback() {
        return HttpsBase::WriteCallback();
    }

    Connection::CallbackReturnType HttpsReverseForwarder::AfterWriteCallback() {
        return HttpsBase::AfterWriteCallback();
    }

    int HttpsReverseForwarder::Close() {
        return HttpsBase::Close();
    }
} // totoro