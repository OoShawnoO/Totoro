#ifndef TOTORO_SSLSOCKET_H
#define TOTORO_SSLSOCKET_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "Socket.h"

namespace totoro {
    /**
     * @brief SSL服务器端上下文 \n SSL Server Context
     */
    struct SSLContext {
        SSL_CTX* context            {nullptr};
        SSLContext(const std::string& CA,const std::string& CERT,const std::string& PRIVATE);
        ~SSLContext();
    };

    /**
     * @brief SSL客户端上下文 \n SSL Client Context
     */
    struct SSLClientContext {
        SSL_CTX* context            {nullptr};
        SSLClientContext(const std::string& CA,const std::string& CERT,const std::string& PRIVATE);
        ~SSLClientContext();
    };
    /**
     * @brief 负责SSL事务相关连接 \n Response SSL connection transactions
     */
    class SSLSocket : public virtual TCPSocket{

        static SSLContext& GetContext();
        static SSLClientContext& GetClientContext();
        int sendImpl(const char *data) override;
        int recvImpl(std::string &data) override;
    protected:
        ::SSL* connection                       {nullptr};
    public:
        int InitSSL();
        ~SSLSocket() override;
        int SendAll(const std::string &data) override;
        int Send(const std::string &data, size_t size) override;
        int Send(std::string &&data, size_t size) override;
        int Send(const char *data, size_t size) override;
        int Recv(std::string &data, size_t size) override;

        bool Connect(const std::string &ip, unsigned short port) override;

        bool RecvAll(std::string &data) override;
        int SendWithHeader(const char *data, size_t size) override;
        int SendWithHeader(const std::string &data) override;
        int SendWithHeader(std::string &&data) override;
        int SendFileWithHeader(const std::string &filePath) override;
        int SendFile(const std::string &filePath) override;

        int RecvWithHeader(std::string &data) override;
        int RecvFileWithHeader(const std::string &filePath) override;
        int RecvFile(const std::string &filePath, size_t size) override;
        int Close() override;

    };

} // totoro

#endif //TOTORO_SSLSOCKET_H
