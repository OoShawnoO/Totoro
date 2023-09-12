#ifndef TOTORO_REVERSEHTTPPROXY_H
#define TOTORO_REVERSEHTTPPROXY_H

#include "HttpBase.h"
#include "ProxyBase.h"

namespace totoro {

    class ReverseHttpProxy : public ProxyBase,public HttpBase{
        static const std::pair<std::string, unsigned short>& GetReverseProxyServer(const std::string& port);
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

#endif //TOTORO_REVERSEHTTPPROXY_H
