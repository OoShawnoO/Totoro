#include <dirent.h>

#include <utility>
#include <sstream>
#include "http/HttpServer.h"
#include "fmt/format.h"

namespace totoro {

    const std::string FileBrowserTemplateHtml =
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "    <meta charset=\"UTF-8\">\n"
            "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
            "    <title>文件资源列表</title>\n"
            "    <style>\n"
            "        body {\n"
            "            font-family: Arial, sans-serif;\n"
            "            background-color: #f3f3f3;\n"
            "            margin: 0;\n"
            "            padding: 0;\n"
            "        }\n"
            "        .container {\n"
            "            max-width: 800px;\n"
            "            margin: 0 auto;\n"
            "            padding: 20px;\n"
            "            background-color: #fff;\n"
            "            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\n"
            "            border-radius: 5px;\n"
            "        }\n"
            "        h1 {\n"
            "            text-align: center;\n"
            "            color: #333;\n"
            "        }\n"
            "        ul {\n"
            "            list-style-type: none;\n"
            "            padding: 0;\n"
            "        }\n"
            "        li {\n"
            "            margin-bottom: 10px;\n"
            "            padding: 10px;\n"
            "            border: 1px solid #ddd;\n"
            "            background-color: #f9f9f9;\n"
            "            border-radius: 5px;\n"
            "            overflow: hidden;\n"
            "        }\n"
            "        .folder {"
            "           background-color: #ffffcd;"
            "        }"
            "        .file{"
            "           background-color: #f0fffff;"
            "        }"
            "        .file:hover{"
            "         background-color:#e0e0e0;"
            "        }"
            "        .folder a {\n"
            "            text-decoration: none;\n"
            "            font-weight: bold;\n"
            "            color: #007BFF;\n"
            "        }\n"
            "        .folder:hover {\n"
            "            background-color: #e0e0e0;\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class=\"container\">\n"
            "        <h1>文件资源列表</h1>\n"
            "        <ul>";

    const std::string FileBrowserTemplateHtmlEnd =
            "</ul>\n"
            "    </div>\n"
            "</body>\n"
            "<script>\n"
            "    function navigateTo(url) {\n"
            "        window.location.href = url;\n"
            "    }\n"
            "</script>"
            "</html>";

    HttpServerImpl::HandlerMapType HttpServerImpl::handlerMap;
    HttpsServerImpl::HandlerMapType HttpsServerImpl::handlerMap;

    HttpServerImpl::FilterMapType HttpServerImpl::filterMap;
    HttpsServerImpl::FilterMapType HttpsServerImpl::filterMap;

