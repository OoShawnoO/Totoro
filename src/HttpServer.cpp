#include <dirent.h>

#include <utility>
#include <sstream>
#include "http/HttpServer.h"
#include "fmt/format.h"

namespace totoro {
/* region FileBrowserTemplateHtml */
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
/* endregion */
/* region FileBrowserTemplateHtmlEnd */
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
/* endregion */
    HttpConnection::HandlerMapType HttpConnection::handlerMap;
    HttpsConnection::HandlerMapType HttpsConnection::handlerMap;

    HttpConnection::FilterMapType HttpConnection::filterMap;
    HttpsConnection::FilterMapType HttpsConnection::filterMap;

    std::string HttpConnection::DirResourceHtml(const std::string &dirPath) {
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

    bool HttpConnection::FilterNode::IsAllowed(HttpMethod method) const {
        return std::find(notAllowed.begin(), notAllowed.end(), method) == notAllowed.end();
    }

    void HttpConnection::FilterNode::AddChild(const std::string &url,
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
                filterNode->AddNotAllow(method);
            }
            auto newFilterNode = filterNode->children.find(line);
            if (newFilterNode == filterNode->children.end()) {
                newFilterNode = filterNode->children.insert({line, std::make_shared<FilterNode>()}).first;
            }
            filterNode = newFilterNode->second.get();
        }
        filterNode->AddNotAllow(method);
    }

    void HttpConnection::FilterNode::AddNotAllow(std::vector<HttpMethod> &method) {
        for (const auto &m: method) notAllowed.emplace_back(m);
    }


    bool HttpConnection::GetHandler() {
        return handler();
    }

    bool HttpConnection::PostHandler() {
        return handler();
    }

    bool HttpConnection::PutHandler() {
        return handler();
    }

    bool HttpConnection::PatchHandler() {
        return handler();
    }

    bool HttpConnection::DeleteHandler() {
        return handler();
    }

    bool HttpConnection::TraceHandler() {
        return handler();
    }

    bool HttpConnection::HeadHandler() {
        return handler();
    }

    bool HttpConnection::OptionsHandler() {
        return handler();
    }

    bool HttpConnection::ConnectHandler() {
        return handler();
    }

    void HttpConnection::Get(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][GET] = handler;
    }

    void HttpConnection::Post(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][POST] = handler;
    }

    void HttpConnection::Put(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][PUT] = handler;
    }

    void HttpConnection::Patch(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][PATCH] = handler;
    }

    void HttpConnection::Delete(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][DELETE] = handler;
    }

    void HttpConnection::Trace(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][TRACE] = handler;
    }

    void HttpConnection::Head(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][HEAD] = handler;
    }

    void HttpConnection::Options(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][OPTIONS] = handler;
    }

    void HttpConnection::Connect(unsigned short port, const std::string &url, const HandlerType &handler) {
        handlerMap[port][url][CONNECT] = handler;
    }

    bool HttpConnection::IsAllow(const std::string &url, HttpMethod method) {
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
            return filterCursor->IsAllowed(method);
        } catch (std::out_of_range &e) {
            return true;
        }
    }

    void HttpConnection::Filter(unsigned short port, const std::string &url, std::vector<HttpMethod> &method) {
        auto &filterRoot = filterMap[port];
        filterRoot.AddChild(url, method);
    }

    bool HttpConnection::handler() {
        try {
            if (!IsAllow(request.header.GetUrl().substr(1),
                         request.header.GetMethod())) {
                RenderStatus(HttpStatus::Forbidden);
                return true;
            }
            auto ret = handlerMap
                    .at(ntohs(local_address.sin_port))
                    .at(request.header.GetUrl())
                    .at(request.header.GetMethod())(request, response);
            if (ret) {
                response.header.SetStatus(HttpStatus::OK);
                if (!response.body.GetResourcePath().empty()) {
                    struct stat stat{};
                    if (::stat(response.body.GetResourcePath().c_str(), &stat) < 0) {
                        RenderStatus(HttpStatus::Not_Found);
                    }
                    response.header.SetContentLength(stat.st_size);
                } else {
                    response.header.SetContentLength(response.body.GetData().size());
                }
            }
            return ret;
        } catch (const std::out_of_range &e) {
            /* 静态资源文件处理 */
            std::string realUrl = request.header.GetUrl().substr(1);
            if (request.header.GetUrl() == "/") realUrl = ".";
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
                    response.body.SetData(DirResourceHtml(realUrl));
                    response.header.SetStatus(HttpStatus::OK);
                    response.header.SetContentLength(response.body.GetData().size());
                    response.header.SetContentType("text/html");
                    return true;
                }
                response.header.SetStatus(HttpStatus::OK);
                response.body.SetResourcePath(realUrl);
                response.header.SetContentLength(stat.st_size);
                std::string contentType;
                try {
                    contentType = http_content_type_map
                            .at(realUrl.substr(realUrl.find('.') + 1));
                } catch (const std::out_of_range &e) {
                    contentType = "text/plain";
                }
                response.header.SetContentType(contentType);
                return true;
            }
        } catch (...) {
            RenderStatus(HttpStatus::Internal_Server_Error);
            return false;
        }
    }

    void HttpConnection::Handler() {
        
        if(!RecvUntil(request_header_text,"\r\n\r\n")) {
            MOLE_ERROR(HttpConnectionChan,"recv request header failed");
            status = Shutdown;
            return;
        }
        if(!request.header.Parse(request_header_text)) {
            MOLE_ERROR(HttpConnectionChan,"parse request header failed");
            status = Shutdown;
            return;
        }

        auto method = request.header.GetMethod();

        if(method == HttpMethod::POST || method == HttpMethod::PATCH || method == HttpMethod::PUT){
            if(Recv(request_body_text, request.header.GetContentLength()) != request.header.GetContentLength()){
                MOLE_ERROR(HttpConnectionChan,"recv request body failed");
                status = Shutdown;
                return;
            }
            if(!request.body.Parse(request_body_text, request.header)) {
                MOLE_ERROR(HttpConnectionChan,"parse request body failed");
                status = Shutdown;
                return;
            }
        }

        switch(request.header.GetMethod()) {
            case GET: {GetHandler();break;}
            case POST: {PostHandler();break;}
            case PUT: {PutHandler();break;}
            case PATCH: {PatchHandler();break;}
            case DELETE: {DeleteHandler();break;}
            case TRACE: {TraceHandler();break;}
            case HEAD: {HeadHandler();break;}
            case OPTIONS: {OptionsHandler();break;}
            case CONNECT: {ConnectHandler();break;}
        }

        if(response_header_text.empty()) {
            response_header_text = response.header.toString();
        }

        Submit(std::move(response_header_text));

        if(!response.body.GetResourcePath().empty()) {
            SubmitFile(response.body.GetResourcePath());
        }else{
            Submit(response.body.GetData());
        }


        if(request.header.GetVersion() == HttpVersion::HTTP10) {
            status = Shutdown;
        }
        Clear();
    }

    void HttpConnection::RenderStatus(HttpStatus status) {
        response.header.SetStatus(status);
        response.header.SetVersion(request.header.GetVersion());
        response.header.SetContentType("text/html");

        std::string data = http_error_template_html + fmt::format(
                "<body>\n"
                "<div class=\"error-container\">\n"
                "    <h1>{}</h1>\n"
                "    <p>{}</p>\n"
                "</div>\n"
                "</body>\n"
                "</html>", std::to_string((int32_t) status) + " " + http_status_map.find(status)->second,
                http_status_map.find(status)->second);

        response.header.SetContentLength(data.size());
        response.body.SetData(data);
    }
    
    HttpServer::HttpServer(const Json &config) : Acceptor(config) {}

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


    void HttpServer::Get(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Get(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Post(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Post(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Put(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Put(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Patch(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Patch(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Delete(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Delete(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Trace(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Trace(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Head(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Head(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Options(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Options(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Connect(const std::string &url, const HttpServer::HandlerType &handler) {
        HttpConnection::Connect(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpServer::Filter(const std::string &url, std::vector<HttpMethod> &&method) {
        HttpConnection::Filter(port, url, method);
    }

    HttpsServer::HttpsServer(const Json &config) : Acceptor<Epoller<HttpsConnection>>(config) {

    }

    void HttpsServer::Get(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Get(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Post(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Post(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Put(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Put(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Patch(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Patch(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Delete(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Delete(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Trace(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Trace(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Head(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Head(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Options(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpsConnection::Options(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Connect(const std::string &url, const HttpsServer::HandlerType &handler) {
        HttpConnection::Connect(port, url, std::forward<const HandlerType &>(handler));
    }

    void HttpsServer::Filter(const std::string &url, std::vector<HttpMethod> &&method) {
        HttpsConnection::Filter(port, url, method);
    }

    HttpsServer::HttpsServer(
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

    int HttpsConnection::Close() {
        return SSLSocket::Close();
    }

    void HttpsConnection::Handler() {
        if (!connection) {
            if (InitSSL() < 0) return;
        }
        HttpConnection::Handler();
    }
} // totoro