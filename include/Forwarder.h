#ifndef TOTORO_FORWARDER_H
#define TOTORO_FORWARDER_H

#include "Connection.h"
#include "core/SSLSocket.h"
#include "HttpBase.h"
#include "HttpsBase.h"

namespace totoro {

    class Forwarder{
    protected:
        virtual int InitForwarder(const std::string& ip,unsigned port) = 0;
        virtual Connection::CallbackReturnType ForwarderReadCallback() = 0;
        virtual Connection::CallbackReturnType ForwarderAfterReadCallback() = 0;
        virtual Connection::CallbackReturnType ForwarderWriteCallback() = 0;
        virtual Connection::CallbackReturnType ForwarderAfterWriteCallback() = 0;
    };

    class HttpForwarder : public HttpBase,public Forwarder {
        HttpBase forwarder;
    protected:
        int InitForwarder(const std::string &ip, unsigned int port) override;
        CallbackReturnType ForwarderReadCallback() override;
        CallbackReturnType ForwarderAfterReadCallback() override;
        CallbackReturnType ForwarderWriteCallback() override;
        CallbackReturnType ForwarderAfterWriteCallback() override;
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;
    };

    class HttpsForwarder : public HttpsBase,public Forwarder {
        HttpsBase forwarder;
    protected:
        int InitForwarder(const std::string &ip, unsigned int port) override;
        CallbackReturnType ForwarderReadCallback() override;
        CallbackReturnType ForwarderAfterReadCallback() override;
        CallbackReturnType ForwarderWriteCallback() override;
        CallbackReturnType ForwarderAfterWriteCallback() override;
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;
    };

} // totoro

#endif //TOTORO_FORWARDER_H
