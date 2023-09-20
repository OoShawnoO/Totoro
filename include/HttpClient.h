#ifndef TOTORO_HTTPCLIENT_H
#define TOTORO_HTTPCLIENT_H

#include "core/HttpsBase.h"

namespace totoro {
    enum ProtoType {
        HTTP,HTTPS
    };
    /**
     * @brief 负责HTTP客户端请求实现 \n Response for HTTP client request Implement
     */
    class HttpClientImpl : protected virtual HttpBase {
        virtual bool connectToHost(const std::string& addr,unsigned short port,bool isProxy = false);
    public:
        virtual bool Get(
                ProtoType type,
                const std::string& addr,
                unsigned short port,
                const std::string& url,
                const HttpHeaderFieldsType& headers = {},
                const HttpParameterType& parameters={},
                const HttpCookieType& cookies = {},
                const std::unordered_map<std::string,std::pair<std::string,unsigned short>>& proxies = {},
                int timeout = -1
                );
        virtual bool Post(
                ProtoType type,
                const std::string& addr,
                unsigned short port,
                const std::string& url,
                const HttpHeaderFieldsType& headers = {},
                const HttpCookieType& cookies = {},
                const HttpMultiPartType& files = {},
                const HttpFormType& forms = {},
                const std::unordered_map<std::string,std::pair<std::string,unsigned short>>& proxies = {},
                int timeout = -1
                );
        const std::string& GetRequestText() const;
        const std::string& GetResponseText() const;
        std::string GetResponseContent() const;
        const HttpBase::RequestHeader& GetRequestHeader();
        const HttpBase::RequestBody& GetRequestBody();
        const HttpBase::ResponseHeader& GetResponseHeader();
        const HttpBase::ResponseBody& GetResponseBody();
    };
    /**
     * @brief 负责HTTPS客户端请求实现 \n Response for HTTPS client request Implement
     */
    class HttpsClientImpl : protected virtual HttpsBase,public virtual HttpClientImpl {
        bool connectToHost(const std::string &addr, unsigned short port,bool isProxy = false) override;
        std::string connectAddr;
        unsigned short connectPort;
    public:
        bool Get(ProtoType type, const std::string &addr, unsigned short port, const std::string &url,
                const HttpHeaderFieldsType &headers, const HttpParameterType &parameters, const HttpCookieType &cookies,
                const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
                int timeout) override;

        bool Post(ProtoType type, const std::string &addr, unsigned short port, const std::string &url,
                 const HttpHeaderFieldsType &headers, const HttpCookieType &cookies, const HttpMultiPartType &files,
                 const HttpFormType &forms,
                 const std::unordered_map<std::string, std::pair<std::string, unsigned short>> &proxies,
                 int timeout) override;
    };

    /**
     * @brief 请求相关参数 \n Parameters for request
     */
     struct HttpRequestParameters{
         // 请求完整url / Request complete url
         std::string                                url{};
         // 请求头字段 / Request header fields
         HttpHeaderFieldsType                       headers{};
         // 请求参数 / Request parameters
         HttpParameterType                          parameters{};
         // 请求cookies / Request cookies
         HttpCookieType                             cookies{};
         // 请求代理 / Request proxy
         std::unordered_map<std::string,
         std::pair<std::string,unsigned short>>     proxies{};
         // 请求文件 / Request files
         const HttpMultiPartType                    files{};
         // 请求表单 / Request form
         const HttpFormType                         forms{};
         // 超时时长(ms) / timeout duration(ms)
         int                                        timeout{-1};
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
        bool parseUrl(const std::string& url);
    public:
        bool Get(const HttpRequestParameters& params);
        bool Post(const HttpRequestParameters& params);

        std::string GetResponseContent();
        const std::string& GetResponseText() const;
        const std::string& GetRequestText() const;

        const HttpBase::RequestHeader& GetRequestHeader();
        const HttpBase::RequestBody& GetRequestBody();
        const HttpBase::ResponseHeader& GetResponseHeader();
        const HttpBase::ResponseBody& GetResponseBody();

    };


} // totoro

#endif //TOTORO_HTTPCLIENT_H
