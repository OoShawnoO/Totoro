#include <cstring>          /* memcpy */
#include <fcntl.h>          /* fcntl */
#include <sys/sendfile.h>   /* sendfile */
#include <sys/time.h>
#include "Socket.h"         /* Socket */

namespace totoro {
    int Socket::SendWithHeader(const char *data) {
        return 1;
    }

    int Socket::SendWithHeader(std::string &data) {
        return 1;
    }

    int Socket::SendWithHeader(std::string &&data) {
        return 1;
    }

    int Socket::RecvWithHeader(std::string &data) {
        return 1;
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
        size_t hadSend,needSend;
        while(writeCursor < writeTotalBytes){
            needSend = writeTotalBytes - writeCursor;
            if((hadSend = ::send(sock,data + writeCursor,needSend,MSG_NOSIGNAL)) <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
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
        while(readCursor < readTotalBytes){
            bzero(readBuffer,SOCKET_BUF_SIZE);
            needRecv = readTotalBytes - readCursor;
            if((hadRecv = ::recv(sock,readBuffer,needRecv,0)) <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                return -1;
            }
            data.append(readBuffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        return 1;
    }

    TCPSocket::~TCPSocket() {
        if(sock != -1){
            close(sock);
            sock = -1;
        }
    }

    bool TCPSocket::Init(const std::string &ip, short port) {
        if(sock != -1) {
            close(sock);
            sock = -1;
        }
        if((sock = socket(AF_INET,type,0)) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        myAddr.sin_family = AF_INET;
        myAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        myAddr.sin_port = htons(port);
    }

    bool TCPSocket::Bind() {
        if(sock == -1){
            LOG_ERROR("please bind after create socket.");
            return false;
        }
        if(bind(sock,(sockaddr*)&myAddr,sizeof(myAddr)) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        return true;
    }

    bool TCPSocket::Listen() {
        if(sock == -1){
            LOG_ERROR("please listen after create socket.");
            return false;
        }
        if(listen(sock,1024) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        return true;
    }

    bool TCPSocket::Connect(const std::string& ip, short port) {
        if(sock != -1) {
            close(sock);
            sock = -1;
        }
        if((sock = socket(AF_INET,type,0)) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        sockaddr_in destAddr{};
        destAddr.sin_port = htons(port);
        destAddr.sin_family = AF_INET;
        destAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        if(connect(sock,(sockaddr*)&destAddr,sizeof(destAddr)) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        return true;
    }

    void TCPSocket::Init(int _sock, sockaddr_in &addr) {
        if(sock != -1) {
            close(sock);
            sock = -1;
        }
        sock = _sock;
        myAddr = addr;
    }

    int TCPSocket::SendWithHeader(const char *data) {
        if(isNew){
            writeTotalBytes = strlen(data) + 1;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            header h{writeTotalBytes};
            if(::send(sock,&h,TCP_HEADER_SIZE,0) <= 0) return -1;
            isNew = false;
        }
        return sendImpl(data);
    }

    int TCPSocket::SendWithHeader(std::string &data) {
        return SendWithHeader(data.c_str());
    }

    int TCPSocket::SendWithHeader(std::string &&data) {
        return SendWithHeader(data.c_str());
    }

    int TCPSocket::SendAll(std::string &data) {
        if(isNew){
            writeTotalBytes = data.size();
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return sendImpl(data.c_str());
    }

    int TCPSocket::Send(const char *data, size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return sendImpl(data);
    }

    int TCPSocket::Send(std::string &data, size_t size) {
        return Send(data.c_str(),size);
    }

    int TCPSocket::Send(std::string &&data, size_t size) {
        return Send(data.c_str(),size);
    }

    int TCPSocket::SendFile(const std::string &filePath) {
        if(isNew){
            int file = open(filePath.c_str(),O_RDONLY);
            if(file < 0){
                LOG_ERROR(strerror(errno));
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

    int TCPSocket::RecvWithHeader(std::string &data) {
        if(isNew){
            header h{};
            if(::recv(sock,&h,TCP_HEADER_SIZE,0) <= 0){
                return false;
            }
            readTotalBytes = h.size;
            readCursor = 0;
            isNew = false;
        }
        return recvImpl(data);
    }

    int TCPSocket::Recv(std::string &data, size_t size) {
        if(isNew){
            readTotalBytes = size;
            readCursor = 0;
            isNew = false;
        }
        return recvImpl(data);
    }

    bool TCPSocket::RecvAll(std::string &data) {
        readTotalBytes = SIZE_MAX;
        readCursor = 0;
        bool ret = recvImpl(data) == 0;
        if(!ret){
            readTotalBytes = 0;
            readCursor = 0;
        }
        return ret;
    }

    int TCPSocket::RecvFile(const std::string &filePath, size_t size) {
        if(isNew){
            readCursor = 0;
            readTotalBytes = size;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY);
            isNew = false;
        }
        ssize_t hadRecv;
        while(readCursor < readTotalBytes){
            bzero(readBuffer,SOCKET_BUF_SIZE);
            if((hadRecv = ::recv(sock,readBuffer,readTotalBytes - readCursor,0)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                close(file);
                file = -1;
                return -1;
            }
            write(file,readBuffer,hadRecv);
            readCursor += hadRecv;
        }
        close(file);
        file = -1;
        return 1;
    }

    /* udp Impl*/
    UDPSocket::UDPSocket() {
        type = SOCK_DGRAM;
    }

    int UDPSocket::sendImpl(const char *data) {
        size_t hadSend,needSend;
        while(writeCursor < writeTotalBytes){
            needSend = writeTotalBytes - writeCursor;
            if((hadSend = sendto(sock,data + writeCursor,needSend,
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
        return true;
    }

    int UDPSocket::recvImpl(std::string &data) {
        if(readTotalBytes <= 0) return false;
        ssize_t hadRecv;
        size_t needRecv;
        if(readTotalBytes != SIZE_MAX && data.size() < readTotalBytes){
            data.reserve(readTotalBytes);
        }
        while(readCursor < readTotalBytes){
            bzero(readBuffer,SOCKET_BUF_SIZE);
            memset(&destAddr,0,sizeof(destAddr));
            needRecv = readTotalBytes - readCursor;
            if((hadRecv = ::recvfrom(sock,readBuffer,needRecv,0,(struct sockaddr*)&destAddr,&destAddrLen)) <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    isNew = false;
                    return 0;
                }
                return -1;
            }
            data.append(readBuffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        return 1;
    }

    UDPSocket::~UDPSocket() {
        if(sock != -1){
            close(sock);
            sock = -1;
        }
    }

    void UDPSocket::initPacket(UDPSocket::packet &packet, uint8_t type, uint16_t length, uint32_t sequence) {
        packet.header.checksum = 0;
        packet.header.type = type;
        packet.header.length = length;
        packet.header.sequence = sequence;
        packet.header.checksum = checksum(&packet,sizeof(packet.header) + length);
    }

    bool UDPSocket::checkPacket(UDPSocket::packet &packet, uint8_t type) {
        if(packet.header.type != type) return false;
        uint32_t check = packet.header.checksum;
        packet.header.checksum = 0;
        uint32_t sum = checksum(&packet,sizeof(packet.header) + packet.header.length);
        return check == sum;
    }

    static uint32_t crc32_for_byte(uint32_t r) {
        for(int j = 0; j < 8; ++j)
            r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
        return r ^ (uint32_t)0xFF000000L;
    }

    static void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
        static uint32_t table[0x100];
        if(!*table)
            for(size_t i = 0; i < 0x100; ++i)
                table[i] = crc32_for_byte(i);
        for(size_t i = 0; i < n_bytes; ++i)
            *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
    }

    uint32_t UDPSocket::checksum(const void *packet, size_t nBytes) {
        uint32_t crc = 0;
        crc32(packet,nBytes,&crc);
        return crc;
    }

    uint64_t UDPSocket::nowUs() {
        struct timeval tv{};
        gettimeofday (&tv, nullptr);
        return (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec);
    }

    bool UDPSocket::Init(const std::string &ip, short port) {
        if((sock = socket(AF_INET,type,0)) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        myAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        myAddr.sin_port = htons(port);
        myAddr.sin_family = AF_INET;
        return true;
    }

    bool UDPSocket::Bind() {
        if(bind(sock,(sockaddr*)&myAddr,sizeof(myAddr)) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        return true;
    }

    bool UDPSocket::Listen() {
        if(listen(sock,1024) < 0){
            LOG_ERROR(strerror(errno));
            return false;
        }
        return false;
    }

    int UDPSocket::SendWithHeader(const char *data) {
        uint32_t random = (uint32_t)rand()%100;
        packet packet{};
        initPacket(packet,UDP_START,0,random);
        if(sendto(sock,&packet,packet.header.length + sizeof(packet.header),0,
                  (sockaddr*)&destAddr,sizeof(destAddr)) <= 0){
            return -1;
        }
        uint64_t timer = nowUs();
        socklen_t socklen;
        while(true){
            if(recvfrom(sock,&packet,PACKET_SIZE,MSG_DONTWAIT,(sockaddr*)&destAddr,&socklen) < 0){
            }
        }
    }

    int UDPSocket::SendWithHeader(std::string &data) {
        return Socket::SendWithHeader(data);
    }

    int UDPSocket::SendWithHeader(std::string &&data) {
        return Socket::SendWithHeader(data);
    }

    int UDPSocket::SendAll(std::string &data) {
        if(isNew){
            writeTotalBytes = data.size();
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return sendImpl(data.c_str());
    }

    int UDPSocket::Send(std::string &data, size_t size) {
        return Send(data.c_str(),size);
    }

    int UDPSocket::Send(std::string &&data, size_t size) {
        return Send(data.c_str(),size);
    }

    int UDPSocket::Send(const char *data, size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return sendImpl(data);
    }

    int UDPSocket::SendFile(const std::string &filePath) {

    }

    int UDPSocket::RecvWithHeader(std::string &data) {
        return Socket::RecvWithHeader(data);
    }

    int UDPSocket::Recv(std::string &data, size_t size) {
        return 0;
    }

    bool UDPSocket::RecvAll(std::string &data) {
        return false;
    }

    int UDPSocket::RecvFile(const std::string &filePath, size_t size) {
        return 0;
    }
} // totoro