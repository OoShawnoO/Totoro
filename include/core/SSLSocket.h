#ifndef TOTORO_SSLSOCKET_H
#define TOTORO_SSLSOCKET_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "Socket.h"

namespace totoro {
    struct SSLContext {
        SSL_CTX* context;
        SSLContext(const std::string& CERT,const std::string& PRIVATE);
        ~SSLContext();
    };

    class SSLSocket : virtual public TCPSocket{

        static SSLContext& GetContext();
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
