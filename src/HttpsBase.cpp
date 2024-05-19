#include "core/HttpsBase.h"

const std::string HttpsBaseChan = "Totoro";
namespace totoro {
    int HttpsBase::Close() {
        return SSLSocket::Close();
    }

    int HttpsBase::Init(const ConnectionInitParameter &connectionInitParameter) {
        return Connection::Init(connectionInitParameter);
    }

    Connection::CallbackReturnType HttpsBase::ReadCallback() {
        if(!connection) {
            if(InitSSL() < 0) return FAILED;
        }
        return HttpBase::ReadCallback();
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
} // totoro