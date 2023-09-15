#include <cstring>
#include <fcntl.h>
#include "core/Configure.h"
#include "core/SSLSocket.h"

const std::string SSLContextChan = "SSLContextChan";
const std::string SSLSocketChan = "SSLSocket";
namespace totoro {
    /* SSLContext Impl */
    SSLContext::SSLContext(const std::string& CA,const std::string& CERT,const std::string& PRIVATE) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        context = SSL_CTX_new(TLS_server_method());

        if(SSL_CTX_load_verify_locations(context, CA.c_str(), nullptr) <=0){
            LOG_ERROR(SSLContextChan,"load ca failed");
            exit(-1);
        }

        if(SSL_CTX_use_certificate_file(context,CERT.c_str(),SSL_FILETYPE_PEM)<=0){
            LOG_ERROR(SSLContextChan,"load public key failed");
            exit(-1);
        }
        if(SSL_CTX_use_PrivateKey_file(context,PRIVATE.c_str(),SSL_FILETYPE_PEM)<=0){
            LOG_ERROR(SSLContextChan,"load private key failed");
            exit(-1);
        }

        if(SSL_CTX_check_private_key(context)<=0) {
            LOG_ERROR(SSLContextChan,"load private key failed");
            exit(-1);
        }
    }

    SSLContext::~SSLContext() {
        SSL_CTX_free(context);
        context = nullptr;
    }

    SSLContext &SSLSocket::GetContext() {
        static SSLContext context(Configure::Get()["CA-CERT"],Configure::Get()["SERVER-CERT"],Configure::Get()["SERVER-KEY"]);
        return context;
    }

    /* SSL Client Context Impl */
    SSLClientContext::SSLClientContext(const std::string& CA,const std::string &CERT, const std::string &PRIVATE) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        context = SSL_CTX_new(TLS_client_method());

        //SSL双向认证
