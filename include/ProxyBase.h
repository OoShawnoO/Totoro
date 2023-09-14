#ifndef TOTOROSERVER_PROXYBASE_H
#define TOTOROSERVER_PROXYBASE_H

#include "Connection.h"     /* Connection */

namespace totoro {
    /**
     * @brief 负责代理请求事务，转发至目标IP和端口 \n Responsible for proxy request transactions, forwarding to the target IP and port
     */
    class ProxyBase : public virtual Connection {
    protected:
        // 转发连接 / Forward connection
        TCPSocket forwardSocket;

        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
        // 客户端请求读回调 / Client read event callback
        virtual int MainReadCallback();
        // 客户端请求读后回调 / After client read event callback
        virtual int MainAfterReadCallback();
        // 转发连接读回调 / Forward connection read event callback
        virtual int ForwardReadCallback();
        // 转发连接读后回调 / After forward connection read event callback
        virtual int ForwardAfterReadCallback();
        // 转发连接写回调 / Forward connection write event callback
        virtual int ForwardWriteCallback();
        // 转发连接写后回调 / After Forward connection write event callback
        virtual int ForwardAfterWriteCallback();
        // 客户端请求写回调 / Client write event callback
        virtual int MainWriteCallback();
        // 客户端请求写后回调 / After client write event callback
        virtual int MainAfterWriteCallback();
        // 添加转发连接 / Add forward connection
        bool AddForward();
    public:
        int Close() override;
    };

} // totoro

#endif //TOTOROSERVER_PROXYBASE_H
