#include "HttpsBase.h"

const std::string HttpsBaseChan = "HttpsBase";
namespace totoro {
    int HttpsBase::Close() {
        return SSLSocket::Close();
    }

    int HttpsBase::Init(const ConnectionInitParameter &connectionInitParameter) {
        if(Connection::Init(connectionInitParameter) < 0){
            LOG_ERROR(HttpsBaseChan,"failed to init connection");
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpsBase::ReadCallback() {
        if(!connection) {
            if(InitSSL() < 0){
                return FAILED;
            }
        }
        if(!SSLSocket::RecvAll(data)) return FAILED;
        if(!requestHeader.Parse(data)) return FAILED;
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsBase::AfterReadCallback() {
        return HttpBase::AfterReadCallback();
    }

    Connection::CallbackReturnType HttpsBase::WriteCallback() {
        return HttpBase::WriteCallback();
    }

    Connection::CallbackReturnType HttpsBase::AfterWriteCallback() {
        return HttpBase::AfterWriteCallback();
    }

    bool HttpsBase::SendResponseHeader() {
        std::string headerText = responseHeader.toString();
        int ret = SSLSocket::SendAll(headerText);
        if(ret == -1) return false;
        else if(ret == 0){
            while((ret =SSLSocket::SendAll(headerText)) == 0){}
            if(ret == -1) return false;
        }
        return true;
    }

    bool HttpsBase::SendResponseBody() {
        if(!responseBody.GetResourcePath().empty()){
            int ret = SSLSocket::SendFile(responseBody.GetResourcePath());
            if(ret == -1) return false;
            else if(ret == 0){
                while((ret = SSLSocket::SendFile(responseBody.GetResourcePath())) == 0){}
                if(ret == -1) return false;
            }
        }
        else
        {
            int ret = SSLSocket::SendAll(responseBody.GetData());
            if(ret == -1) return false;
            else if(ret == 0){
                while((ret =SSLSocket::SendAll(responseBody.GetData())) == 0){}
                if(ret == -1) return false;
            }
        }
        return true;
    }
} // totoro