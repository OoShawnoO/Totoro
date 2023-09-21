#ifndef TOTORO_FORWARDER_H
#define TOTORO_FORWARDER_H

#include "core/HttpsBase.h"         /* HttpsBase*/
#include "core/Acceptor.h"          /* Acceptor */
#include "core/ForwardEpoller.h"    /* ForwardEpoller */

namespace totoro {
    /**
     * @brief 负责转发相关事务接口 \n Response for forward transaction interface
     */
    class Forwarder{
    protected:
        // 初始化转发者 / Initialize forwarder
        virtual int InitForwarder() = 0;
        // 转发者读回调函数 / Forwarder read callback
        virtual Connection::CallbackReturnType ForwarderReadCallback() = 0;
        // 转发者读后回调函数 / Forwarder after read callback
        virtual Connection::CallbackReturnType ForwarderAfterReadCallback() = 0;
        // 转发者写回调函数 / Forwarder write callback
        virtual Connection::CallbackReturnType ForwarderWriteCallback() = 0;
        // 转发者写后回调函数 / Forwarder after write callback
        virtual Connection::CallbackReturnType ForwarderAfterWriteCallback() = 0;
    };
    /**
     * @brief 负责HTTP反向代理事务 \n Response for HTTP reverse proxy transaction
     */
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
    /**
     * @brief 负责HTTPS反向代理事务 \n Response for HTTPS reverse proxy transaction
     */
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
    /**
     * @brief 负责HTTP正向代理事务 \n Response HTTP forward proxy transaction
     */
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
    /**
     * @brief 负责HTTPS正向代理事务 \n Response for HTTPS forward proxy transaction
     */
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

    /**
     * @brief HTTP反向代理服务器 \n HTTP reverse proxy server
     */
    using HttpReverseServer = Acceptor<ForwardEpoller<HttpReverseForwarder>>;
    /**
     * @brief HTTPS反向代理服务器 \n HTTPS reverse proxy server
     */
    using HttpsReverseServer = Acceptor<ForwardEpoller<HttpsReverseForwarder>>;
    /**
     * @brief HTTP正向代理服务器 \n HTTP forward proxy server
     */
    using HttpForwardServer = Acceptor<ForwardEpoller<HttpForwardForwarder>>;
    /**
     * @brief HTTPS正向代理服务器 \n HTTPS forward proxy server
     */
    using HttpsForwardServer = Acceptor<ForwardEpoller<HttpsForwardForwarder>>;

} // totoro

#endif //TOTORO_FORWARDER_H
