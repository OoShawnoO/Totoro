#ifndef TOTORO_FORWARDER_H
#define TOTORO_FORWARDER_H

#include "core/Connection.h"
#include "core/SSLSocket.h"
#include "core/HttpBase.h"
#include "core/HttpsBase.h"
#include "core/Acceptor.h"
#include "core/ForwardEpoller.h"

namespace totoro {

    class Forwarder{
    protected:
        virtual int InitForwarder() = 0;
        virtual Connection::CallbackReturnType ForwarderReadCallback() = 0;
        virtual Connection::CallbackReturnType ForwarderAfterReadCallback() = 0;
        virtual Connection::CallbackReturnType ForwarderWriteCallback() = 0;
        virtual Connection::CallbackReturnType ForwarderAfterWriteCallback() = 0;
    };

    class HttpReverseForwarder : public HttpBase,public Forwarder {
        static const std::pair<std::string,unsigned short>& GetHttpForwardAddress(const std::string& port);

        HttpBase forwarder;
    protected:
        int InitForwarder() override;
        CallbackReturnType ForwarderReadCallback() override;
        CallbackReturnType ForwarderAfterReadCallback() override;
        CallbackReturnType ForwarderWriteCallback() override;
        CallbackReturnType ForwarderAfterWriteCallback() override;
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;
    public:
        int Close() override;
    };

    class HttpsReverseForwarder : public HttpsBase,public Forwarder {
        static const std::pair<std::string,unsigned short>& GetHttpsForwardAddress(const std::string& port);

        HttpsBase forwarder;
    protected:
        int InitForwarder() override;
        CallbackReturnType ForwarderReadCallback() override;
        CallbackReturnType ForwarderAfterReadCallback() override;
        CallbackReturnType ForwarderWriteCallback() override;
        CallbackReturnType ForwarderAfterWriteCallback() override;
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;
    public:
        int Close() override;
    };

    class HttpForwardForwarder : public HttpBase,public Forwarder {
        HttpBase forwarder;
    protected:
        int InitForwarder() override;
        CallbackReturnType ForwarderReadCallback() override;
        CallbackReturnType ForwarderAfterReadCallback() override;
        CallbackReturnType ForwarderWriteCallback() override;
        CallbackReturnType ForwarderAfterWriteCallback() override;
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;
    public:
        int Close() override;
    };

    class HttpsForwardForwarder : public HttpsBase,public Forwarder {
        HttpsBase forwarder;
    protected:
        int InitForwarder() override;
        CallbackReturnType ForwarderReadCallback() override;
        CallbackReturnType ForwarderAfterReadCallback() override;
        CallbackReturnType ForwarderWriteCallback() override;
        CallbackReturnType ForwarderAfterWriteCallback() override;
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;
    public:
        int Close() override;
    };

    using HttpReverseServer = Acceptor<ForwardEpoller<HttpReverseForwarder>>;
    using HttpsReverseServer = Acceptor<ForwardEpoller<HttpsReverseForwarder>>;
    using HttpForwardServer = Acceptor<ForwardEpoller<HttpForwardForwarder>>;
    using HttpsForwardServer = Acceptor<ForwardEpoller<HttpsForwardForwarder>>;

} // totoro

#endif //TOTORO_FORWARDER_H
