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

    int HttpsBase::ReadCallback() {
        if(!connection) {
            if(InitSSL() < 0){
                return -1;
            }
        }
        if(!SSLSocket::RecvAll(data)) return -1;
        if(!requestHeader.Parse(data)) return -1;
        return 1;
    }

    int HttpsBase::AfterReadCallback() {
        return HttpBase::AfterReadCallback();
    }

    int HttpsBase::WriteCallback() {
        return HttpBase::WriteCallback();
    }

    int HttpsBase::AfterWriteCallback() {
        return HttpBase::AfterWriteCallback();
    }
} // totoro