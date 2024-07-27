#include <netdb.h>
#include <iostream>
#include <future>
#include "http/HttpClient.h"
#include "fmt/format.h"

const std::string HttpClientChan = "Totoro";
namespace totoro {

//#define TIMEOUT(function, _timeout) do {                                \
//    std::future<bool> result = std::async(std::launch::async,[&]{       \
//        return function;                                                \
//    });                                                                 \
//    if(timeout > 0){                                                    \
//        std::chrono::milliseconds to(_timeout);                         \
//        if(result.wait_for(to) == std::future_status::timeout){         \
//            MOLE_ERROR(HttpClientChan,"timeout");                       \
//            Close();                                                    \
//            return false;                                               \
//        }                                                               \
//    }                                                                   \
//    if(!result.get()) return false;                                     \
//}while(0)

    bool
    HttpClientImpl::connectToHost(
            const std::string &addr,
            unsigned short port,
            const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
            unsigned int timeout
    ) {
        std::string realAddr = addr;
        unsigned short realPort = port;
        auto proxy = proxies.begin();
        if ((proxy = proxies.find("http")) != proxies.end()) {
            realAddr = proxy->second.first;
            realPort = proxy->second.second;
            request.header.SetField("Host", {fmt::format("{}:{}", addr, port)});
        }

        auto host = gethostbyname(realAddr.c_str());
        if (!host) {
            MOLE_ERROR(HttpClientChan, "failed to get host name");
            return false;
        }
        bool connected = false;
        std::string tempIP;
        for (int i = 0; host->h_addr_list[i]; i++) {
            tempIP = inet_ntoa(*(struct in_addr *) host->h_addr_list[i]);
            if (Connect(tempIP, realPort,timeout)) {
                connected = true;
                break;
            }
        }
        if (!connected) {
            MOLE_ERROR(HttpClientChan, "failed to connect host");
            return false;
        }
        return true;
    }

    bool
    HttpClientImpl::processGetRequestParameters(
            const std::string &url,
            const HttpHeaderFieldsType &headers,
            const HttpParameterType &parameters,
            const HttpCookieType &cookies) {
        std::string realUrl = url;
        bool isFirst = false;
        if (url.find('?') == std::string::npos) {
            isFirst = true;
        }
        for (const auto &parameter: parameters) {
            if (isFirst) {
                realUrl += '?';
                isFirst = false;
            } else realUrl += '&';
            realUrl += fmt::format("{}={}", parameter.first, parameter.second);
        }

        request.header.SetMethod(GET);
        request.header.SetVersion(HTTP11);
        request.header.SetUrl(realUrl);

        for (const auto &header: headers) {
            request.header.SetField(header.first, header.second);
        }
        std::string cookieStr;
        isFirst = true;
        for (const auto &cookie: cookies) {
            if (isFirst) {
                isFirst = false;
            } else { cookieStr += ';'; }
            cookieStr += fmt::format("{}={}", cookie.first, cookie.second);
        }
        if (!cookieStr.empty()) {
            request.header.SetField("Cookies", {cookieStr});
        }

        return true;
    }

    bool HttpClientImpl::processPostRequestParameters(
            const std::string &url,
            const HttpHeaderFieldsType &headers,
            const HttpCookieType &cookies,
            const HttpMultiPartType &files,
            const HttpFormType &forms,
            const nlohmann::json &json) {
        request.header.SetMethod(POST);
        request.header.SetVersion(HTTP11);
        request.header.SetUrl(url);
        for (const auto &header: headers) {
            request.header.SetField(header.first, header.second);
        }
        std::string cookieStr;
        bool isFirst = true;
        for (const auto &cookie: cookies) {
            if (isFirst) {
                isFirst = false;
            } else { cookieStr += ';'; }
            cookieStr += fmt::format("{}={}", cookie.first, cookie.second);
        }
        if (!cookieStr.empty()) {
            request.header.SetField("Cookies", {cookieStr});
        }
        if (!files.empty()) {
            request.header.SetField("Content-Type", {"multipart/form-data"});
            for (const auto &file: files) {
                request.body.SetFilesFieldData(file.first, file.second);
            }
        } else if (!forms.empty()) {
            request.header.SetField("Content-Type", {"application/x-www-form-urlencoded"});
            request.body.SetForm(forms);
        } else if (!json.empty()) {
            request.header.SetField("Content-Type", {"application/json"});
            request.body.SetJson(json);
        } else return false;

        return true;
    }

