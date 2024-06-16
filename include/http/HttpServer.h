#ifndef TOTORO_HTTPSERVER_H
#define TOTORO_HTTPSERVER_H

#include "http/Http.h"
#include "core/Acceptor.h"
#include "utils/SSLSocket.h"

namespace totoro {

    const std::string HttpConnectionChan = "Totoro";

    /* HTTP 服务器连接 / HTTP server connection */
    class HttpConnection : public Http, public virtual Connection {
        template<typename T>
        using Ptr = std::shared_ptr<T>;

        struct FilterNode {
            std::unordered_map<std::string, Ptr<FilterNode>> children;

            std::vector<HttpMethod> notAllowed;

            bool IsAllowed(HttpMethod method) const;

            void AddChild(const std::string &url, std::vector<HttpMethod> &method);

            void AddNotAllow(std::vector<HttpMethod> &method);
        };

        bool IsAllow(const std::string &url, HttpMethod method);

        bool handler();

    public:
        using HandlerType = std::function<bool(const HttpRequest &, HttpResponse &)>;
        using HandlerMapType = std::unordered_map<unsigned short, std::unordered_map<std::string, std::unordered_map<HttpMethod, HandlerType>>>;
        using FilterMapType = std::unordered_map<unsigned short, FilterNode>;

        // 注册GET请求路由 / Register Get method router
        static void Get(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册POST请求路由 / Register Post method router
        static void Post(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册PUT请求路由 / Register Get method router
        static void Put(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册PATCH请求路由 / Register Patch method router
        static void Patch(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册DELETE请求路由 / Register Delete method router
        static void Delete(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册TRACE请求路由 / Register Trace method router
        static void Trace(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册HEAD请求路由 / Register Head method router
        static void Head(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册OPTIONS请求路由 / Register Options method router
        static void Options(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册CONNECT请求路由 / Register Connect method router
        static void Connect(unsigned short port, const std::string &url, const HandlerType &handler);

        // 注册过滤器 / Register filter
        static void Filter(unsigned short port, const std::string &url, std::vector<HttpMethod> &method);

    private:
        static HandlerMapType handlerMap;
        static FilterMapType filterMap;

        static std::string DirResourceHtml(const std::string &dirPath);

    protected:
        void Handler() override;

        void RenderStatus(HttpStatus status);

        bool GetHandler();

        bool PostHandler();

        bool PutHandler();

        bool PatchHandler();

        bool DeleteHandler();

        bool TraceHandler();

        bool HeadHandler();

        bool OptionsHandler();

        bool ConnectHandler();
    };

    /* HTTPS 服务器连接 / HTTPS server connection */
    class HttpsConnection : public HttpConnection, public SSLSocket {
        using HandlerType = std::function<bool(const HttpRequest &, HttpResponse &)>;
        using HandlerMapType = std::unordered_map<unsigned short, std::unordered_map<std::string, std::unordered_map<HttpMethod, HandlerType>>>;
        static HandlerMapType handlerMap;
        static FilterMapType filterMap;
    public:
        void Handler() override;
        int Close() final;
    };

    /* HTTP 路由服务器 / HTTP Router Server */
    class HttpServer : public Acceptor<Epoller<HttpConnection>> {
    public:
        using HandlerType = std::function<bool(const Http::HttpRequest &, Http::HttpResponse &)>;

        explicit HttpServer(const Json &config);

        HttpServer(
            std::string ip_,
            unsigned short port_,
            int epoll_timeout = -1,
            int reactor_count = 1,
            bool edge_trigger = false,
            bool oneshot = true,
            bool none_block = true
       );

        // 注册GET请求路由 / Register Get method router
        void Get(const std::string &url, const HandlerType &handler);

        // 注册POST请求路由 / Register Post method router
        void Post(const std::string &url, const HandlerType &handler);

        // 注册PUT请求路由 / Register Get method router
        void Put(const std::string &url, const HandlerType &handler);

        // 注册PATCH请求路由 / Register Patch method router
        void Patch(const std::string &url, const HandlerType &handler);

        // 注册DELETE请求路由 / Register Delete method router
        void Delete(const std::string &url, const HandlerType &handler);

        // 注册TRACE请求路由 / Register Trace method router
        void Trace(const std::string &url, const HandlerType &handler);

        // 注册HEAD请求路由 / Register Head method router
        void Head(const std::string &url, const HandlerType &handler);

        // 注册OPTIONS请求路由 / Register Options method router
        void Options(const std::string &url, const HandlerType &handler);

        // 注册CONNECT请求路由 / Register Connect method router
        void Connect(const std::string &url, const HandlerType &handler);

        // 注册过滤器 / Register filter
        void Filter(const std::string &url, std::vector<HttpMethod> &&method);
    };

    /* HTTPS 路由服务器 / HTTPS Router Server */
    class HttpsServer : public Acceptor<Epoller<HttpsConnection>> {
        using HandlerType = std::function<bool(const Http::HttpRequest &, Http::HttpResponse &)>;
    public:
        HttpsServer(const Json &config);

        HttpsServer(
            std::string ip_,
            unsigned short port_,
            int epoll_timeout = -1,
            int reactor_count = 1,
            bool edge_trigger = false,
            bool oneshot = true,
            bool none_block = true
        );

        // 注册GET请求路由 / Register Get method router
        void Get(const std::string &url, const HandlerType &handler);

        // 注册POST请求路由 / Register Post method router
        void Post(const std::string &url, const HandlerType &handler);

        // 注册PUT请求路由 / Register Get method router
        void Put(const std::string &url, const HandlerType &handler);

        // 注册PATCH请求路由 / Register Patch method router
        void Patch(const std::string &url, const HandlerType &handler);

        // 注册DELETE请求路由 / Register Delete method router
        void Delete(const std::string &url, const HandlerType &handler);

        // 注册TRACE请求路由 / Register Trace method router
        void Trace(const std::string &url, const HandlerType &handler);

        // 注册HEAD请求路由 / Register Head method router
        void Head(const std::string &url, const HandlerType &handler);

        // 注册OPTIONS请求路由 / Register Options method router
        void Options(const std::string &url, const HandlerType &handler);

        // 注册CONNECT请求路由 / Register Connect method router
        void Connect(const std::string &url, const HandlerType &handler);

        // 注册过滤器 / Register filter
        void Filter(const std::string &url, std::vector<HttpMethod> &&method);
    };

} // totoro

#endif //TOTORO_HTTPSERVER_H