    std::string HttpServerImpl::DirResourceHtml(const std::string &dirPath) {
        std::string htmlData = FileBrowserTemplateHtml;
        DIR *dir = opendir(dirPath.c_str());
        if (!dir) return {};
        std::vector<std::string> dirs;
        std::vector<std::string> files;
        dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR) dirs.emplace_back(entry->d_name);
            else files.emplace_back(entry->d_name);
        }
        sort(dirs.begin(), dirs.end());
        sort(files.begin(), files.end());
        for (const auto &name: dirs) {
            htmlData += fmt::format("<li class=\"folder\" onclick=\"navigateTo(\'{}\')\">{}</li>",
                                    name + std::string("/"), name);
        }
        for (const auto &name: files) {
            htmlData += fmt::format("<li class=\"file\" onclick=\"navigateTo(\'{}\')\">{}</li>",
                                    name, name);
        }
        closedir(dir);
        htmlData += FileBrowserTemplateHtmlEnd;
        return htmlData;
    }

    bool HttpServerImpl::FilterNode::isAllowed(HttpMethod method) const {
        return std::find(notAllowed.begin(), notAllowed.end(), method) == notAllowed.end();
    }

    void HttpServerImpl::FilterNode::addChild(const std::string &url,
                                              std::vector<HttpMethod> &method) {
        std::stringstream stream;
        if (!url.empty() && url[0] == '/') {
            stream.str(url.substr(1));
        } else {
            stream.str(url);
        }
        std::string line;
        auto filterNode = this;

        while (std::getline(stream, line, '/')) {
            if (line.empty() || line == "*") {
                filterNode->addNotAllowed(method);
            }
            auto newFilterNode = filterNode->children.find(line);
            if (newFilterNode == filterNode->children.end()) {
                newFilterNode = filterNode->children.insert({line, std::make_shared<FilterNode>()}).first;
            }
            filterNode = newFilterNode->second.get();
        }
        filterNode->addNotAllowed(method);
    }

    void HttpServerImpl::FilterNode::addNotAllowed(std::vector<HttpMethod> &method) {
        for (const auto &m: method) notAllowed.emplace_back(m);
    }


    bool HttpServerImpl::GetHandler() {
        return handler();
    }

    bool HttpServerImpl::PostHandler() {
        return handler();
    }

    bool HttpServerImpl::PutHandler() {
        return handler();
    }

    bool HttpServerImpl::PatchHandler() {
        return handler();
    }

    bool HttpServerImpl::DeleteHandler() {
        return handler();
    }

    bool HttpServerImpl::TraceHandler() {
        return handler();
    }

    bool HttpServerImpl::HeadHandler() {
        return handler();
    }

    bool HttpServerImpl::OptionsHandler() {
        return handler();
    }

    bool HttpServerImpl::ConnectHandler() {
        return handler();
    }

    void HttpServerImpl::Get(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][GET] = handler;
    }

    void HttpServerImpl::Post(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][POST] = handler;
    }

    void HttpServerImpl::Put(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][PUT] = handler;
    }

    void HttpServerImpl::Patch(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][PATCH] = handler;
    }

    void HttpServerImpl::Delete(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][DELETE] = handler;
    }

    void HttpServerImpl::Trace(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][TRACE] = handler;
    }

    void HttpServerImpl::Head(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][HEAD] = handler;
    }

    void HttpServerImpl::Options(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][OPTIONS] = handler;
    }

    void HttpServerImpl::Connect(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][CONNECT] = handler;
    }

    bool HttpServerImpl::isAllowed(const std::string &url, HttpMethod method) {
        try {
            auto filterRoot = filterMap.at(ntohs(local_address.sin_port));
            std::stringstream stream(url);
            std::string line;
            auto filterCursor = &filterRoot;
            try {
                while (std::getline(stream, line, '/')) {
                    filterCursor = filterCursor->children.at(line).get();
                }
            } catch (std::out_of_range &e) {}
            return filterCursor->isAllowed(method);
        } catch (std::out_of_range &e) {
            return true;
        }
    }

    void HttpServerImpl::Filter(unsigned short port, const std::string &url, std::vector<HttpMethod> &method) {
        auto &filterRoot = filterMap[port];
        filterRoot.addChild(url, method);
    }

    bool HttpServerImpl::handler() {
        try {
            if (!isAllowed(httpRequest.header.GetUrl().substr(1),
                           httpRequest.header.GetMethod())) {
                RenderStatus(HttpStatus::Forbidden);
                return true;
            }
            auto ret = handlerMap
                    .at(ntohs(local_address.sin_port))
                    .at(httpRequest.header.GetUrl())
                    .at(httpRequest.header.GetMethod())(httpRequest, httpResponse);
            if (ret) {
                httpResponse.header.SetStatus(HttpStatus::OK);
                if (!httpResponse.body.GetResourcePath().empty()) {
                    struct stat stat{};
                    if (::stat(httpResponse.body.GetResourcePath().c_str(), &stat) < 0) {
                        RenderStatus(HttpStatus::Not_Found);
                    }
                    httpResponse.header.SetContentLength(stat.st_size);
                } else {
                    httpResponse.header.SetContentLength(httpResponse.body.GetData().size());
                }
            }
            return ret;
        } catch (const std::out_of_range &e) {
            /* 静态资源文件处理 */
            std::string realUrl = httpRequest.header.GetUrl().substr(1);
            if (httpRequest.header.GetUrl() == "/") realUrl = ".";
            struct stat stat{};
            if (::stat(realUrl.c_str(), &stat) < 0) {
                RenderStatus(HttpStatus::Not_Found);
                return true;
            } else {
                if (!(stat.st_mode & S_IRUSR)) {
                    RenderStatus(HttpStatus::Forbidden);
                    return true;
                }
                if ((S_IFMT & stat.st_mode) == S_IFDIR) {
                    httpResponse.body.SetData(DirResourceHtml(realUrl));
                    httpResponse.header.SetStatus(HttpStatus::OK);
                    httpResponse.header.SetContentLength(httpResponse.body.GetData().size());
                    httpResponse.header.SetContentType("text/html");
                    return true;
                }
                httpResponse.header.SetStatus(HttpStatus::OK);
                httpResponse.body.SetResourcePath(realUrl);
                httpResponse.header.SetContentLength(stat.st_size);
                std::string contentType;
                try {
                    contentType = HttpContentTypeMap
                            .at(realUrl.substr(realUrl.find('.') + 1));
                } catch (const std::out_of_range &e) {
                    contentType = "text/plain";
                }
                httpResponse.header.SetContentType(contentType);
                return true;
            }
        } catch (...) {
            RenderStatus(HttpStatus::Internal_Server_Error);
            return false;
        }
    }

    HttpServer::HttpServer(const Json &config) : Acceptor(config) {}

    void HttpServer::Get(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Get(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Post(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Post(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Put(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Put(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Patch(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Patch(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Delete(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Delete(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Trace(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Trace(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Head(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Head(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Options(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Options(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Connect(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpServerImpl::Connect(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Filter(const std::string &url, std::vector<HttpMethod> &&method) {
        HttpServerImpl::Filter(port, url, method);
    }

    HttpServer::HttpServer(
            std::string ip_,
            unsigned short port_,
            int epoll_timeout,
            int reactor_count,
            bool edge_trigger,
            bool oneshot,
            bool none_block
    ) : Acceptor(
            std::move(ip_), port_, epoll_timeout,reactor_count, edge_trigger,oneshot, none_block
    ){}

    HttpsServer::HttpsServer(const Json &config) : Acceptor<Epoller<HttpsServerImpl>>(config) {

    }

    void HttpsServer::Get(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Get(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Post(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Post(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Put(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Put(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Patch(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Patch(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Delete(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Delete(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Trace(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Trace(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Head(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Head(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Options(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsServerImpl::Options(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Connect(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpServerImpl::Connect(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Filter(const std::string &url, std::vector<HttpMethod> &&method) {
        HttpsServerImpl::Filter(port, url, method);
    }

} // totoro