    bool
    HttpClientImpl::processRequestResponse() {
        auto method = request.header.GetMethod();

        if(method == HttpMethod::POST || method == HttpMethod::PATCH || method == HttpMethod::PUT) {
            request_body_text = request.body.toString(request.header);
            request.header.SetContentLength(request_body_text.size());
        }

        if(request_header_text.empty()) {
            request_header_text = request.header.toString();
        }

        TcpSocket::SendAll(request_header_text);

        if(method == HttpMethod::POST || method == HttpMethod::PATCH || method == HttpMethod::PUT) {
            TcpSocket::SendAll(request_body_text);
        }

        if(!TcpSocket::RecvUntil(response_header_text,"\r\n\r\n")) {
            MOLE_ERROR("Totoro","recv request header failed");
            return false;
        }
        if(!response.header.Parse(response_header_text)) {
            MOLE_ERROR("Totoro","parse request header failed");
            return false;
        }
        if(response.header.GetContentLength() != 0){
            if(Recv(response_body_text,response.header.GetContentLength()) != response.header.GetContentLength()){
                MOLE_ERROR("Totoro","recv request body failed");
                return false;
            }
        }else if(response.header.GetTransferEncoding() == "chunked"){
            // TODO chunked
        }else{
            return true;
        }

        return true;
        
    }

    bool
    HttpClientImpl::Get(
            ProtoType type,
            const std::string &addr,
            unsigned short port,
            const std::string &url,
            const HttpHeaderFieldsType &headers,
            const HttpParameterType &parameters,
            const HttpCookieType &cookies,
            const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
            int timeout) {
//        TIMEOUT(connectToHost(addr, port, proxies,timeout), timeout);
        if(!connectToHost(addr,port,proxies,timeout)) {
            return false;
        }

        processGetRequestParameters(url, headers, parameters, cookies);

//        TIMEOUT(processRequestResponse(), timeout);
        processRequestResponse();

        Close();
        return true;
    }

    bool
    HttpClientImpl::Post(
            ProtoType type,
            const std::string &addr,
            unsigned short port,
            const std::string &url,
            const HttpHeaderFieldsType &headers,
            const HttpCookieType &cookies,
            const HttpMultiPartType &files,
            const HttpFormType &forms,
            const nlohmann::json &json,
            const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
            int timeout) {
//        TIMEOUT(connectToHost(addr, port, proxies,timeout), timeout);
        if(!connectToHost(addr,port,proxies,timeout)) {
            return false;
        }
        if (!processPostRequestParameters(url, headers, cookies, files, forms, json)) return false;
//        TIMEOUT(processRequestResponse(), timeout);
        processRequestResponse();
        Close();
        return true;
    }

    const Http::RequestHeader &HttpClientImpl::GetRequestHeader() { return request.header; }

    const Http::RequestBody &HttpClientImpl::GetRequestBody() { return request.body; }

    const Http::ResponseHeader &HttpClientImpl::GetResponseHeader() { return response.header; }

    const Http::ResponseBody &HttpClientImpl::GetResponseBody() { return response.body; }

    const std::string &HttpClientImpl::GetRequestHeaderText() const {
        return request_header_text;
    }

    const std::string &HttpClientImpl::GetRequestBodyText() const {
        return request_body_text;
    }

    const std::string &HttpClientImpl::GetResponseHeaderText() const {
        return response_header_text;
    }

    const std::string &HttpClientImpl::GetResponseBodyText() const {
        return response_body_text;
    }


    /* -------------------------- HttpsClient ------------------------------- */


