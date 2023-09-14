#ifndef TOTORO_FORWARDHTTPPROXY_H
#define TOTORO_FORWARDHTTPPROXY_H

#include "ProxyBase.h"
#include "HttpBase.h"

namespace totoro {
    /**
     * @brief 负责HTTP正向代理事务 / Response HTTP forward proxy transactions
     */
    class ForwardHttpProxy : public ProxyBase,public virtual HttpBase {
    public:
        int Init(const ConnectionInitParameter &connectionInitParameter) override;

        int Close() override;

    protected:
        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
        int MainReadCallback() override;
        int MainAfterReadCallback() override;
        int ForwardReadCallback() override;
        int ForwardAfterReadCallback() override;
        int ForwardWriteCallback() override;
        int ForwardAfterWriteCallback() override;
        int MainWriteCallback() override;
        int MainAfterWriteCallback() override;
    };

} // totoro

#endif //TOTORO_FORWARDHTTPPROXY_H
