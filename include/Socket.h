#ifndef TOTOROSERVER_SOCKET_H
#define TOTOROSERVER_SOCKET_H

#include <unistd.h>         /* close */
#include <arpa/inet.h>      /* socket addr */
#include <string>           /* string */
#include "AsyncLogger.h"    /* AsyncLogger */
#include <sys/stat.h>       /* stat */
#include <vector>

namespace totoro {
#define SOCKET_BUF_SIZE 4096

    class Socket {
    protected:
        bool isNew                          {true};
        int type                            {SOCK_STREAM};
        char readBuffer[SOCKET_BUF_SIZE]  = {0};
        char writeBuffer[SOCKET_BUF_SIZE] = {0};
        size_t readCursor                   {0};
        size_t writeCursor                  {0};
        size_t readTotalBytes               {0};
        size_t writeTotalBytes              {0};
        int file                            {-1};
        struct stat stat                    {};
        int sock                            {-1};
        sockaddr_in myAddr                  {};
        sockaddr_in destAddr                {};
        socklen_t   destAddrLen             {};

        virtual int sendImpl(const char* data) = 0;
        virtual int recvImpl(std::string& data) = 0;
    public:
        virtual ~Socket();
        virtual bool Init(const std::string& ip,short port) = 0;
        virtual inline bool Bind() = 0;
        virtual inline bool Listen() = 0;
        virtual int SendWithHeader(const char* data,size_t size);
        virtual int SendWithHeader(std::string& data);
        virtual int SendWithHeader(std::string&& data);
        virtual int SendAll(std::string& data) = 0;
        virtual int Send(std::string& data,size_t size) = 0;
        virtual int Send(std::string&& data,size_t size) = 0;
        virtual int Send(const char* data,size_t size) = 0;
        virtual int SendFile(const std::string& filePath) = 0;

        virtual int RecvWithHeader(std::string& data);
        virtual int Recv(std::string& data,size_t size) = 0;
        virtual bool RecvAll(std::string& data) = 0;
        virtual int RecvFile(const std::string& filePath) = 0;

        virtual void Close();
    };

    class TCPSocket : public Socket {
    #pragma pack(1)
        struct header{
            size_t size;
        };
    #define TCP_HEADER_SIZE sizeof(header)
    #pragma pack()
    protected:
        int sendImpl(const char* data) override;
        int recvImpl(std::string& data) override;
    public:
        TCPSocket();
        ~TCPSocket() override;
        bool Init(const std::string& ip,short port) override;
        void Init();
        inline bool Bind() override;
        inline bool Listen() override;
        bool Accept(TCPSocket& tcpSocket);
        bool Connect(const std::string& ip,short port);
        int SendWithHeader(const char* data,size_t size) override;
        int SendWithHeader(std::string& data) override;
        int SendWithHeader(std::string&& data) override;
        int SendAll(std::string& data) override;
        int Send(std::string& data,size_t size) override;
        int Send(std::string&& data,size_t size) override;
        int Send(const char* data,size_t size) override;
        int SendFile(const std::string& filePath) override;

        int RecvWithHeader(std::string& data) override;
        int Recv(std::string& data,size_t size) override;
        bool RecvAll(std::string& data) override;
        int RecvFile(const std::string& filePath) override;

        void Close() override;
    };

    class UDPSocket : public Socket {
    protected:
        int sendImpl(const char* data) override;
        int recvImpl(std::string& data) override;
    public:
        UDPSocket();
        ~UDPSocket() override;
        bool Init(const std::string& ip,short port) override;
        void SetDestAddr(const std::string& ip,short port);
        inline bool Bind() override;
        inline bool Listen() override;
        int SendWithHeader(const char* data,size_t size) override;
        int SendWithHeader(std::string& data) override;
        int SendWithHeader(std::string&& data) override;
        int SendAll(std::string& data) override;
        int Send(std::string& data,size_t size) override;
        int Send(std::string&& data,size_t size) override;
        int Send(const char* data,size_t size) override;
        int SendFile(const std::string& filePath) override;

        int RecvWithHeader(std::string& data) override;
        int Recv(std::string& data,size_t size) override;
        bool RecvAll(std::string& data) override;
        int RecvFile(const std::string& filePath) override;

        void Close() override;
    };
} // totoro

#endif //TOTOROSERVER_SOCKET_H
