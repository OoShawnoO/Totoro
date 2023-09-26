#ifndef TOTORO_HTTPSERVER_H
#define TOTORO_HTTPSERVER_H

#include "core/HttpBase.h"
#include "core/Acceptor.h"
#include "core/HttpsBase.h"

namespace totoro {
    /* HTTP 服务器连接 / HTTP server connection */
    class HttpServerImpl : public virtual HttpBase {
        template<typename T>
        using Ptr = std::shared_ptr<T>;
        struct FilterNode {
            std::unordered_map<std::string,Ptr<FilterNode>> children;
            std::vector<HttpMethod> notAllowed;
            bool isAllowed(HttpMethod method) const;
            void addChild(const std::string& url,
                          std::vector<HttpMethod>& method);
            void addNotAllowed(std::vector<HttpMethod>& method);
        };

        bool isAllowed(const std::string& url,HttpMethod method);
        bool handler();
    public:
        using HandlerType = std::function<bool(const HttpRequest&,HttpResponse&)>;
        using HandlerMapType = std::unordered_map<unsigned short,std::unordered_map<std::string,std::unordered_map<HttpMethod,HandlerType>>>;
        using FilterMapType = std::unordered_map<unsigned short,FilterNode>;
        // 注册GET请求路由 / Register Get method router
        static void Get(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册POST请求路由 / Register Post method router
        static void Post(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册PUT请求路由 / Register Get method router
        static void Put(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册PATCH请求路由 / Register Patch method router
        static void Patch(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册DELETE请求路由 / Register Delete method router
        static void Delete(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册TRACE请求路由 / Register Trace method router
        static void Trace(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册HEAD请求路由 / Register Head method router
        static void Head(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册OPTIONS请求路由 / Register Options method router
        static void Options(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册CONNECT请求路由 / Register Connect method router
        static void Connect(unsigned short port,const std::string& url,const HandlerType& handler);
        // 注册过滤器 / Register filter
        static void Filter(unsigned short port,const std::string &url,std::vector<HttpMethod>& method);
    private:
        static HandlerMapType handlerMap;
        static FilterMapType filterMap;
        static std::string DirResourceHtml(const std::string& dirPath);
    protected:
        bool GetHandler() override;
        bool PostHandler() override;
        bool PutHandler() override;
        bool PatchHandler() override;
        bool DeleteHandler() override;
        bool TraceHandler() override;
        bool HeadHandler() override;
        bool OptionsHandler() override;
        bool ConnectHandler() override;
    };
    /* HTTPS 服务器连接 / HTTPS server connection */
    class HttpsServerImpl : public HttpServerImpl,public HttpsBase {
        using HandlerType = std::function<bool(const HttpRequest&,HttpResponse&)>;
        using HandlerMapType = std::unordered_map<unsigned short,std::unordered_map<std::string,std::unordered_map<HttpMethod,HandlerType>>>;
        static HandlerMapType handlerMap;
        static FilterMapType filterMap;
    };

    /* HTTP 路由服务器 / HTTP Router Server */
    class HttpServer : public Acceptor<Epoller<HttpServerImpl>> {
    public:
        using HandlerType = std::function<bool(const HttpBase::HttpRequest&,HttpBase::HttpResponse&)>;

        HttpServer(bool& _isStop,const Json& config);

        // 注册GET请求路由 / Register Get method router
        void Get(const std::string& url,const HandlerType& handler);
        // 注册POST请求路由 / Register Post method router
        void Post(const std::string& url,const HandlerType& handler);
        // 注册PUT请求路由 / Register Get method router
        void Put(const std::string& url,const HandlerType& handler);
        // 注册PATCH请求路由 / Register Patch method router
        void Patch(const std::string& url,const HandlerType& handler);
        // 注册DELETE请求路由 / Register Delete method router
        void Delete(const std::string& url,const HandlerType& handler);
        // 注册TRACE请求路由 / Register Trace method router
        void Trace(const std::string& url,const HandlerType& handler);
        // 注册HEAD请求路由 / Register Head method router
        void Head(const std::string& url,const HandlerType& handler);
        // 注册OPTIONS请求路由 / Register Options method router
        void Options(const std::string& url,const HandlerType& handler);
        // 注册CONNECT请求路由 / Register Connect method router
        void Connect(const std::string& url,const HandlerType& handler);
        // 注册过滤器 / Register filter
        void Filter(const std::string &url,std::vector<HttpMethod>&& method);
    };
    /* HTTPS 路由服务器 / HTTPS Router Server */
    class HttpsServer : public Acceptor<Epoller<HttpsServerImpl>> {
        using HandlerType = std::function<bool(const HttpBase::HttpRequest&,HttpBase::HttpResponse&)>;
    public:
        HttpsServer(bool& _isStop,const Json& config);

        // 注册GET请求路由 / Register Get method router
        void Get(const std::string& url,const HandlerType& handler);
        // 注册POST请求路由 / Register Post method router
        void Post(const std::string& url,const HandlerType& handler);
        // 注册PUT请求路由 / Register Get method router
        void Put(const std::string& url,const HandlerType& handler);
        // 注册PATCH请求路由 / Register Patch method router
        void Patch(const std::string& url,const HandlerType& handler);
        // 注册DELETE请求路由 / Register Delete method router
        void Delete(const std::string& url,const HandlerType& handler);
        // 注册TRACE请求路由 / Register Trace method router
        void Trace(const std::string& url,const HandlerType& handler);
        // 注册HEAD请求路由 / Register Head method router
        void Head(const std::string& url,const HandlerType& handler);
        // 注册OPTIONS请求路由 / Register Options method router
        void Options(const std::string& url,const HandlerType& handler);
        // 注册CONNECT请求路由 / Register Connect method router
        void Connect(const std::string& url,const HandlerType& handler);
        // 注册过滤器 / Register filter
        void Filter(const std::string &url,std::vector<HttpMethod>&& method);
    };

    // 请求信息 / Request Information
    using HttpRequest = HttpBase::HttpRequest;
    // 响应信息 / Response Information
    using HttpResponse = HttpBase::HttpResponse;

} // totoro

#endif //TOTORO_HTTPSERVER_H
