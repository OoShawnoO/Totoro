#include <netdb.h>
#include <iostream>
#include "HttpClient.h"
#include "fmt/format.h"

const std::string HttpClientChan = "HttpClientChan";
namespace totoro {
    bool
    HttpClientImpl::Get(
    ProtoType                                                                       type,
    const std::string                                                               &addr,
    unsigned short                                                                  port,
    const std::string                                                               &url,
    const HttpHeaderFieldsType                                                      &headers,
    const HttpParameterType                                                         &parameters,
    const HttpCookieType                                                            &cookies,
    const std::unordered_map<std::string, std::pair<std::string,unsigned short>>    &proxies,
    int                                                                             timeout)
    {
        std::string realAddr = addr;
        unsigned short realPort = port;
        auto proxy = proxies.begin();
        if((proxy = proxies.find("http")) != proxies.end()){
            realAddr = proxy->second.first;
            realPort = proxy->second.second;
            requestHeader.SetField("Host",{fmt::format("{}:{}",addr,port)});
        }

        auto hostEntry = gethostbyname(realAddr.c_str());
        if(!hostEntry) {
            LOG_ERROR(HttpClientChan,"failed to get host name");
            return false;
        }
        bool connected = false;
        std::string tempIP;
        for(int i=0;hostEntry->h_addr_list[i];i++){
            tempIP = inet_ntoa(*(struct in_addr*)hostEntry->h_addr_list[i]);
            if(Connect(tempIP,realPort)){ connected = true;break; }
        }
        if(!connected) {
            LOG_ERROR(HttpClientChan,"failed to connect host");
            return false;
        }


        std::string realUrl = url;
        bool isFirst = false;
        if(url.find('?') == std::string::npos) {
            isFirst = true;
        }
        for(const auto& parameter : parameters ){
            if(isFirst) {
                realUrl += '?';
                isFirst = false;
            }
            else realUrl += '&';
            realUrl += fmt::format("{}={}",parameter.first,parameter.second);
        }

        requestHeader.SetMethod(GET);
        requestHeader.SetVersion(HTTP11);
        requestHeader.SetUrl(realUrl);

        for(const auto& header : headers ){
            requestHeader.SetField(header.first,header.second);
        }
        std::string cookieStr;
        isFirst = true;
        for(const auto& cookie : cookies ){
            if(isFirst) {
                isFirst = false;
            }else { cookieStr += ';'; }
            cookieStr += fmt::format("{}={}",cookie.first,cookie.second);
        }
        if(!cookieStr.empty())
            requestHeader.SetField("Cookies",{cookieStr});

        parseStatus = SendHeader;
        auto status = SUCCESS;
        while((status = SendRequest()) == AGAIN){}
        if(status == FAILED) return false;
        while((status = ParseResponse()) == AGAIN){}
        if(status == FAILED) return false;
        return true;
    }

    bool
    HttpClientImpl::Post(
    ProtoType                                                                       type,
    const std::string                                                               &addr,
    unsigned short                                                                  port,
    const std::string                                                               &url,
    const HttpHeaderFieldsType                                                      &headers,
    const HttpCookieType                                                            &cookies,
    const HttpMultiPartType                                                         &files,
    const HttpFormType                                                              &forms,
    const std::unordered_map<std::string, std::pair<std::string,unsigned short>>    &proxies,
    int                                                                             timeout)
    {
        return 0;
    }

    const HttpBase::RequestHeader &HttpClientImpl::GetRequestHeader() {
        return requestHeader;
    }

    const HttpBase::RequestBody &HttpClientImpl::GetRequestBody() {
        return requestBody;
    }

    const HttpBase::ResponseHeader &HttpClientImpl::GetResponseHeader() {
        return responseHeader;
    }

    const HttpBase::ResponseBody &HttpClientImpl::GetResponseBody() {
        return responseBody;
    }

