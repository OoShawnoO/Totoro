#ifndef TOTORO_HTTPCLIENT_H
#define TOTORO_HTTPCLIENT_H

#include "http/Http.h"     /* HttpsBase */
#include "SSLSocket.h"

namespace totoro {
    enum ProtoType {
        HTTP, HTTPS
    };

    /**
     * @brief 负责HTTP客户端请求实现 \n Response for HTTP client request Implement
     */
    class HttpClientImpl : public virtual TcpClient,public Http {
    protected:
        virtual bool
        connectToHost(
                const std::string &addr,
                unsigned short port,
                const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
                unsigned int timeout = 0
        );

        virtual bool
        processRequestResponse();

        bool
        processGetRequestParameters(
                const std::string &url,
                const HttpHeaderFieldsType &headers,
                const HttpParameterType &parameters,
                const HttpCookieType &cookies);

        bool
        processPostRequestParameters(
                const std::string &url,
                const HttpHeaderFieldsType &headers,
                const HttpCookieType &cookies,
                const HttpMultiPartType &files,
                const HttpFormType &forms,
                const nlohmann::json &json);

    public:
        virtual bool
        Get(
                ProtoType type,
                const std::string &addr,
                unsigned short port,
                const std::string &url,
                const HttpHeaderFieldsType &headers,
                const HttpParameterType &parameters,
                const HttpCookieType &cookies,
                const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
                int timeout);

        virtual bool
        Post(
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
                int timeout);

        const std::string &GetRequestHeaderText() const;

        const std::string &GetRequestBodyText() const;

        const std::string &GetResponseHeaderText() const;

        const std::string &GetResponseBodyText() const;

        const Http::RequestHeader &GetRequestHeader();

        const Http::RequestBody &GetRequestBody();

        const Http::ResponseHeader &GetResponseHeader();

        const Http::ResponseBody &GetResponseBody();
    };

    /**
     * @brief 负责HTTPS客户端请求实现 \n Response for HTTPS client request Implement
     */
    class HttpsClientImpl : public virtual SSLSocket, public virtual HttpClientImpl {
    protected:
        std::string connectAddr;
        unsigned short connectPort;

        bool
        connectToHost(
                const std::string &addr,
                unsigned short port,
                const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
                unsigned int timeout = 0
        ) override;

        bool
        processRequestResponse() override;

    public:
        bool
        Get(
                ProtoType type,
                const std::string &addr,
                unsigned short port,
                const std::string &url,
                const HttpHeaderFieldsType &headers,
                const HttpParameterType &parameters,
                const HttpCookieType &cookies,
                const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
                int timeout) override;


        bool
        Post(
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
                int timeout) override;
    };

    /**
     * @brief 请求相关参数 \n Parameters for request
     */
    struct HttpRequestParameters {
        // 请求完整url / Request complete url
        std::string url{};
        // 请求头字段 / Request header fields
        HttpHeaderFieldsType headers{};
        // 请求参数 / Request parameters
        HttpParameterType parameters{};
        // 请求cookies / Request cookies
        HttpCookieType cookies{};
        // 请求代理 / Request proxy
        std::unordered_map<std::string,
                std::pair<std::string, unsigned short>> proxies{};
        // 请求文件 / Request files
        HttpMultiPartType files{};
        // 请求表单 / Request form
        HttpFormType forms{};
        // 请求json / Request json
        nlohmann::json json{};
        // 超时时长(ms) / timeout duration(ms)
        int timeout{-1};
    };

    /**
     * @brief 负责HTTP/HTTPS客户端请求 \n Response for HTTP/HTTPS client request
     */
    class HttpClient {
        HttpClientImpl http;
        HttpsClientImpl https;
        ProtoType type;
        unsigned short port;
        std::string requestAddr;
        std::string requestUrl;

        bool parseUrl(const std::string &url);

    public:
        // HTTP Get 请求 / HTTP Get method
        bool Get(const HttpRequestParameters &params);

        // HTTP Post 请求 / HTTP Post method
        bool Post(const HttpRequestParameters &params);

        // 获取响应报文 / Get response text
        const std::string &GetResponseHeaderText() const;

        // 获取请求报文 / Get request text
        const std::string &GetRequestHeaderText() const;

        // 获取响应报文 / Get response text
        const std::string &GetResponseBodyText() const;

        // 获取请求报文 / Get request text
        const std::string &GetRequestBodyText() const;


        // 获取请求头 / Get request header
        const Http::RequestHeader &GetRequestHeader();

        // 获取请求体 / Get request body
        const Http::RequestBody &GetRequestBody();

        // 获取响应头 / Get response header
        const Http::ResponseHeader &GetResponseHeader();

        // 获取响应体 / Get response body
        const Http::ResponseBody &GetResponseBody();

    };


} // totoro

#endif //TOTORO_HTTPCLIENT_H
