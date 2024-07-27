#ifndef TOTORO_SSLSOCKET_H
#define TOTORO_SSLSOCKET_H

#include <openssl/ssl.h>        /* SSL */
#include <openssl/err.h>        /* SSL */
#include "utils/Socket.h"             /* Socket */

namespace totoro {
    /**
     * @brief SSL服务器端上下文 \n SSL Server Context
     */
    struct SSLContext {
        SSL_CTX *context{nullptr};

        SSLContext(const std::string &CA, const std::string &CERT, const std::string &PRIVATE);

        ~SSLContext();
    };

    /**
     * @brief SSL客户端上下文 \n SSL Client Context
     */
    struct SSLClientContext {
        SSL_CTX *context{nullptr};

        SSLClientContext(const std::string &CA, const std::string &CERT, const std::string &PRIVATE);

        ~SSLClientContext();
    };

    /**
     * @brief 负责SSL事务相关连接 \n Response SSL connection transactions
     */
    class SSLSocket : virtual public TcpClient {

        static SSLContext &GetContext();

    protected:
        ::SSL *connection{nullptr};

        static SSLClientContext &GetClientContext();

    public:
        int InitSSL();

        ~SSLSocket();

        int Close() override;

        size_t Send(const char *data, size_t size) override;

        size_t SendAll(const std::string& data) override;

        bool SendFile(const std::string &file_path) override;

        size_t Recv(std::string &data, size_t size) override;

        size_t RecvUntil(std::string &data, const char *key) override;

        // 连接套接字 / connect socket
        bool Connect(const std::string &ip, unsigned short port,unsigned int timeout = 0) override;

    };

} // totoro

#endif //TOTORO_SSLSOCKET_H
