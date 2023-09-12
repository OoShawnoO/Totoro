#ifndef TOTORO_FORWARDHTTPPROXY_H
#define TOTORO_FORWARDHTTPPROXY_H

#include "ProxyBase.h"
#include "HttpBase.h"

namespace totoro {

    class ForwardHttpProxy : public ProxyBase,public HttpBase {
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