//        SSL_CTX_set_verify(context, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
        SSL_CTX_set_options(context, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
        SSL_CTX_set_min_proto_version(context, TLS1_1_VERSION);

        if(SSL_CTX_use_certificate_file(context,CERT.c_str(),SSL_FILETYPE_PEM)<=0){
            LOG_ERROR(SSLContextChan,"load public key failed");
            exit(-1);
        }
        if(SSL_CTX_use_PrivateKey_file(context,PRIVATE.c_str(),SSL_FILETYPE_PEM)<=0){
            LOG_ERROR(SSLContextChan,"load private key failed");
            exit(-1);
        }
        if(SSL_CTX_check_private_key(context)<=0) {
            LOG_ERROR(SSLContextChan,"load private key failed");
            exit(-1);
        }
    }

    SSLClientContext::~SSLClientContext() {
        SSL_CTX_free(context);
    }

    SSLClientContext &SSLSocket::GetClientContext() {
        static SSLClientContext context(Configure::Get()["CA-CERT"],Configure::Get()["CLIENT-CERT"],Configure::Get()["CLIENT-KEY"]);
        return context;
    }
    /* SSLSocket Impl */

    int SSLSocket::sendImpl(const char *data) {
        size_t hadSend,needSend;
        while(writeCursor < writeTotalBytes){
            needSend = writeTotalBytes - writeCursor;
            if((hadSend = SSL_write(connection,data + writeCursor,(int)needSend)) <= 0){
                if(hadSend == 0){
                    LOG_INFO(SSLSocketChan,"ssl connection closed");
                    return -1;
                }
                LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
                return -1;
            }
            writeCursor += hadSend;
        }
        writeCursor = 0;
        writeTotalBytes = 0;
        isNew = true;
        return 1;
    }

    int SSLSocket::recvImpl(std::string &data) {
        if(readTotalBytes <= 0) return false;
        ssize_t hadRecv;
        size_t needRecv;
        if(readTotalBytes != SIZE_MAX && data.size() < readTotalBytes){
            data.reserve(readTotalBytes);
        }
        bool flag = readTotalBytes == SIZE_MAX;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = SSL_read(connection,buffer,(int)needRecv)) <= 0){
                if(hadRecv == 0) {
                    LOG_INFO(SSLSocketChan,"ssl connection closed");
                    return -1;
                }
                LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
                return -1;
            }
            data.append(buffer,hadRecv);
            readCursor += hadRecv;
            if(flag){
                isNew = true;
                return 0;
            }
        }
        isNew = true;
        return 1;
    }

    SSLSocket::~SSLSocket() {
        SSLSocket::Close();
    }


    int SSLSocket::SendWithHeader(const char *data, size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            header h{writeTotalBytes};
            if(SSL_write(connection,&h,TCP_HEADER_SIZE) <= 0) return -1;
            isNew = false;
        }
        return SSLSocket::sendImpl(data);
    }

    int SSLSocket::SendWithHeader(const std::string &data) {
        return SSLSocket::SendWithHeader(data.c_str(),data.size());
    }

    int SSLSocket::SendWithHeader(std::string &&data) {
        return SSLSocket::SendWithHeader(data.c_str(),data.size());
    }

    int SSLSocket::SendFileWithHeader(const std::string &filePath) {
        if(isNew){
            file = open(filePath.c_str(),O_RDONLY);
            if(file < 0){
                LOG_ERROR(SSLSocketChan,strerror(errno));
                return -1;
            }
            memset(&stat,0,sizeof(stat));
            fstat(file,&stat);
            writeTotalBytes = stat.st_size;
            writeCursor = 0;
            if(SSL_write(connection,&writeTotalBytes,sizeof(writeTotalBytes)) <= 0){
                LOG_ERROR(SSLSocketChan, strerror(errno));
                return -1;
            }
        }
        int needSend;
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            needSend = writeTotalBytes - writeCursor > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (int)(writeTotalBytes - writeCursor);
            if(read(file,buffer,needSend) <= 0){
                LOG_ERROR(SSLSocketChan,"file read failed");
                return -1;
            }
            if((hadSend = SSL_write(connection,buffer,needSend)) <= 0){
                if(hadSend == 0) {
                    LOG_INFO(SSLSocketChan,"ssl connection closed");
                    return -1;
                }
                LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
                return -1;
            }
            writeCursor += hadSend;
        }
        close(file);
        file = -1;
        isNew = true;
        return 1;
    }

    int SSLSocket::SendFile(const std::string &filePath) {
        if(isNew){
            file = open(filePath.c_str(),O_RDONLY);
            if(file < 0){
                LOG_ERROR(SSLSocketChan,strerror(errno));
                return -1;
            }
            memset(&stat,0,sizeof(stat));
            fstat(file,&stat);
            writeTotalBytes = stat.st_size;
            writeCursor = 0;
        }
        int needSend;
        ssize_t hadSend;
        while(writeCursor < writeTotalBytes){
            needSend = writeTotalBytes - writeCursor > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (int)(writeTotalBytes - writeCursor);
            if(read(file,buffer,needSend) <= 0){
                LOG_ERROR(SSLSocketChan,"file read failed");
                return -1;
            }
            if((hadSend = SSL_write(connection,buffer,needSend)) <= 0){
                if(hadSend == 0) {
                    LOG_INFO(SSLSocketChan,"ssl connection closed");
                    return -1;
                }
                LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
                return -1;
            }
            writeCursor += hadSend;
        }
        close(file);
        file = -1;
        isNew = true;
        return 1;
    }

    int SSLSocket::RecvWithHeader(std::string &data) {
        if(isNew){
            data.clear();
            header h{};
            if(SSL_read(connection,&h,TCP_HEADER_SIZE) <= 0){
                return -1;
            }
            readTotalBytes = h.size;
            readCursor = 0;
            isNew = false;
        }
        return SSLSocket::recvImpl(data);
    }

    int SSLSocket::RecvFileWithHeader(const std::string &filePath) {
        if(isNew){
            size_t size;
            if(SSL_read(connection,&size,sizeof(size)) < 0){
                LOG_ERROR(SSLSocketChan, strerror(errno));
                return -1;
            }
            readCursor = 0;
            readTotalBytes = size;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY,777);
            isNew = false;
        }
        ssize_t hadRecv;
        size_t needRecv;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = SSL_read(connection,buffer,(int)needRecv)) < 0){
                close(file);
                file = -1;
                if(hadRecv == 0) {
                    LOG_INFO(SSLSocketChan,"ssl connection closed");
                    return -1;
                }
                LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
            }
            write(file,buffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        close(file);
        file = -1;
        return 1;
    }

    int SSLSocket::RecvFile(const std::string &filePath, size_t size) {
        if(isNew){
            readCursor = 0;
            readTotalBytes = size;
            file = open(filePath.c_str(),O_CREAT | O_WRONLY,777);
            isNew = false;
        }
        ssize_t hadRecv;
        size_t needRecv;
        while(readCursor < readTotalBytes){
            bzero(buffer,SOCKET_BUF_SIZE);
            needRecv = (readTotalBytes - readCursor) > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : (readTotalBytes - readCursor);
            if((hadRecv = SSL_read(connection,buffer,(int)needRecv)) < 0){
                close(file);
                file = -1;
                if(hadRecv == 0) {
                    LOG_INFO(SSLSocketChan,"ssl connection closed");
                    return -1;
                }
                LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
            }
            write(file,buffer,hadRecv);
            readCursor += hadRecv;
        }
        isNew = true;
        close(file);
        file = -1;
        return 1;
    }

    int SSLSocket::Close() {
        if(connection){
            SSL_shutdown(connection);
            SSL_free(connection);
            connection = nullptr;
        }
        return TCPSocket::Close();
    }

    int SSLSocket::SendAll(const std::string &data) {
        if(isNew){
            writeTotalBytes = data.size();
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return SSLSocket::sendImpl(data.c_str());
    }

    int SSLSocket::Send(const std::string &data, size_t size) {
        return SSLSocket::Send(data.c_str(), size);
    }

    int SSLSocket::Send(std::string &&data, size_t size) {
        return SSLSocket::Send(data.c_str(), size);
    }

    int SSLSocket::Send(const char *data, size_t size) {
        if(isNew){
            writeTotalBytes = size;
            writeCursor = 0;
            if(writeTotalBytes <= 1) return -1;
            isNew = false;
        }
        return SSLSocket::sendImpl(data);
    }

    int SSLSocket::Recv(std::string &data, size_t size) {
        if(isNew){
            data.clear();
            readTotalBytes = size;
            readCursor = 0;
            isNew = false;
        }
        return SSLSocket::recvImpl(data);
    }

    bool SSLSocket::RecvAll(std::string &data) {
        if(isNew){
            data.clear();
            readTotalBytes = SIZE_MAX;
            readCursor = 0;
            isNew = false;
        }
        return SSLSocket::recvImpl(data) == 0;
    }

    int SSLSocket::InitSSL() {
        connection = SSL_new(GetContext().context);
        if(!connection){
            LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
            return -1;
        }
        if(SSL_set_fd(connection,sock) <= 0){
            LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
            return -1;
        }
        if(SSL_accept(connection) <= 0){
            LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
            return -1;
        }
        if (SSL_is_init_finished(connection) <= 0) {
            LOG_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(),buffer));
            return -1;
        }
        return 1;
    }

    bool SSLSocket::Connect(const std::string &ip, unsigned short port) {

        if(!TCPSocket::Connect(ip,port)) return false;

        if(!connection){
            connection = SSL_new(GetClientContext().context);
            if(!connection){
                LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
                return false;
            }
            if(SSL_set_fd(connection,sock) <= 0){
                LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
                return false;
            }
            if(SSL_connect(connection) <= 0){
                LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
                return false;
            }
            if(SSL_is_init_finished(connection) <= 0){
                LOG_ERROR(SSLSocketChan,ERR_error_string(ERR_get_error(),buffer));
                return false;
            }
        }
        return true;
    }
} // totoro