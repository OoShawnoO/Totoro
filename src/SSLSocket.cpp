#include <cstring>
#include <fcntl.h>
#include <csignal>
#include "utils/Configure.h"
#include "utils/SSLSocket.h"

const std::string SSLContextChan = "Totoro";
const std::string SSLSocketChan = "Totoro";
namespace totoro {
    /* SSLContext Impl */
    SSLContext::SSLContext(const std::string &CA, const std::string &CERT, const std::string &PRIVATE) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        context = SSL_CTX_new(SSLv23_server_method());

        if (SSL_CTX_use_certificate_file(context, CERT.c_str(), SSL_FILETYPE_PEM) <= 0) {
            MOLE_ERROR(SSLContextChan, ERR_error_string(ERR_get_error(), nullptr));
            exit(-1);
        }
        if (SSL_CTX_use_PrivateKey_file(context, PRIVATE.c_str(), SSL_FILETYPE_PEM) <= 0) {
            MOLE_ERROR(SSLContextChan, ERR_error_string(ERR_get_error(), nullptr));
            exit(-1);
        }

        if (SSL_CTX_check_private_key(context) <= 0) {
            MOLE_ERROR(SSLContextChan, ERR_error_string(ERR_get_error(), nullptr));
            exit(-1);
        }
    }

    SSLContext::~SSLContext() {
        SSL_CTX_free(context);
        context = nullptr;
    }

    SSLContext &SSLSocket::GetContext() {
        static SSLContext context(
                Configure::config.conf["ca-cert"],
                Configure::config.conf["server-cert"],
                Configure::config.conf["server-key"]
        );
        return context;
    }

    /* SSL Client Context Impl */
    SSLClientContext::SSLClientContext(const std::string &CA, const std::string &CERT, const std::string &PRIVATE) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        context = SSL_CTX_new(SSLv23_client_method());
    }

    SSLClientContext::~SSLClientContext() {
        SSL_CTX_free(context);
    }

    SSLClientContext &SSLSocket::GetClientContext() {
        static SSLClientContext context(
                Configure::config.conf["ca-cert"],
                Configure::config.conf["client-cert"],
                Configure::config.conf["client-key"]
        );
        return context;
    }

    /* SSLSocket Impl */


    SSLSocket::~SSLSocket() {
        SSLSocket::Close();
    }

    int SSLSocket::InitSSL() {
        connection = SSL_new(GetContext().context);
        char buffer[4096] = {0};
        if (!connection) {
            MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
            return -1;
        }
        if (SSL_set_fd(connection, sock) <= 0) {
            MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
            return -1;
        }
        if (SSL_accept(connection) <= 0) {
            MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
            return -1;
        }
        if (SSL_is_init_finished(connection) <= 0) {
            MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
            return -1;
        }
        return 1;
    }

    int SSLSocket::Close() {
        if (connection) {
            signal(SIGPIPE, SIG_IGN);
            SSL_shutdown(connection);
            SSL_free(connection);
            connection = nullptr;
        }
        return TcpSocket::Close();
    }

    size_t SSLSocket::Send(const char *data, size_t size) {
        size_t needSend;
        ssize_t hadSend;
        size_t write_cursor = 0;
        while(write_cursor < size){
            needSend = size - write_cursor;
            if((hadSend = SSL_write(connection,data + write_cursor,needSend)) <= 0){
                MOLE_ERROR(SSLSocketChan, strerror(errno));
                return write_cursor;
            }
            write_cursor += hadSend;
        }
        return size;
    }

    bool SSLSocket::SendFile(const std::string &file_path) {
        auto file = open(file_path.c_str(),O_RDONLY);
        struct stat stat{};
        if(file < 0){
            MOLE_ERROR(SSLSocketChan,strerror(errno));
            return -1;
        }
        fstat(file,&stat);
        size_t write_cursor = 0;
        ssize_t hadSend,hadRead;
        char buffer[4096] = {0};
        while(write_cursor < stat.st_size){
            bzero(buffer,sizeof(buffer));
            hadRead = read(file,buffer,sizeof(buffer));
            if(hadRead < 0) {
                close(file);
                return false;
            }
            if((hadSend = SSL_write(connection,buffer,hadRead)) < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                MOLE_ERROR(SSLSocketChan, strerror(errno));
                close(file);
                return false;
            }
            write_cursor += hadSend;
        }
        close(file);
        return true;
    }

    size_t SSLSocket::Recv(std::string &data, size_t size) {
        if(cache.size() >= size) {
            data = cache.substr(0,size);
            cache = cache.substr(size);
            return size;
        }else{
            size -= cache.size();
            data = std::move(cache);
        }
        ssize_t hadRecv;
        size_t needRecv;
        size_t read_cursor = 0;
        char buffer[4096] = {0};
        while(read_cursor < size){
            bzero(buffer,4096);
            needRecv = size - read_cursor > 4096 ? 4096 : size - read_cursor;
            if((hadRecv = SSL_read(connection,buffer,needRecv)) <= 0){
                MOLE_ERROR(SSLSocketChan, strerror(errno));
                return read_cursor;
            }
            data.append(buffer,hadRecv);
            read_cursor += hadRecv;
        }
        return size;
    }

    size_t SSLSocket::RecvUntil(std::string &data, const char *key) {
        if(strlen(key) == 0) return 0;
        ssize_t hadRecv;
        size_t read_cursor = 0;
        size_t pos;
        char buffer[4096] = {0};
        while(true){
            bzero(buffer,4096);
            if((hadRecv = SSL_read(connection,buffer,4096)) <= 0){
                MOLE_ERROR(SSLSocketChan, strerror(errno));
                return 0;
            }
            cache.append(buffer,hadRecv);
            if((pos = cache.find(key)) != std::string::npos) {
                data = cache.substr(0,pos + strlen(key));
                return pos + strlen(key);
            }
            read_cursor += hadRecv;
        }
    }

    bool SSLSocket::Connect(const std::string &ip, unsigned short port) {

        if (!TcpClient::Connect(ip, port)) return false;

        char buffer[4096] = {0};
        if (!connection) {
            connection = SSL_new(GetClientContext().context);
            if (!connection) {
                MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
                return false;
            }
            if (SSL_set_fd(connection, sock) <= 0) {
                MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
                return false;
            }
            if (SSL_connect(connection) <= 0) {
                MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
                return false;
            }
            if (SSL_is_init_finished(connection) <= 0) {
                MOLE_ERROR(SSLSocketChan, ERR_error_string(ERR_get_error(), buffer));
                return false;
            }
        }
        return true;
    }

    size_t SSLSocket::SendAll(const std::string &data) {
        return SSLSocket::Send(data.c_str(),data.size());
    }
} // totoro