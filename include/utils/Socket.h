#ifndef TOTORO_SOCKET_H
#define TOTORO_SOCKET_H

#include <unistd.h>         /* close */
#include <arpa/inet.h>      /* socket addr */
#include <string>           /* string */
#include "Mole.h"           /* AsyncLogger */
#include <sys/stat.h>       /* stat */
#include <vector>           /* vector */

namespace totoro {

#define SOCKET int
#define BAD_SOCKET (-1)

    class Socket {
    protected:
        // 套接字文件描述符 / socket file descriptor
        SOCKET sock{BAD_SOCKET};
        // 套接字本地地址 / socket local address
        sockaddr_in local_address{};
        // 套接字目的地址 / socket destination address
        sockaddr_in destination_address{};
    public:
        Socket() = default;

        // 构造函数 / constructor
        explicit Socket(SOCKET sock_fd, sockaddr_in local_addr, sockaddr_in dest_addr)
                : sock(sock_fd), local_address(local_addr), destination_address(dest_addr) {};

        // 关闭套接字 / close socket
        virtual int Close();

        SOCKET Sock() const;
    };

    class TcpListener;

    class TcpSocket : virtual public Socket {
        friend class TcpListener;

    protected:
        std::string cache;
    public:
        // 构造函数 / constructor
        TcpSocket() = default;

        // 构造函数 / constructor
        explicit TcpSocket(SOCKET sock_fd, const sockaddr_in &local_addr = {}, const sockaddr_in &dest_addr = {})
                : Socket(sock_fd, local_addr, dest_addr) {}

        // 初始化 / initialize
        void Init(SOCKET sock_fd, const sockaddr_in &local_addr = {}, const sockaddr_in &dest_addr = {});

        // 发送数据 / send data
        virtual size_t Send(const char *data, size_t size);

        // 发送数据 / send data
        virtual size_t SendAll(const std::string &data);

        // 发送文件 / send file
        virtual bool SendFile(const std::string &file_path);

        // 接收数据 / recv data
        virtual size_t Recv(std::string &data, size_t size);

        // 接收数据直到关键字符串停止 / recv data until key
        virtual size_t RecvUntil(std::string &data, const char *key);

    };


    class TcpListener : public Socket {
    public:
        // 构造函数 / constructor
        TcpListener(const std::string &ip, unsigned short port);

        // 绑定 / bind
        bool Bind();

        // 监听 / listen
        bool Listen();

        // 接收新连接 / accept new socket
        bool Accept(TcpSocket &socket);
    };

    class TcpClient : virtual public TcpSocket {
    public:
        // 连接套接字 / connect socket
        virtual bool Connect(const std::string &ip, unsigned short port);
    };

} // totoro

#endif //TOTORO_SOCKET_H
