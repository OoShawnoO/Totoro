#include <cstring>          /* memcpy */
#include <fcntl.h>          /* fcntl */
#include <sys/sendfile.h>   /* sendfile */
#include "core/Socket.h"         /* Socket */
static const std::string SocketChan = "Socket";
namespace totoro {
    int Socket::SendWithHeader(const char *data,size_t size) {
        return 1;
    }

    int Socket::SendWithHeader(const std::string &data) {
        return 1;
    }

    int Socket::SendWithHeader(std::string &&data) {
        return 1;
    }

    int Socket::RecvWithHeader(std::string &data,bool isAppend) {
        return 1;
    }

    int Socket::Close() {
        if(sock != BAD_FILE_DESCRIPTOR){ close(sock); sock = BAD_FILE_DESCRIPTOR;}
        if(file != BAD_FILE_DESCRIPTOR){ close(file); file = BAD_FILE_DESCRIPTOR;}
        return -1;
    }

    Socket::~Socket() {
        Socket::Close();
    }

    void Socket::Moveto(Socket &s) {
        s = *this;
        sock = BAD_FILE_DESCRIPTOR;
        file = BAD_FILE_DESCRIPTOR;
    }


    int Socket::Sock() const {
        return sock;
    }

    sockaddr_in Socket::DestAddr() const {
        return destAddr;
    }

    sockaddr_in Socket::Addr() const{
        return myAddr;
    }

    void Socket::Init(int _sock, sockaddr_in _myAddr, sockaddr_in _destAddr) {
        sock = _sock;
        myAddr = _myAddr;
        myPort = ntohs(_myAddr.sin_port);
        destAddr = _destAddr;
    }