    bool HttpClient::parseUrl(const std::string &url) {
        std::string proto = url.substr(0,5);
        std::string tempUrl,tempAddr;
        if(proto == "http:") {
            type = HTTP;
            tempUrl = url.substr(7);
            auto pos = tempUrl.find('/');
            if(pos == std::string::npos) requestUrl = "/";
            else requestUrl = tempUrl.substr(pos);
            tempAddr = tempUrl.substr(0,pos);
            pos = tempAddr.find(':');
            if(pos == std::string::npos) port = 80;
            else port = stoi(tempAddr.substr(pos+1));
            requestAddr = tempAddr.substr(0,pos);
            return true;
        }
        else if(proto == "https") {
            type = HTTPS;
            tempUrl = url.substr(8);
            auto pos = tempUrl.find('/');
            if(pos == std::string::npos) requestUrl = "/";
            else requestUrl = tempUrl.substr(pos);
            tempAddr = tempUrl.substr(0,pos);
            pos = tempAddr.find(':');
            if(pos == std::string::npos) port = 443;
            else port = stoi(tempAddr.substr(pos+1));
            requestAddr = tempAddr.substr(0,pos);
            return true;
        }
        else return false;
    }

    bool HttpsClientImpl::Get(
            ProtoType                                                                       type,
            const std::string                                                               &addr,
            unsigned short                                                                  port,
            const std::string                                                               &url,
            const HttpHeaderFieldsType                                                      &headers,
            const HttpParameterType                                                         &parameters,
            const HttpCookieType                                                            &cookies,
            const std::unordered_map<std::string, std::pair<std::string,unsigned short>>    &proxies,
            int                                                                             timeout)
    {
        std::string realAddr = addr;
        unsigned short realPort = port;
        auto proxy = proxies.begin();

        if((proxy = proxies.find("https")) != proxies.end()){
            realAddr = proxy->second.first;
            realPort = proxy->second.second;
        }


        auto hostEntry = gethostbyname(realAddr.c_str());
        if(!hostEntry) {
            LOG_ERROR(HttpClientChan,"failed to get host name");
            return false;
        }
        bool connected = false;
        if((proxy = proxies.find("https")) != proxies.end()) {
            std::string tempIP;
            for (int i = 0; hostEntry->h_addr_list[i]; i++) {
                tempIP = inet_ntoa(*(struct in_addr *) hostEntry->h_addr_list[i]);
                if (TCPSocket::Connect(tempIP, realPort)) {
                    connected = true;
                    break;
                }
            }
            if (!connected) {
                LOG_ERROR(HttpClientChan, "failed to connect host");
                return false;
            }
            if (TCPSocket::SendAll(fmt::format("{} {} {}\r\n\r\n","CONNECT", fmt::format("{}:{}", addr, port), "HTTP1/1")) < 0) {
                LOG_ERROR(HttpClientChan, "send connect failed");
                return false;
            }
            while (!TCPSocket::RecvAll(data)) {}
            if (data.rfind("Connection Established") == std::string::npos) {
                LOG_ERROR(HttpClientChan, "connect proxy failed");
                return false;
            }
            if (!connection) {
                connection = SSL_new(GetClientContext().context);
                if (!connection) {
                    LOG_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
                if (SSL_set_fd(connection, sock) <= 0) {
                    LOG_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
                if (SSL_connect(connection) <= 0) {
                    LOG_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
                if (SSL_is_init_finished(connection) <= 0) {
                    LOG_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
            }
        }
        else {
            std::string tempIP;
            for (int i = 0; hostEntry->h_addr_list[i]; i++) {
                tempIP = inet_ntoa(*(struct in_addr *) hostEntry->h_addr_list[i]);
                if (Connect(tempIP, realPort)) {
                    connected = true;
                    break;
                }
            }
            if (!connected) {
                LOG_ERROR(HttpClientChan, "failed to connect host");
                return false;
            }
        }
        std::string realUrl = url;
        bool isFirst = false;
        if(url.find('?') == std::string::npos) {
            isFirst = true;
        }
        for(const auto& parameter : parameters ){
            if(isFirst) {
                realUrl += '?';
                isFirst = false;
            }
            else realUrl += '&';
            realUrl += fmt::format("{}={}",parameter.first,parameter.second);
        }

        requestHeader.SetMethod(GET);
        requestHeader.SetVersion(HTTP10);
        requestHeader.SetUrl(realUrl);

        for(const auto& header : headers ){
            requestHeader.SetField(header.first,header.second);
        }
        std::string cookieStr;
        isFirst = true;
        for(const auto& cookie : cookies ){
            if(isFirst) {
                isFirst = false;
            }else { cookieStr += ';'; }
            cookieStr += fmt::format("{}={}",cookie.first,cookie.second);
        }
        requestHeader.SetField("Cookies",{cookieStr});

        parseStatus = SendHeader;
        auto status = SUCCESS;
        while((status = SendRequest()) == AGAIN){}
        if(status == FAILED) return false;
        while((status = ParseResponse()) == AGAIN){}
        if(status == FAILED) return false;
        return true;
    }

    bool HttpsClientImpl::Post(
            ProtoType                                                                       type,
            const std::string                                                               &addr,
            unsigned short                                                                  port,
            const std::string                                                               &url,
            const HttpHeaderFieldsType                                                      &headers,
            const HttpCookieType                                                            &cookies,
            const HttpMultiPartType                                                         &files,
            const HttpFormType                                                              &forms,
            const std::unordered_map<std::string, std::pair<std::string,unsigned short>>    &proxies,
            int                                                                             timeout)
    {
        return HttpClientImpl::Post(type, addr, port, url, headers, cookies, files, forms, proxies, timeout);
    }

    bool
    HttpClient::Get(const std::string &url, const HttpHeaderFieldsType &headers, const HttpParameterType &parameters,
                    const HttpCookieType &cookies, const std::unordered_map<std::string, std::pair<std::string,unsigned short>> &proxies,
                    int timeout) {
        if(!parseUrl(url)) return false;
        switch(type) {
            case HTTP : {
                return http.Get(type,requestAddr,port,requestUrl,headers,parameters,cookies,proxies,timeout);
            }
            case HTTPS : {
                return https.Get(type,requestAddr,port,requestUrl,headers,parameters,cookies,proxies,timeout);
            }
        }
        return false;
    }

    bool HttpClient::Post(const std::string &url, const HttpHeaderFieldsType &headers, const HttpCookieType &cookies,
                          const HttpMultiPartType &files, const HttpFormType &forms,
                          const std::unordered_map<std::string, std::pair<std::string,unsigned short>> &proxies, int timeout) {
        if(!parseUrl(url)) return false;
        switch(type) {
            case HTTP : {
                return http.Post(type,requestAddr,port,requestUrl,headers,cookies,files,forms,proxies,timeout);
            }
            case HTTPS : {
                return https.Post(type,requestAddr,port,requestUrl,headers,cookies,files,forms,proxies,timeout);
            }
        }
        return false;
    }

    const HttpBase::RequestHeader &HttpClient::GetRequestHeader() {
        switch (type) {
            case HTTP : return http.GetRequestHeader();
            case HTTPS : return https.GetRequestHeader();
        }
    }

    const HttpBase::RequestBody &HttpClient::GetRequestBody() {
        switch (type) {
            case HTTP : return http.GetRequestBody();
            case HTTPS : return https.GetRequestBody();
        }
    }

    const HttpBase::ResponseHeader &HttpClient::GetResponseHeader() {
        switch (type) {
            case HTTP : return http.GetResponseHeader();
            case HTTPS : return https.GetResponseHeader();
        }
    }

    const HttpBase::ResponseBody &HttpClient::GetResponseBody() {
        switch (type) {
            case HTTP : return http.GetResponseBody();
            case HTTPS : return https.GetResponseBody();
        }
    }
} // totoro