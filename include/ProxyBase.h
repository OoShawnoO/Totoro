#ifndef TOTOROSERVER_PROXYBASE_H
#define TOTOROSERVER_PROXYBASE_H

#include "Connection.h"     /* Connection */

namespace totoro {
    /**
     * @brief 负责代理请求事务，转发至目标IP和端口 \n Responsible for proxy request transactions, forwarding to the target IP and port
     */
    class ProxyBase : public virtual Connection {
    protected:
        TCPSocket forwardSocket;

        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
        virtual int MainReadCallback();
        virtual int MainAfterReadCallback();
        virtual int ForwardReadCallback();
        virtual int ForwardAfterReadCallback();
        virtual int ForwardWriteCallback();
        virtual int ForwardAfterWriteCallback();
        virtual int MainWriteCallback();
        virtual int MainAfterWriteCallback();
        bool AddForward();
    public:
        int Close() override;
    };

} // totoro

#endif //TOTOROSERVER_PROXYBASE_H