    bool HttpsClientImpl::connectToHost(
            const std::string &addr,
            unsigned short port,
            const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
            unsigned int timeout
    ) {
        connectAddr = addr;
        connectPort = port;
        std::string realAddr = addr;
        unsigned short realPort = port;
        auto proxy = proxies.begin();

        if ((proxy = proxies.find("https")) != proxies.end()) {
            realAddr = proxy->second.first;
            realPort = proxy->second.second;
            request.header.SetField("Host", {fmt::format("{}:{}", addr, port)});
        }

        auto host = gethostbyname(realAddr.c_str());
        if (!host) {
            MOLE_ERROR(HttpClientChan, "failed to get host name");
            return false;
        }
        bool connected = false;
        if (proxy != proxies.end()) {
            std::string tempIP;
            for (int i = 0; host->h_addr_list[i]; i++) {
                tempIP = inet_ntoa(*(struct in_addr *) host->h_addr_list[i]);
                if (TcpClient::Connect(tempIP, realPort,timeout)) {
                    connected = true;
                    break;
                }
            }
            if (!connected) {
                MOLE_ERROR(HttpClientChan, "failed to connect host");
                return false;
            }
            if (TcpSocket::SendAll(
                    fmt::format("{} {} {}\r\n\r\n", "CONNECT", fmt::format("{}:{}", connectAddr, connectPort),
                                "HTTP/1.1")) < 0) {
                MOLE_ERROR(HttpClientChan, "send connect failed");
                return false;
            }
            TcpSocket::RecvUntil(response_header_text,"\r\n\r\n");
            if (response_header_text.rfind("Connection established") == std::string::npos) {
                MOLE_ERROR(HttpClientChan, "connect proxy failed");
                return false;
            }

            char buffer[4096] = {0};
            if (!connection) {
                connection = SSL_new(GetClientContext().context);
                if (!connection) {
                    MOLE_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
                if (SSL_set_fd(connection, sock) <= 0) {
                    MOLE_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
                if (SSL_connect(connection) <= 0) {
                    MOLE_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
                if (SSL_is_init_finished(connection) <= 0) {
                    MOLE_ERROR(HttpClientChan, ERR_error_string(ERR_get_error(), buffer));
                    return false;
                }
            }
        } else {
            std::string tempIP;
            for (int i = 0; host->h_addr_list[i]; i++) {
                tempIP = inet_ntoa(*(struct in_addr *) host->h_addr_list[i]);
                if (Connect(tempIP, port,timeout)) {
                    connected = true;
                    break;
                }
            }
            if (!connected) {
                MOLE_ERROR(HttpClientChan, "failed to connect host");
                return false;
            }
        }
        return true;
    }

    bool HttpsClientImpl::processRequestResponse() {
        auto method = request.header.GetMethod();

        if(method == HttpMethod::POST || method == HttpMethod::PATCH || method == HttpMethod::PUT) {
            request_body_text = request.body.toString(request.header);
            request.header.SetContentLength(request_body_text.size());
        }

        if(request_header_text.empty()) {
            request_header_text = request.header.toString();
        }

        SSLSocket::SendAll(request_header_text);

        if(method == HttpMethod::POST || method == HttpMethod::PATCH || method == HttpMethod::PUT) {
            SSLSocket::SendAll(request_body_text);
        }

        if(!SSLSocket::RecvUntil(response_header_text,"\r\n\r\n")) {
            MOLE_ERROR("Totoro","recv request header failed");
            return false;
        }
        if(!response.header.Parse(response_header_text)) {
            MOLE_ERROR("Totoro","parse request header failed");
            return false;
        }
        if(response.header.GetContentLength() != 0){
            if(SSLSocket::Recv(response_body_text,response.header.GetContentLength()) != response.header.GetContentLength()){
                MOLE_ERROR("Totoro","recv request body failed");
                return false;
            }
        }else if(response.header.GetTransferEncoding() == "chunked"){
            // TODO chunked
        }else{
            return true;
        }

        return true;
    }

    bool
    HttpsClientImpl::Get(
            ProtoType type,
            const std::string &addr,
            unsigned short port,
            const std::string &url,
            const HttpHeaderFieldsType &headers,
            const HttpParameterType &parameters,
            const HttpCookieType &cookies,
            const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
            int timeout) {
//        TIMEOUT(connectToHost(addr, port, proxies,timeout), timeout);
        if(!connectToHost(addr,port,proxies,timeout)) {
            return false;
        }
//        if(!proxies.empty()) {
//            HttpClientImpl::processGetRequestParameters(url, headers, parameters, cookies);
//        }else{
            processGetRequestParameters(url, headers, parameters, cookies);
//        }

//        TIMEOUT(processRequestResponse(), timeout);
//        if(!proxies.empty()) {
//            HttpClientImpl::processRequestResponse();
//        }else{
            processRequestResponse();
//        }

        Close();
        return true;
    }

    bool
    HttpsClientImpl::Post(
            ProtoType type,
            const std::string &addr,
            unsigned short port,
            const std::string &url,
            const HttpHeaderFieldsType &headers,
            const HttpCookieType &cookies,
            const HttpMultiPartType &files,
            const HttpFormType &forms,
            const nlohmann::json &json,
            const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
            int timeout) {
//        TIMEOUT(connectToHost(addr, port, proxies,timeout), timeout);
        if(!connectToHost(addr,port,proxies,timeout)) {
            return false;
        }
        if(!proxies.empty()) {
            HttpClientImpl::processPostRequestParameters(url, headers, cookies, files, forms, json);
        }else{
            processPostRequestParameters(url, headers, cookies, files, forms, json);
        }

//        TIMEOUT(processRequestResponse(), timeout);
        if(!proxies.empty()) {
            HttpClientImpl::processRequestResponse();
        }else{
            processRequestResponse();
        }

        Close();
        return true;
    }


    /* ----------------------------- HttpClient ------------------------------------*/


    bool
    HttpClient::parseUrl(const std::string &url) {
        std::string proto = url.substr(0, 5);
        std::string tempUrl, tempAddr;
        if (proto == "http:") {
            type = HTTP;
            tempUrl = url.substr(7);
            auto pos = tempUrl.find('/');
            if (pos == std::string::npos) requestUrl = "/";
            else requestUrl = tempUrl.substr(pos);
            tempAddr = tempUrl.substr(0, pos);
            pos = tempAddr.find(':');
            if (pos == std::string::npos) port = 80;
            else port = stoi(tempAddr.substr(pos + 1));
            requestAddr = tempAddr.substr(0, pos);
            return true;
        } else if (proto == "https") {
            type = HTTPS;
            tempUrl = url.substr(8);
            auto pos = tempUrl.find('/');
            if (pos == std::string::npos) requestUrl = "/";
            else requestUrl = tempUrl.substr(pos);
            tempAddr = tempUrl.substr(0, pos);
            pos = tempAddr.find(':');
            if (pos == std::string::npos) port = 443;
            else port = stoi(tempAddr.substr(pos + 1));
            requestAddr = tempAddr.substr(0, pos);
            return true;
        } else return false;
    }

    bool
    HttpClient::Get(const HttpRequestParameters &params) {
        if (!parseUrl(params.url)) return false;
        switch (type) {
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
    HttpClient::Post(const HttpRequestParameters &params) {
        if (!parseUrl(params.url)) return false;
        switch (type) {
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

    const Http::RequestHeader &HttpClient::GetRequestHeader() {
        switch (type) {
            case HTTP :
                return http.GetRequestHeader();
            case HTTPS :
                return https.GetRequestHeader();
        }
        return http.GetRequestHeader();
    }

    const Http::RequestBody &HttpClient::GetRequestBody() {
        switch (type) {
            case HTTP :
                return http.GetRequestBody();
            case HTTPS :
                return https.GetRequestBody();
        }
        return http.GetRequestBody();
    }

    const Http::ResponseHeader &HttpClient::GetResponseHeader() {
        switch (type) {
            case HTTP :
                return http.GetResponseHeader();
            case HTTPS :
                return https.GetResponseHeader();
        }
        return http.GetResponseHeader();
    }

    const Http::ResponseBody &HttpClient::GetResponseBody() {
        switch (type) {
            case HTTP :
                return http.GetResponseBody();
            case HTTPS :
                return https.GetResponseBody();
        }
        return http.GetResponseBody();
    }


    const std::string &HttpClient::GetResponseHeaderText() const {
        return type == HTTP ? http.GetResponseHeaderText() : https.GetResponseHeaderText();
    }

    const std::string &HttpClient::GetRequestHeaderText() const {
        return type == HTTP ? http.GetRequestHeaderText() : https.GetRequestHeaderText();
    }

    const std::string &HttpClient::GetResponseBodyText() const {
        return type == HTTP ? http.GetResponseBodyText() : https.GetResponseBodyText();
    }

    const std::string &HttpClient::GetRequestBodyText() const {
        return type == HTTP ? http.GetRequestBodyText() : https.GetRequestBodyText();
    }
} // totoro