#ifndef TOTOROSERVER_SOCKET_H
#define TOTOROSERVER_SOCKET_H

#include <unistd.h>         /* close */
#include <arpa/inet.h>      /* socket addr */
#include <string>           /* string */
#include "AsyncLogger.h"    /* AsyncLogger */
#include <sys/stat.h>       /* stat */

namespace totoro {
#define SOCKET_BUF_SIZE 4096

    class Socket {
    protected:
        bool isNew                          {true};
        int type                            {SOCK_STREAM};
        char readBuffer[SOCKET_BUF_SIZE]  = {0};
        size_t readCursor                   {0};
        size_t writeCursor                  {0};
        size_t readTotalBytes               {0};
        size_t writeTotalBytes              {0};
        int file                            {-1};
        struct stat stat                    {};

        virtual int sendImpl(const char* data) = 0;
        virtual int recvImpl(std::string& data) = 0;
    public:
        int sock                            {-1};
        sockaddr_in myAddr                  {};
        virtual ~Socket() = default;
        virtual bool Init(const std::string& ip,short port) = 0;
        virtual inline bool Bind() = 0;
        virtual inline bool Listen() = 0;
        virtual int SendWithHeader(const char* data);
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
        virtual int RecvFile(const std::string& filePath,size_t size) = 0;
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
        void Init(int sock,sockaddr_in& addr);
        inline bool Bind() override;
        inline bool Listen() override;
        bool Connect(const std::string& ip,short port);
        int SendWithHeader(const char* data) override;
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
        int RecvFile(const std::string& filePath,size_t size) override;
    };

    class UDPSocket : public Socket {
        #pragma pack(1)
        #define UDP_START 0
        #define UDP_END 1
        #define UDP_DATA 2
        #define UDP_ACK 3
        struct header{
            uint8_t type;
            uint16_t length;
            uint32_t sequence;
            uint32_t checksum;
        };
        #define UDP_HEADER_SIZE sizeof(header)

        #define PAYLOAD_SIZE 1461
        #define PACKET_SIZE 1500
        struct packet{
            header header;
            char payload[PAYLOAD_SIZE];
        };
        #pragma pack()
        static void initPacket(packet& packet,uint8_t type,uint16_t length,uint32_t sequence);
        static bool checkPacket(packet& packet,uint8_t type);
        static uint32_t checksum(const void* packet,size_t nBytes);
        static inline uint64_t nowUs();

        sockaddr_in destAddr        {};
        socklen_t   destAddrLen     {};
    protected:
        int sendImpl(const char* data) override;
        int recvImpl(std::string& data) override;
    public:
        UDPSocket();
        ~UDPSocket() override;
        bool Init(const std::string& ip,short port) override;
        inline bool Bind() override;
        inline bool Listen() override;
        int SendWithHeader(const char* data) override;
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
        int RecvFile(const std::string& filePath,size_t size) override;
    };
} // totoro

#endif //TOTOROSERVER_SOCKET_H
