#include <netdb.h>
#include <iostream>
#include <future>
#include "HttpClient.h"
#include "fmt/format.h"

const std::string HttpClientChan = "HttpClient";
namespace totoro {

#define TIMEOUT(function,_timeout) do {                                 \
    std::future<bool> result = std::async(std::launch::async,[&]{       \
        return function;                                                \
    });                                                                 \
    if(timeout > 0){                                                    \
        std::chrono::milliseconds to(_timeout);                         \
        if(result.wait_for(to) == std::future_status::timeout){         \
            LOG_ERROR(HttpClientChan,"timeout");                        \
            Close();                                                    \
            return false;                                               \
        }                                                               \
    }                                                                   \
    if(!result.get()) return false;                                     \
}while(0)

    bool
    HttpClientImpl::connectToHost(
    const std::string&                                                              addr,
    unsigned short                                                                  port,
    const std::unordered_map<std::string,std::pair<std::string,unsigned short>>     &proxies)
    {
        std::string realAddr = addr;
        unsigned short realPort = port;
        auto proxy = proxies.begin();
        if((proxy = proxies.find("http")) != proxies.end()){
            realAddr = proxy->second.first;
            realPort = proxy->second.second;
            requestHeader.SetField("Host",{fmt::format("{}:{}",addr,port)});
        }

        auto host = gethostbyname(realAddr.c_str());
        if(!host) {
            LOG_ERROR(HttpClientChan,"failed to get host name");
            return false;
        }
        bool connected = false;
        std::string tempIP;
        for(int i=0;host->h_addr_list[i];i++){
            tempIP = inet_ntoa(*(struct in_addr*)host->h_addr_list[i]);
            if(Connect(tempIP,realPort)){ connected = true;break; }
        }
        if(!connected) {
            LOG_ERROR(HttpClientChan,"failed to connect host");
            return false;
        }
        return true;
    }

    bool
    HttpClientImpl::processGetRequestParameters(
    const std::string                                                               &url,
    const HttpHeaderFieldsType                                                      &headers,
    const HttpParameterType                                                         &parameters,
    const HttpCookieType                                                            &cookies)
    {
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
        if(!cookieStr.empty()) {
            requestHeader.SetField("Cookies", {cookieStr});
        }

        parseStatus = SendHeader;
        return true;
    }

    bool HttpClientImpl::processPostRequestParameters(
            const std::string                                                               &url,
            const HttpHeaderFieldsType                                                      &headers,
            const HttpCookieType                                                            &cookies,
            const HttpMultiPartType                                                         &files,
            const HttpFormType                                                              &forms,
            const nlohmann::json                                                            &json)
    {
        requestHeader.SetMethod(POST);
        requestHeader.SetVersion(HTTP11);
        requestHeader.SetUrl(url);
        for(const auto& header : headers ){
            requestHeader.SetField(header.first,header.second);
        }
        std::string cookieStr;
        bool isFirst = true;
        for(const auto& cookie : cookies ){
            if(isFirst) {
                isFirst = false;
            }else { cookieStr += ';'; }
            cookieStr += fmt::format("{}={}",cookie.first,cookie.second);
        }
        if(!cookieStr.empty()) {
            requestHeader.SetField("Cookies", {cookieStr});
        }
        if(!files.empty()) {
            requestHeader.SetField("Content-Type", {"multipart/form-data"});
            for(const auto& file : files) {
                requestBody.SetFilesFieldData(file.first,file.second);
            }
        }
        else if(!forms.empty()) {
            requestHeader.SetField("Content-Type", {"application/x-www-form-urlencoded"});
            requestBody.SetForm(forms);
        }
        else if(!json.empty()) {
            requestHeader.SetField("Content-Type", {"application/json"});
            requestBody.SetJson(json);
        }
        else return false;

        parseStatus = SendHeader;
        return true;
    }

    bool
    HttpClientImpl::processRequestResponse()
    {
        decltype(SUCCESS) status;
        while((status = SendRequest()) == AGAIN){}
        if(status == FAILED) return false;
        while((status = ParseResponse()) == AGAIN){}
        if(status == FAILED) return false;
        return true;
    }

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
        TIMEOUT(connectToHost(addr,port,proxies),timeout);

        processGetRequestParameters(url,headers,parameters,cookies);

        TIMEOUT(processRequestResponse(),timeout);

        HttpBase::Close();
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
    const nlohmann::json                                                            &json,
    const std::unordered_map<std::string, std::pair<std::string,unsigned short>>    &proxies,
    int                                                                             timeout)
    {
        TIMEOUT(connectToHost(addr,port,proxies),timeout);
        if(!processPostRequestParameters(url,headers,cookies,files,forms,json)) return false;
        TIMEOUT(processRequestResponse(),timeout);
        HttpBase::Close();
        return true;
    }

    const HttpBase::RequestHeader &HttpClientImpl::GetRequestHeader() { return requestHeader; }

    const HttpBase::RequestBody &HttpClientImpl::GetRequestBody() { return requestBody; }

    const HttpBase::ResponseHeader &HttpClientImpl::GetResponseHeader() { return responseHeader; }

    const HttpBase::ResponseBody &HttpClientImpl::GetResponseBody() { return responseBody; }

    const std::string &HttpClientImpl::GetRequestText() const { return requestText; }

    const std::string &HttpClientImpl::GetResponseText() const { return responseText; }

    std::string HttpClientImpl::GetResponseContent() const { return responseText.substr(responseHeaderEnd); }


    /* -------------------------- HttpsClient ------------------------------- */


    bool HttpsClientImpl::connectToHost(
    const std::string                                                               &addr,
    unsigned short                                                                  port,
    const std::unordered_map<std::string,std::pair<std::string,unsigned short>>     &proxies)
    {
        connectAddr = addr;
        connectPort = port;
        std::string realAddr = addr;
        unsigned short realPort = port;
        auto proxy = proxies.begin();

        if((proxy = proxies.find("https")) != proxies.end()){
            realAddr = proxy->second.first;
            realPort = proxy->second.second;
        }

        auto host = gethostbyname(realAddr.c_str());
        if(!host) {
            LOG_ERROR(HttpClientChan,"failed to get host name");
            return false;
        }
        bool connected = false;
        if(proxy != proxies.end()) {
            std::string tempIP;
            for (int i = 0; host->h_addr_list[i]; i++) {
                tempIP = inet_ntoa(*(struct in_addr *) host->h_addr_list[i]);
                if (TCPSocket::Connect(tempIP, realPort)) {
                    connected = true;
                    break;
                }
            }
            if (!connected) {
                LOG_ERROR(HttpClientChan, "failed to connect host");
                return false;
            }
            if (TCPSocket::SendAll(fmt::format("{} {} {}\r\n\r\n","CONNECT", fmt::format("{}:{}", connectAddr, connectPort), "HTTP/1.1")) < 0) {
                LOG_ERROR(HttpClientChan, "send connect failed");
                return false;
            }
            while (!TCPSocket::RecvAll(data)) {}
            if (data.rfind("Connection established") == std::string::npos) {
                LOG_ERROR(HttpClientChan, "connect proxy failed");
                return false;
            }
            ClearData();
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
            for (int i = 0; host->h_addr_list[i]; i++) {
                tempIP = inet_ntoa(*(struct in_addr *) host->h_addr_list[i]);
                if (Connect(tempIP, port)) {
                    connected = true;
                    break;
                }
            }
            if (!connected) {
                LOG_ERROR(HttpClientChan, "failed to connect host");
                return false;
            }
        }
        return true;
    }

    bool HttpsClientImpl::processRequestResponse()
    {
        return HttpClientImpl::processRequestResponse();
    }

    bool
    HttpsClientImpl::Get(
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
        TIMEOUT(connectToHost(addr,port,proxies),timeout);

        processGetRequestParameters(url,headers,parameters,cookies);

        TIMEOUT(processRequestResponse(),timeout);

        HttpsBase::Close();
        return true;
    }

    bool
    HttpsClientImpl::Post(
    ProtoType                                                                       type,
    const std::string                                                               &addr,
    unsigned short                                                                  port,
    const std::string                                                               &url,
    const HttpHeaderFieldsType                                                      &headers,
    const HttpCookieType                                                            &cookies,
    const HttpMultiPartType                                                         &files,
    const HttpFormType                                                              &forms,
    const nlohmann::json                                                            &json,
    const std::unordered_map<std::string, std::pair<std::string,unsigned short>>    &proxies,
    int                                                                             timeout)
    {
        TIMEOUT(connectToHost(addr,port,proxies),timeout);
        if(!processPostRequestParameters(url,headers,cookies,files,forms,json)) return false;
        TIMEOUT(processRequestResponse(),timeout);
        HttpsBase::Close();
        return true;
    }


    /* ----------------------------- HttpClient ------------------------------------*/


    bool
    HttpClient::parseUrl(const std::string &url) {
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

    bool
    HttpClient::Get(const HttpRequestParameters& params) {
        if(!parseUrl(params.url)) return false;
        switch(type) {
            case HTTP : {
                return http.Get(type,
                                requestAddr,
                                port,
                                requestUrl,
                                params.headers,
                                params.parameters,
                                params.cookies,
                                params.proxies,
                                params.timeout);
            }
            case HTTPS : {
                return https.Get(type,
                                 requestAddr,
                                 port,
                                 requestUrl,
                                 params.headers,
                                 params.parameters,
                                 params.cookies,
                                 params.proxies,
                                 params.timeout);
            }
        }
        return false;
    }

    bool
    HttpClient::Post(const HttpRequestParameters& params) {
        if(!parseUrl(params.url)) return false;
        switch(type) {
            case HTTP : {
                return http.Post(type,
                                 requestAddr,
                                 port,
                                 requestUrl,
                                 params.headers,
                                 params.cookies,
                                 params.files,
                                 params.forms,
                                 params.json,
                                 params.proxies,
                                 params.timeout);
            }
            case HTTPS : {
                return https.Post(type,
                                  requestAddr,
                                  port,
                                  requestUrl,
                                  params.headers,
                                  params.cookies,
                                  params.files,
                                  params.forms,
                                  params.json,
                                  params.proxies,
                                  params.timeout);
            }
        }
        return false;
    }

    const HttpBase::RequestHeader &HttpClient::GetRequestHeader() {
        switch (type) {
            case HTTP : return http.GetRequestHeader();
            case HTTPS : return https.GetRequestHeader();
        }
        return http.GetRequestHeader();
    }

    const HttpBase::RequestBody &HttpClient::GetRequestBody() {
        switch (type) {
            case HTTP : return http.GetRequestBody();
            case HTTPS : return https.GetRequestBody();
        }
        return http.GetRequestBody();
    }

    const HttpBase::ResponseHeader &HttpClient::GetResponseHeader() {
        switch (type) {
            case HTTP : return http.GetResponseHeader();
            case HTTPS : return https.GetResponseHeader();
        }
        return http.GetResponseHeader();
    }

    const HttpBase::ResponseBody &HttpClient::GetResponseBody() {
        switch (type) {
            case HTTP : return http.GetResponseBody();
            case HTTPS : return https.GetResponseBody();
        }
        return http.GetResponseBody();
    }

    const std::string &HttpClient::GetResponseText() const {
        switch(type) {
            case HTTP : return http.GetResponseText();
            case HTTPS : return https.GetResponseText();
        }
        return http.GetResponseText();
    }

    const std::string &HttpClient::GetRequestText() const {
        switch(type) {
            case HTTP : return http.GetRequestText();
            case HTTPS : return https.GetRequestText();
        }
        return http.GetRequestText();
    }

    std::string HttpClient::GetResponseContent() {
        switch(type) {
            case HTTP : return http.GetResponseContent();
            case HTTPS : return https.GetResponseContent();
        }
        return http.GetResponseContent();
    }
} // totoro