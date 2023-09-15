#ifndef TOTORO_FORWARDER_H
#define TOTORO_FORWARDER_H

#include "Connection.h"
#include "core/SSLSocket.h"
#include "HttpBase.h"
#include "HttpsBase.h"

namespace totoro {

    class Forwarder{
    protected:
        virtual int WakeClient() = 0;
    };

    class HttpForwarder : public HttpBase,public Forwarder {
        HttpBase& client;
        int WakeClient() override;
    protected:
        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
    public:
        HttpForwarder(HttpBase& client);
    };

    class HttpsForwarder : public HttpsBase,public Forwarder {
        HttpsBase& client;
        int WakeClient() override;
    protected:
        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
    public:
        HttpsForwarder(HttpsBase& client);
    };

} // totoro

#endif //TOTORO_FORWARDER_H
