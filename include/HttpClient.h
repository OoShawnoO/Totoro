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
        const HttpBase::RequestHeader& GetRequestHeader();
        const HttpBase::RequestBody& GetRequestBody();
        const HttpBase::ResponseHeader& GetResponseHeader();
        const HttpBase::ResponseBody& GetResponseBody();
    };
    /**
     * @brief 负责HTTPS客户端请求实现 \n Response for HTTPS client request Implement
     */
    class HttpsClientImpl : protected virtual HttpsBase,public virtual HttpClientImpl {
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
        bool Get(const std::string& url,
                const HttpHeaderFieldsType& headers = {},
                const HttpParameterType& parameters={},
                const HttpCookieType& cookies = {},
                const std::unordered_map<std::string,std::pair<std::string,unsigned short>>& proxies = {},
                int timeout = -1
        );
        bool Post(const std::string& url,
                 const HttpHeaderFieldsType& headers = {},
                 const HttpCookieType& cookies = {},
                 const HttpMultiPartType& files = {},
                 const HttpFormType& forms = {},
                 const std::unordered_map<std::string,std::pair<std::string,unsigned short>>& proxies = {},
                 int timeout = -1
        );

        const HttpBase::RequestHeader& GetRequestHeader();
        const HttpBase::RequestBody& GetRequestBody();
        const HttpBase::ResponseHeader& GetResponseHeader();
        const HttpBase::ResponseBody& GetResponseBody();
    };


} // totoro

#endif //TOTORO_HTTPCLIENT_H