    /* TCP Impl */
    TCPSocket::TCPSocket() {
        type = SOCK_STREAM;
    }
    /**
     * @return 0 again
     * @return 1 true
     * @return -1 false
     */
    int TCPSocket::sendImpl(const char *data) {
        size_t needSend;
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            needSend = writeTotalBytes - writeCursor;
            if((hadSend = ::send(sock,data + writeCursor,needSend,MSG_NOSIGNAL)) <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = true;
                    return 0;
                }
                isNew = true;
                return -1;
            }
            writeCursor += hadSend;
        }
        writeCursor = 0;
        writeTotalBytes = 0;
        isNew = true;
        return 1;
    }
    /**
     * @return 0 again
     * @return 1 true
     * @return -1 false
     */
    int TCPSocket::recvImpl(std::string &data) {
        if(readTotalBytes <= 0) return false;
        ssize_t hadRecv;
        size_t needRecv;
        if(readTotalBytes != SIZE_MAX && data.size() < readTotalBytes){
            data.reserve(readTotalBytes);
        }
        int flag = readTotalBytes == SIZE_MAX ? MSG_DONTWAIT : 0;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = ::recv(sock,buffer,needRecv,flag)) <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    if(data.empty()) continue;
                    isNew = true;
                    return 0;
                }
                if(hadRecv == 0 && readTotalBytes == SIZE_MAX) {
                    shutdown(sock,SHUT_RDWR);
                    isNew = true;
                    return 0;
                }
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            data.append(buffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        return 1;
    }

    TCPSocket::~TCPSocket() {
        TCPSocket::Close();
    }

    bool TCPSocket::Init(const std::string &ip, short port) {
        if(sock != -1) {
            close(sock);
            sock = -1;
        }
        if((sock = socket(AF_INET,type,0)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        int reuse = 1;
        if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) < 0){
            LOG_ERROR(SocketChan, strerror(errno));
            return false;
        }
        if(setsockopt(sock,SOL_SOCKET,SO_REUSEPORT,&reuse,sizeof(reuse)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }

        myAddr.sin_family = AF_INET;
        myAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        myAddr.sin_port = htons(port);
        return true;
    }

    bool TCPSocket::Bind() {
        if(sock == -1){
            LOG_ERROR(SocketChan,"please bind after create socket.");
            return false;
        }
        if(bind(sock,(sockaddr*)&myAddr,sizeof(myAddr)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        return true;
    }

    bool TCPSocket::Listen() {
        if(sock == -1){
            LOG_ERROR(SocketChan,"please listen after create socket.");
            return false;
        }
        if(listen(sock,1024) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        return true;
    }

     bool TCPSocket::Accept(TCPSocket& tcpSocket) {
        tcpSocket.sock = BAD_FILE_DESCRIPTOR;
        bzero(&tcpSocket.destAddr,sizeof(tcpSocket.destAddr));
        tcpSocket.destAddrLen = sizeof(tcpSocket.destAddr);
        if((tcpSocket.sock = accept(sock,(sockaddr*)&tcpSocket.destAddr,&tcpSocket.destAddrLen)) < 0){
            LOG_ERROR(SocketChan, strerror(errno));
            return false;
        }

        tcpSocket.myAddr = myAddr;
        return true;
    }

    bool TCPSocket::Connect(const std::string& ip, unsigned short port) {
        if(sock != -1) {
            close(sock);
            sock = -1;
        }
        if((sock = socket(AF_INET,type,0)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        sockaddr_in destAddr{};
        destAddr.sin_port = htons(port);
        destAddr.sin_family = AF_INET;
        destAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        if(connect(sock,(sockaddr*)&destAddr,sizeof(destAddr)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        return true;
    }

    void TCPSocket::Init() {
        if(sock != -1) {
            close(sock);
            sock = -1;
        }
        if((sock = socket(AF_INET,type,0)) < 0 ){
            LOG_ERROR(SocketChan, strerror(errno));
            return;
        }
    }

    int TCPSocket::SendWithHeader(const char *data,size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            header h{writeTotalBytes};
            if(::send(sock,&h,TCP_HEADER_SIZE,0) <= 0) return -1;
            isNew = false;
        }
        return TCPSocket::sendImpl(data);
    }

    int TCPSocket::SendWithHeader(const std::string &data) {
        return SendWithHeader(data.c_str(),data.size());
    }

    int TCPSocket::SendWithHeader(std::string &&data) {
        return SendWithHeader(data.c_str(),data.size());
    }

    int TCPSocket::SendAll(const std::string &data) {
        if(isNew){
            writeTotalBytes = data.size();
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return TCPSocket::sendImpl(data.c_str());
    }

    int TCPSocket::Send(const char *data, size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return TCPSocket::sendImpl(data);
    }

    int TCPSocket::Send(const std::string &data, size_t size) {
        return Send(data.c_str(),size);
    }

    int TCPSocket::Send(std::string &&data, size_t size) {
        return Send(data.c_str(),size);
    }

    int TCPSocket::SendFileWithHeader(const std::string &filePath) {
        if(isNew){
            file = open(filePath.c_str(),O_RDONLY);
            if(file < 0){
                LOG_ERROR(SocketChan,strerror(errno));
                return -1;
            }
            memset(&stat,0,sizeof(stat));
            fstat(file,&stat);
            writeTotalBytes = stat.st_size;
            writeCursor = 0;
            if(send(sock,&writeTotalBytes,sizeof(writeTotalBytes),0) < 0){
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
        }
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            auto offset = (off_t)writeCursor;
            if((hadSend = sendfile(sock,file,&offset,writeTotalBytes - writeCursor)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                LOG_ERROR(SocketChan, strerror(errno));
                close(file);
                file = -1;
                return -1;
            }
            writeCursor += hadSend;
        }
        close(file);
        file = -1;
        isNew = true;
        return 1;
    }

    int TCPSocket::SendFile(const std::string &filePath) {
        if(isNew){
            file = open(filePath.c_str(),O_RDONLY);
            if(file < 0){
                LOG_ERROR(SocketChan,strerror(errno));
                return -1;
            }
            memset(&stat,0,sizeof(stat));
            fstat(file,&stat);
            writeTotalBytes = stat.st_size;
            writeCursor = 0;
        }
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            auto offset = (off_t)writeCursor;
            if((hadSend = sendfile(sock,file,&offset,writeTotalBytes - writeCursor)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                LOG_ERROR(SocketChan, strerror(errno));
                close(file);
                file = -1;
                return -1;
            }
            writeCursor += hadSend;
        }
        close(file);
        file = -1;
        isNew = true;
        return 1;
    }

    int TCPSocket::RecvWithHeader(std::string &data,bool isAppend) {
        if(isNew){
            if(!isAppend) data.clear();
            header h{};
            if(::recv(sock,&h,TCP_HEADER_SIZE,0) <= 0){
                return -1;
            }
            readTotalBytes = h.size;
            readCursor = 0;
            isNew = false;
        }
        return TCPSocket::recvImpl(data);
    }

    int TCPSocket::Recv(std::string &data, size_t size,bool isAppend) {
        if(isNew){
            if(!isAppend) data.clear();
            readTotalBytes = size;
            readCursor = 0;
            isNew = false;
        }
        return TCPSocket::recvImpl(data);
    }

    bool TCPSocket::RecvAll(std::string &data,bool isAppend) {
        if(!isAppend) data.clear();
        readTotalBytes = SIZE_MAX;
        readCursor = 0;

        return TCPSocket::recvImpl(data) == 0;
    }

    int TCPSocket::RecvFileWithHeader(const std::string &filePath) {
        if(isNew){
            size_t size;
            if(recv(sock,&size,sizeof(size),0) < 0){
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            readCursor = 0;
            readTotalBytes = size;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY);
            if(file < 0){
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            isNew = false;
        }
        ssize_t hadRecv;
        size_t needRecv;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = ::recv(sock,buffer,needRecv,0)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                close(file);
                file = -1;
                return -1;
            }
            if(write(file,buffer,hadRecv) != hadRecv){
                LOG_ERROR(SocketChan,"failed to write");
                return -1;
            }
            readCursor += hadRecv;
        }
        isNew = true;
        close(file);
        file = -1;
        return 1;
    }

    int TCPSocket::RecvFile(const std::string &filePath,size_t size) {
        if(isNew){
            readCursor = 0;
            readTotalBytes = size;
            isNew = false;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY);
        }
        ssize_t hadRecv;
        size_t needRecv;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = ::recv(sock,buffer,needRecv,0)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                close(file);
                file = -1;
                return -1;
            }
            write(file,buffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        close(file);
        file = -1;
        return 1;
    }

    int TCPSocket::Close() {
        return Socket::Close();
    }

    /* udp Impl*/
    UDPSocket::UDPSocket() {
        type = SOCK_DGRAM;
    }

    int UDPSocket::sendImpl(const char *data) {
        size_t needSend;
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            needSend = (writeTotalBytes - writeCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : writeTotalBytes - writeCursor;
            memcpy(buffer,data + writeCursor,needSend);
            if((hadSend = sendto(sock,buffer,needSend,
                                 MSG_NOSIGNAL,(sockaddr*)&destAddr,sizeof(destAddr))) < 0){
                if(errno == EWOULDBLOCK || errno == EAGAIN){
                    isNew = false;
                    return 0;
                }
                return -1;
            }
            writeCursor += hadSend;
        }
        isNew = true;
        return 1;
    }

    int UDPSocket::recvImpl(std::string &data) {
        if(readTotalBytes <= 0) return false;
        ssize_t hadRecv;
        size_t needRecv;
        if(readTotalBytes != SIZE_MAX && data.size() < readTotalBytes){
            data.reserve(readTotalBytes);
        }
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            memset(&destAddr,0,sizeof(destAddr));
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = ::recvfrom(sock,buffer,needRecv,0,(struct sockaddr*)&destAddr,&destAddrLen)) <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            data.append(buffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        return 1;
    }

    UDPSocket::~UDPSocket() {
        UDPSocket::Close();
    }

    bool UDPSocket::Init(const std::string &ip, short port) {
        if((sock = socket(AF_INET,type,0)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        myAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        myAddr.sin_port = htons(port);
        myAddr.sin_family = AF_INET;
        return true;
    }

    bool UDPSocket::Bind() {
        if(bind(sock,(sockaddr*)&myAddr,sizeof(myAddr)) < 0){
            LOG_ERROR(SocketChan,strerror(errno));
            return false;
        }
        return true;
    }

    void UDPSocket::SetDestAddr(const std::string &ip, short port){
        destAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        destAddr.sin_port = htons(port);
        destAddr.sin_family = type;
        destAddrLen = sizeof(destAddr);
    }

    bool UDPSocket::Init(const std::string &ip, short port, const std::string &destIP, short destPort) {
        SetDestAddr(destIP,destPort);
        return Init(ip,port);
    }

    int UDPSocket::SendWithHeader(const char *data,size_t size) {
        if(sendto(sock,&size,sizeof(size),0,(sockaddr*)&destAddr,destAddrLen) < 0){
            LOG_ERROR(SocketChan, strerror(errno));
            return -1;
        }
        return Send(data,size);
    }

    int UDPSocket::SendWithHeader(const std::string &data) {
        return SendWithHeader(data.c_str(),data.size());
    }

    int UDPSocket::SendWithHeader(std::string &&data) {
        return SendWithHeader(data.c_str(),data.size());
    }

    int UDPSocket::SendAll(const std::string &data) {
        if(isNew){
            writeTotalBytes = data.size();
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return UDPSocket::sendImpl(data.c_str());
    }

    int UDPSocket::Send(const std::string &data, size_t size) {
        return Send(data.c_str(),size);
    }

    int UDPSocket::Send(std::string &&data, size_t size) {
        return Send(data.c_str(),size);
    }

    int UDPSocket::Send(const char *data, size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes < 1) return -1;
            isNew = false;
        }
        return UDPSocket::sendImpl(data);
    }

    int UDPSocket::SendFileWithHeader(const std::string &filePath) {
        if(isNew){
            file = open(filePath.c_str(),O_RDONLY);
            fstat(file,&stat);
            writeTotalBytes = stat.st_size;
            writeCursor = 0;
            if(sendto(sock,&writeTotalBytes,SOCKET_BUF_SIZE,0,(sockaddr*)&destAddr,destAddrLen) < 0){
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            isNew = false;
        }
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            ssize_t hadRead = read(file,buffer,SOCKET_BUF_SIZE);
            if(hadRead <= 0){
                return -1;
            }
            if((hadSend = sendto(sock,buffer,hadRead,0,(sockaddr*)&destAddr,sizeof(destAddr))) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
                return -1;
            }
            writeCursor += hadSend;
        }
        close(file);
        file = -1;
        isNew = true;
        return 1;
    }

    int UDPSocket::SendFile(const std::string &filePath) {
        if(isNew){
            file = open(filePath.c_str(),O_RDONLY);
            fstat(file,&stat);
            writeTotalBytes = stat.st_size;
            writeCursor = 0;
            isNew = false;
        }
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            ssize_t hadRead = read(file,buffer,SOCKET_BUF_SIZE);
            if(hadRead <= 0){
                return -1;
            }
            if((hadSend = sendto(sock,buffer,hadRead,0,(sockaddr*)&destAddr,sizeof(destAddr))) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
                return -1;
            }
            writeCursor += hadSend;
        }
        close(file);
        file = -1;
        isNew = true;
        return 1;
    }


    int UDPSocket::RecvWithHeader(std::string &data,bool isAppend) {
        size_t size;
        if(recvfrom(sock,&size,sizeof(size_t),0,(sockaddr*)&destAddr,&destAddrLen) < 0){
            LOG_ERROR(SocketChan, strerror(errno));
            return -1;
        }
        return Recv(data,size,isAppend);
    }

    int UDPSocket::Recv(std::string &data, size_t size,bool isAppend) {
        if(isNew){
            if(!isAppend) data.clear();
            readCursor = 0;
            readTotalBytes = size;
            if(readTotalBytes < 1) return -1;
            isNew = false;
        }
        return UDPSocket::recvImpl(data);
    }

    bool UDPSocket::RecvAll(std::string &data,bool isAppend) {
        if(isNew){
            if(!isAppend) data.clear();
            readCursor = 0;
            readTotalBytes = SIZE_MAX;
            if(readTotalBytes < 1) return -1;
            isNew = false;
        }
        return UDPSocket::recvImpl(data) == 0;
    }

    int UDPSocket::RecvFileWithHeader(const std::string &filePath) {
        if(isNew){
            size_t size;
            if(recvfrom(sock,&size,sizeof(size),0,(sockaddr*)&destAddr,&destAddrLen) < 0){
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            readCursor = 0;
            readTotalBytes = size;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY);
            if(file < 0){
                LOG_ERROR(SocketChan, strerror(errno));
                return -1;
            }
            isNew = false;
        }
        ssize_t hadRecv;
        size_t needRecv;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = readTotalBytes - readCursor > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : readTotalBytes - readCursor;
            if((hadRecv = ::recvfrom(sock,buffer,needRecv,0,(sockaddr*)&destAddr,&destAddrLen)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                close(file);
                file = -1;
                return -1;
            }
            if(write(file,buffer,hadRecv) != hadRecv){
                LOG_ERROR(SocketChan,"failed to write");
                return -1;
            }
            readCursor += hadRecv;
        }
        isNew = true;
        close(file);
        file = -1;
        return 1;
    }

    int UDPSocket::RecvFile(const std::string &filePath,size_t size) {
        if(isNew){
            readCursor = 0;
            readTotalBytes = size;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY);
            isNew = false;
        }
        ssize_t hadRecv;
        size_t needRecv;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = readTotalBytes - readCursor > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : readTotalBytes - readCursor;
            if((hadRecv = ::recvfrom(sock,buffer,needRecv,0,(sockaddr*)&destAddr,&destAddrLen)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                close(file);
                file = -1;
                return -1;
            }
            write(file,buffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        close(file);
        file = -1;
        return 1;
    }

    int UDPSocket::Close() {
        return Socket::Close();
    }

} // totoro