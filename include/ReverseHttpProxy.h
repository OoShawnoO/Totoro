#ifndef TOTORO_REVERSEHTTPPROXY_H
#define TOTORO_REVERSEHTTPPROXY_H

#include "HttpBase.h"
#include "ProxyBase.h"

namespace totoro {
    /**
     * @brief 负责HTTP反向代理事务 / Response HTTP reverse proxy transactions
     */
    class ReverseHttpProxy : public virtual ProxyBase,public virtual HttpBase{
        // 获取反向代理服务器ip与端口 / Get reverse proxy server ip address and port
        static const std::pair<std::string, unsigned short>& GetReverseProxyServer(const std::string& port);
    public:
        // 初始化连接 / Init connection
        int Init(const ConnectionInitParameter &connectionInitParameter) override;
        // 关闭连接 / Close connection
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
