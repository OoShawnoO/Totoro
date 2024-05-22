#include <netdb.h>
#include "http/Forwarder.h"
#include "fmt/format.h"

const std::string HttpReverseForwarderChan = "Totoro";
const std::string HttpsReverseForwarderChan = "Totoro";
const std::string HttpsForwardForwarderChan = "Totoro";
const std::string HttpForwardForwarderChan = "Totoro";
namespace totoro {
    /* region HttpForwardForwarder */
    int HttpForwardForwarder::InitForwarder() {
        auto host = requestHeader.GetFields().find("Host");
        if (host == requestHeader.GetFields().end() || host->second.empty()) {
            MOLE_ERROR(HttpForwardForwarderChan, "host not found");
            return -1;
        }
        std::string ip, hostAddr = host->second[0];
        unsigned short port;
        auto pos = hostAddr.find(':');
        if (pos == std::string::npos) {
            ip = hostAddr;
            port = 80;
        } else {
            ip = hostAddr.substr(0, pos);
            port = std::stoi(hostAddr.substr(pos + 1));
        }
        auto hostEntry = gethostbyname(ip.c_str());
        if (!hostEntry) {
            MOLE_ERROR(HttpForwardForwarderChan, "can't parse ip address:" + ip);
            return -1;
        }
        bool connected = false;
        std::string tempIP;
        for (int i = 0; hostEntry->h_addr_list[i]; i++) {
            tempIP = inet_ntoa(*(struct in_addr *) hostEntry->h_addr_list[i]);
            if (forwarder.TcpClient::Connect(tempIP, port)) {
                connected = true;
                break;
            }
        }
        if (!connected) {
            MOLE_ERROR(HttpForwardForwarderChan, "can't connect destination address");
            return -1;
        }
        if (!forwardCandidateMap->insert({forwarder.Sock(), sock}).second) {
            MOLE_ERROR(HttpForwardForwarderChan, "unable to add forward pair");
            return -1;
        }
        if (EpollAdd(forwarder.Sock()) < 0) {
            MOLE_ERROR(HttpForwardForwarderChan, strerror(errno));
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderReadCallback() {
        return forwarder.ParseResponse();
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderAfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwardForwarder::ForwarderAfterWriteCallback() {
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(sock, Write, true);
        forwarder.ClearData();
        return INTERRUPT;
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::ReadCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderReadCallback();
        }
        return HttpBase::ReadCallback();
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::AfterReadCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterReadCallback();
        }
        if (forwarder.Sock() == BAD_SOCKET) {
            if (InitForwarder() < 0) return FAILED;
        }
        int ret = forwarder.SendAll(requestText);
        if (ret < 0) {
            MOLE_ERROR(HttpForwardForwarderChan, "forwarder send request failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(forwarder.Sock(), Read, true);
        return INTERRUPT;
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::WriteCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderWriteCallback();
        }
        int ret = SendAll(forwarder.responseText);
        if (ret < 0) {
            MOLE_ERROR(HttpForwardForwarderChan, "send response failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        return SUCCESS;
    }

    Connection::CallbackReturnType totoro::HttpForwardForwarder::AfterWriteCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterWriteCallback();
        }
        return HttpBase::AfterWriteCallback();
    }

    int HttpForwardForwarder::Close() {
        int forwardSock = forwarder.Sock();
        forwarder.Close();
        HttpBase::Close();
        return forwardSock;
    }
    /* endregion */

    /* region HttpsForwardForwarder */
    int HttpsForwardForwarder::InitForwarder() {
        std::string ip, hostAddr = requestHeader.GetUrl();
        unsigned short port;
        auto pos = hostAddr.find(':');
        if (pos == std::string::npos) {
            ip = hostAddr;
            port = 443;
        } else {
            ip = hostAddr.substr(0, pos);
            port = std::stoi(hostAddr.substr(pos + 1));
        }
        auto hostEntry = gethostbyname(ip.c_str());
        if (!hostEntry) {
            MOLE_ERROR(HttpsForwardForwarderChan, "can't parse ip address:" + ip);
            return -1;
        }
        bool connected = false;
        std::string tempIP;
        for (int i = 0; hostEntry->h_addr_list[i]; i++) {
            tempIP = inet_ntoa(*(struct in_addr *) hostEntry->h_addr_list[i]);
            if (forwarder.Connect(tempIP, port)) {
                connected = true;
                break;
            }
        }
        if (!connected) {
            MOLE_ERROR(HttpsForwardForwarderChan, "can't connect destination address");
            return -1;
        }
        if (!forwardCandidateMap->insert({forwarder.Sock(), sock}).second) {
            MOLE_ERROR(HttpsForwardForwarderChan, "unable to add forward pair");
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderReadCallback() {
        return forwarder.ParseResponse();
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderAfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ForwarderAfterWriteCallback() {
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(sock, Write, true);
        forwarder.ClearData();
        return INTERRUPT;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::ReadCallback() {
        if (!connection) {
            if (!TcpSocket::Recv(data,SIZE_MAX)) {
                return FAILED;
            }
            if (!requestHeader.Parse(data) || requestHeader.GetMethod() != CONNECT) return FAILED;
            if (InitForwarder() < 0) return FAILED;
            data.clear();
            return SUCCESS;
        }
        if (workSock == forwarder.Sock()) {
            return ForwarderReadCallback();
        }
        return HttpsBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpsForwardForwarder::AfterReadCallback() {
        if (!connection) {
            return SUCCESS;
        }
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterReadCallback();
        }
        int ret = forwarder.SendAll(requestText);
        if (ret < 0) {
            MOLE_ERROR(HttpForwardForwarderChan, "forwarder send request failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(forwarder.Sock(), Read, true);
        return INTERRUPT;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::WriteCallback() {
        if (!connection) {
            int ret = TcpSocket::SendAll(fmt::format("{} 200 Connection established\r\n\r\n",
                                                     HttpVersionMap.at(requestHeader.GetVersion())));
            if (ret < 0) {
                MOLE_ERROR(HttpForwardForwarderChan, "send response failed");
                return FAILED;
            } else if (ret == 0) {
                return AGAIN;
            }
            return SUCCESS;
        }
        if (workSock == forwarder.Sock()) {
            return ForwarderWriteCallback();
        }
        int ret = SendAll(forwarder.responseText);
        if (ret < 0) {
            MOLE_ERROR(HttpForwardForwarderChan, "send response failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwardForwarder::AfterWriteCallback() {
        if (!connection) {
            if (InitSSL() < 0) {
                return FAILED;
            }
            if (EpollAdd(forwarder.Sock()) < 0) {
                MOLE_ERROR(HttpsForwardForwarderChan, strerror(errno));
                return FAILED;
            }
            return SUCCESS;
        }
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterWriteCallback();
        }
        return HttpsBase::AfterWriteCallback();
    }

    int HttpsForwardForwarder::Close() {
        int forwardSock = forwarder.Sock();
        forwarder.Close();
        HttpsBase::Close();
        return forwardSock;
    }

    /* endregion */

    /* region HttpReverseForwarder */
    const std::pair<std::string, unsigned short> &
    HttpReverseForwarder::GetHttpForwardAddress(const std::string &port) {
        static std::unordered_map<std::string, std::vector<std::pair<std::string, unsigned short>>> candidateServers;
        static std::unordered_map<std::string, int> candidateServersConnectionNum;
        static bool isInit = true;
        if (isInit) {
            auto reverseProxyMap = Configure::config.conf["http-reverse-proxy"];
            for (auto items = reverseProxyMap.cbegin(); items != reverseProxyMap.cend(); items++) {
                std::vector<std::pair<std::string, unsigned short>> servers;
                for (const auto &item: items.value()) {
                    servers.emplace_back(item["ip"], item["port"]);
                }
                auto ret = candidateServers.insert({items.key(), std::move(servers)});
                if (!ret.second) {
                    MOLE_ERROR(HttpReverseForwarderChan, "candidate server insert failed");
                    exit(-1);
                }
            }
            isInit = false;
        }
        auto candidateServersIter = candidateServers.end();
        if ((candidateServersIter = candidateServers.find(port)) == candidateServers.end()) {
            MOLE_ERROR(HttpReverseForwarderChan, "candidate server find port failed");
            exit(-1);
        }
        auto candidateNum = (candidateServersConnectionNum[port]++) % candidateServersIter->second.size();
        return candidateServersIter->second[candidateNum];
    }

    int HttpReverseForwarder::InitForwarder() {
        const auto &addr = GetHttpForwardAddress(std::to_string(ntohs(local_address.sin_port)));
        std::string ip = addr.first;
        unsigned short port = addr.second;

        auto hostEntry = gethostbyname(ip.c_str());
        if (!hostEntry) {
            MOLE_ERROR(HttpReverseForwarderChan, "can't parse ip address:" + ip);
            return -1;
        }
        bool connected = false;
        std::string tempIP;
        for (int i = 0; hostEntry->h_addr_list[i]; i++) {
            tempIP = inet_ntoa(*(struct in_addr *) hostEntry->h_addr_list[i]);
            if (forwarder.Connect(tempIP, port)) {
                connected = true;
                break;
            }
        }
        if (!connected) {
            MOLE_ERROR(HttpReverseForwarderChan, "can't connect destination address");
            return -1;
        }
        if (!forwardCandidateMap->insert({forwarder.Sock(), sock}).second) {
            MOLE_ERROR(HttpReverseForwarderChan, "unable to add forward pair");
            return -1;
        }
        if (EpollAdd(forwarder.Sock()) < 0) {
            MOLE_ERROR(HttpReverseForwarderChan, strerror(errno));
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderReadCallback() {
        return forwarder.ParseResponse();
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderAfterReadCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderWriteCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ForwarderAfterWriteCallback() {
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(sock, Write, true);
        forwarder.ClearData();
        return INTERRUPT;
    }

    Connection::CallbackReturnType HttpReverseForwarder::ReadCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderReadCallback();
        }
        return HttpBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpReverseForwarder::AfterReadCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterReadCallback();
        }
        if (forwarder.Sock() == BAD_SOCKET) {
            if (InitForwarder() < 0) return FAILED;
        }
        int ret = forwarder.SendAll(requestText);
        if (ret < 0) {
            MOLE_ERROR(HttpReverseForwarderChan, "forwarder send request failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(forwarder.Sock(), Read, true);
        return INTERRUPT;
    }

    Connection::CallbackReturnType HttpReverseForwarder::WriteCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderWriteCallback();
        }
        int ret = SendAll(forwarder.responseText);
        if (ret < 0) {
            MOLE_ERROR(HttpReverseForwarderChan, "send response failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpReverseForwarder::AfterWriteCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterWriteCallback();
        }
        return HttpBase::AfterWriteCallback();
    }

    int HttpReverseForwarder::Close() {
        int forwardSock = forwarder.Sock();
        forwarder.Close();
        HttpBase::Close();
        return forwardSock;
    }
    /* endregion */

    /* region HttpsReverseForwarder */
    const std::pair<std::string, unsigned short> &
    HttpsReverseForwarder::GetHttpsForwardAddress(const std::string &port) {
        static std::unordered_map<std::string, std::vector<std::pair<std::string, unsigned short>>> candidateServers;
        static std::unordered_map<std::string, int> candidateServersConnectionNum;
        static bool isInit = true;
        if (isInit) {
            auto reverseProxyMap = Configure::config.conf["https-reverse-proxy"];
            for (auto items = reverseProxyMap.cbegin(); items != reverseProxyMap.cend(); items++) {
                std::vector<std::pair<std::string, unsigned short>> servers;
                for (const auto &item: items.value()) {
                    servers.emplace_back(item["ip"], item["port"]);
                }
                auto ret = candidateServers.insert({items.key(), std::move(servers)});
                if (!ret.second) {
                    MOLE_ERROR(HttpsReverseForwarderChan, "candidate server insert failed");
                    exit(-1);
                }
            }
            isInit = false;
        }
        auto candidateServersIter = candidateServers.end();
        if ((candidateServersIter = candidateServers.find(port)) == candidateServers.end()) {
            MOLE_ERROR(HttpReverseForwarderChan, "candidate server find port failed");
            exit(-1);
        }
        auto candidateNum = (candidateServersConnectionNum[port]++) % candidateServersIter->second.size();
        return candidateServersIter->second[candidateNum];
    }

    int HttpsReverseForwarder::InitForwarder() {
        const auto &addr = GetHttpsForwardAddress(std::to_string(ntohs(local_address.sin_port)));
        std::string ip = addr.first;
        unsigned short port = addr.second;

        auto hostEntry = gethostbyname(ip.c_str());
        if (!hostEntry) {
            MOLE_ERROR(HttpsReverseForwarderChan, "can't parse ip address:" + ip);
            return -1;
        }
        bool connected = false;
        std::string tempIP;
        for (int i = 0; hostEntry->h_addr_list[i]; i++) {
            tempIP = inet_ntoa(*(struct in_addr *) hostEntry->h_addr_list[i]);
            if (forwarder.Connect(tempIP, port)) {
                connected = true;
                break;
            }
        }
        if (!connected) {
            MOLE_ERROR(HttpsReverseForwarderChan, "can't connect destination address");
            return -1;
        }
        if (!forwardCandidateMap->insert({forwarder.Sock(), sock}).second) {
            MOLE_ERROR(HttpsReverseForwarderChan, "unable to add forward pair");
            return -1;
        }
        if (EpollAdd(forwarder.Sock()) < 0) {
            MOLE_ERROR(HttpsReverseForwarderChan, strerror(errno));
            return -1;
        }
        return 1;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderReadCallback() {
        return forwarder.ParseResponse();
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderAfterReadCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderWriteCallback() {
        return Connection::SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ForwarderAfterWriteCallback() {
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(sock, Write, true);
        forwarder.ClearData();
        return INTERRUPT;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::ReadCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderReadCallback();
        }
        return HttpsBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpsReverseForwarder::AfterReadCallback() {
        if (!connection) {
            return SUCCESS;
        }
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterReadCallback();
        }
        if (forwarder.Sock() == BAD_SOCKET) {
            if (InitForwarder() < 0) {
                MOLE_ERROR(HttpsReverseForwarderChan, "init forwarder failed");
                return FAILED;
            }
        }
        int ret = forwarder.SendAll(requestText);
        if (ret < 0) {
            MOLE_ERROR(HttpsReverseForwarderChan, "forwarder send request failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        forwarder.parseStatus = RecvHeader;
        RegisterNextEvent(forwarder.Sock(), Read, true);
        return INTERRUPT;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::WriteCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderWriteCallback();
        }
        int ret = SendAll(forwarder.responseText);
        if (ret < 0) {
            MOLE_ERROR(HttpsReverseForwarderChan, "send response failed");
            return FAILED;
        } else if (ret == 0) {
            return AGAIN;
        }
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsReverseForwarder::AfterWriteCallback() {
        if (workSock == forwarder.Sock()) {
            return ForwarderAfterWriteCallback();
        }
        return HttpsBase::AfterWriteCallback();
    }

    int HttpsReverseForwarder::Close() {
        int forwardSock = forwarder.Sock();
        forwarder.Close();
        HttpsBase::Close();
        return forwardSock;
    }
    /* endregion */

} // totoro