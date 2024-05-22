#include "http/HttpsBase.h"

const std::string HttpsBaseChan = "Totoro";
namespace totoro {
    int HttpsBase::Close() {
        return SSLSocket::Close();
    }

    int HttpsBase::Init(const ConnectionInitParameter &connectionInitParameter) {
        return Connection::Init(connectionInitParameter);
    }

    void HttpsBase::Handler() {
        if (!connection) {
            if (InitSSL() < 0) return;
        }
        HttpBase::Handler();
    }

} // totoro