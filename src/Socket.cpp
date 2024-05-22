#include <cstring>          /* memcpy */
#include <fcntl.h>          /* fcntl */
#include <sys/uio.h>        /* writev */
#include <sys/sendfile.h>   /* sendfile */
#include "utils/Socket.h"         /* Socket */

static const std::string SocketChan = "Totoro";
namespace totoro {
    int Socket::Close() {
        if(sock == BAD_SOCKET) return BAD_SOCKET;
        close(sock);
        SOCKET temp = sock;
        sock = BAD_SOCKET;
        return temp;
    }

    SOCKET Socket::Sock() const {
        return sock;
    }

    void TcpSocket::Init(int sock_fd, const sockaddr_in& local_addr, const sockaddr_in& dest_addr) {
        sock = sock_fd;
        local_address = local_addr;
        destination_address = dest_addr;
    }

    size_t TcpSocket::Send(const char *data, size_t size) {
        size_t needSend;
        ssize_t hadSend;
        size_t write_cursor = 0;
        while(write_cursor < size){
            needSend = size - write_cursor;
            if((hadSend = ::send(sock,data + write_cursor,needSend,MSG_NOSIGNAL)) <= 0){
                MOLE_ERROR(SocketChan, strerror(errno));
                return write_cursor;
            }
            write_cursor += hadSend;
        }
        return size;
    }

    size_t TcpSocket::SendAll(const std::string &data) { return Send(data.c_str(),data.size()); }

    bool TcpSocket::SendFile(const std::string &file_path) {
        auto file = open(file_path.c_str(),O_RDONLY);
        struct stat stat{};
        if(file < 0){
            MOLE_ERROR(SocketChan,strerror(errno));
            return -1;
        }
        memset(&stat,0,sizeof(stat));
        fstat(file,&stat);
        size_t write_cursor = 0;
        ssize_t hadSend;
        while(write_cursor < stat.st_size){
            auto offset = (off_t)write_cursor;
            if((hadSend = sendfile(sock,file,&offset,stat.st_size - write_cursor)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                MOLE_ERROR(SocketChan, strerror(errno));
                close(file);
                return false;
            }
            write_cursor += hadSend;
        }
        close(file);
        return true;
    }

    size_t TcpSocket::Recv(std::string &data, size_t size) {
        if(cache.size() >= size) {
            data = cache.substr(0,size);
            cache = cache.substr(size);
            return size;
        }else{
            data = std::move(cache);
            size -= cache.size();
            cache.clear();
        }
        ssize_t hadRecv;
        size_t needRecv;
        size_t read_cursor = 0;
        char buffer[4096] = {0};
        while(read_cursor < size){
            bzero(buffer,4096);
            needRecv = size - read_cursor;
            if((hadRecv = ::recv(sock,buffer,needRecv,MSG_DONTWAIT)) <= 0){
                MOLE_ERROR(SocketChan, strerror(errno));
                return read_cursor;
            }
            data.append(buffer,hadRecv);
            read_cursor += hadRecv;
        }
        return size;
    }

    size_t TcpSocket::RecvUntil(std::string &data, const char *key) {
        if(strlen(key) == 0) return 0;
        ssize_t hadRecv;
        size_t read_cursor = 0;
        size_t pos;
        if((pos = cache.find(key)) != std::string::npos) {
            data = cache.substr(0,pos + strlen(key));
            cache = cache.substr(pos + strlen(key));
            return pos + strlen(key);
        }
        char buffer[4096] = {0};
        while(true){
            bzero(buffer,4096);
            if((hadRecv = ::recv(sock,buffer,4096,MSG_DONTWAIT)) <= 0){
                MOLE_ERROR(SocketChan, strerror(errno));
                return 0;
            }
            cache.append(buffer,hadRecv);
            if((pos = cache.find(key)) != std::string::npos) {
                data = cache.substr(0,pos + strlen(key));
                cache = cache.substr(pos + strlen(key));
                return pos + strlen(key);
            }
            read_cursor += hadRecv;
        }
    }


    TcpListener::TcpListener(const std::string &ip, unsigned short port) {
        Close();

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            MOLE_ERROR(SocketChan, strerror(errno));
            exit(-1);
        }
        local_address.sin_addr.s_addr = inet_addr(ip.c_str());
        local_address.sin_port = htons(port);
        local_address.sin_family = AF_INET;
        int reuse = 1;
        if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) < 0){
            MOLE_ERROR(SocketChan, strerror(errno));
        }
        if(setsockopt(sock,SOL_SOCKET,SO_REUSEPORT,&reuse,sizeof(reuse)) < 0){
            MOLE_ERROR(SocketChan,strerror(errno));
        }
    }

    bool TcpListener::Bind() {
        if(bind(sock,(sockaddr*)&local_address,sizeof(local_address)) < 0){
            MOLE_ERROR(SocketChan,strerror(errno));
            return false;
        }
        return true;
    }

    bool TcpListener::Listen() {
        if(sock == BAD_SOCKET){
            MOLE_ERROR(SocketChan,"please listen after create socket.");
            return false;
        }
        if(listen(sock,1024) < 0){
            MOLE_ERROR(SocketChan,strerror(errno));
            return false;
        }
        return true;
    }

    bool TcpListener::Accept(TcpSocket &socket) {
        socklen_t len;
        if((socket.sock = accept(sock,(sockaddr*)&socket.destination_address,&len)) < 0){
            MOLE_ERROR(SocketChan, strerror(errno));
            return false;
        }
        return true;
    }

    bool TcpClient::Connect(const std::string &ip, unsigned short port) {
        Close();

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            MOLE_ERROR(SocketChan, strerror(errno));
            return false;
        }
        destination_address.sin_addr.s_addr = inet_addr(ip.c_str());
        destination_address.sin_port = htons(port);
        destination_address.sin_family = AF_INET;

        return connect(sock,(sockaddr*)&destination_address,sizeof(destination_address)) > 0;
    }
} // totoro