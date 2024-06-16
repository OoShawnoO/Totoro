#include "http/Http.h"

#include <utility>
#include <sstream>
#include <regex>
#include "fmt/format.h"
#include "fmt/ranges.h"

static const std::string HttpBaseChan = "Totoro";
namespace totoro {
    /* region Http Method Map */
    const std::unordered_map<HttpMethod, std::string> HttpMethodMap{
            {GET,     "GET"},
            {POST,    "POST"},
            {PUT,     "PUT"},
            {PATCH,   "PATCH"},
            {DELETE,  "DELETE"},
            {HEAD,    "HEAD"},
            {TRACE,   "TRACE"},
            {OPTIONS, "OPTIONS"},
            {CONNECT, "CONNECT"}
    };
    const std::unordered_map<std::string, HttpMethod> ReverseHttpMethodMap{
            {"GET",     GET},
            {"POST",    POST},
            {"PUT",     PUT},
            {"PATCH",   PATCH},
            {"DELETE",  DELETE},
            {"TRACE",   TRACE},
            {"HEAD",    HEAD},
            {"OPTIONS", OPTIONS},
            {"CONNECT", CONNECT},
    };
    /* endregion */
    /* region Http Status Map */
    const std::unordered_map<HttpStatus, std::string> http_status_map{
            {HttpStatus::Continue,                        "Continue"},
            {HttpStatus::Switching_Protocols,             "Switching Protocols"},
            {HttpStatus::Processing,                      "Processing"},
            {HttpStatus::OK,                              "OK"},
            {HttpStatus::Created,                         "Created"},
            {HttpStatus::Accepted,                        "Accepted"},
            {HttpStatus::Non_Authoritative_information,   "Non-Authoritative Information"},
            {HttpStatus::No_Content,                      "No Content"},
            {HttpStatus::Reset_Content,                   "Reset Content"},
            {HttpStatus::Partial_Content,                 "Partial Content"},
            {HttpStatus::Multi_Status,                    "Multi-Status"},
            {HttpStatus::Multiple_Choice,                 "Multiple Choices"},
            {HttpStatus::Moved_Permanently,               "Moved Permanently"},
            {HttpStatus::Move_Temporarily,                "Moved Temporarily"},
            {HttpStatus::See_Other,                       "See Other"},
            {HttpStatus::Not_Modified,                    "Not Modified"},
            {HttpStatus::Use_Proxy,                       "Use Proxy"},
            {HttpStatus::Switch_Proxy,                    "Switch Proxy"},
            {HttpStatus::Temporary_Redirect,              "Temporary Redirect"},
            {HttpStatus::Bad_Request,                     "Bad Request"},
            {HttpStatus::Unauthorized,                    "Unauthorized"},
            {HttpStatus::Payment_Required,                "Payment Required"},
            {HttpStatus::Forbidden,                       "Forbidden"},
            {HttpStatus::Not_Found,                       "Not Found"},
            {HttpStatus::Method_Not_Allowed,              "Method Not Allowed"},
            {HttpStatus::Not_Acceptable,                  "Not Acceptable"},
            {HttpStatus::Proxy_Authentication_Required,   "Proxy Authentication Required"},
            {HttpStatus::Request_Timeout,                 "Request Timeout"},
            {HttpStatus::Conflict,                        "Conflict"},
            {HttpStatus::Gone,                            "Gone"},
            {HttpStatus::Length_Required,                 "Length Required"},
            {HttpStatus::Precondition_Failed,             "Precondition Failed"},
            {HttpStatus::Request_Entity_Too_Large,        "Request Entity Too Large"},
            {HttpStatus::Request_URI_Too_Long,            "Request-URI Too Long"},
            {HttpStatus::Unsupported_Media_Type,          "Unsupported Media Type"},
            {HttpStatus::Requested_Range_Not_Satisfiable, "Requested Range Not Satisfiable"},
            {HttpStatus::Expectation_Failed,              "Expectation Failed"},
            {HttpStatus::I_Am_A_Teapot,                   "I'm a teapot"},
            {HttpStatus::Misdirected_Request,             "Misdirected Request"},
            {HttpStatus::Unprocessable_Entity,            "Unprocessable Entity"},
            {HttpStatus::Locked,                          "Locked"},
            {HttpStatus::Failed_Dependency,               "Failed Dependency"},
            {HttpStatus::Too_Early,                       "Too Early"},
            {HttpStatus::Upgrade_Required,                "Upgrade Required"},
            {HttpStatus::Retry_With,                      "Retry With"},
            {HttpStatus::Unavailable_For_Legal_Reasons,   "Unavailable For Legal Reasons"},
            {HttpStatus::Internal_Server_Error,           "Internal Server Error"}
    };
    /* endregion */
    /* region Http Version Map */
    std::unordered_map<HttpVersion, std::string> http_version_map{
            {HTTP11, "HTTP/1.1"},
            {HTTP10, "HTTP/1.0"},
            {HTTP20, "HTTP/2.0"},
            {HTTP30, "HTTP/3.0"}
    };
    std::unordered_map<std::string, HttpVersion> reverse_http_version_map{
            {"HTTP/1.1", HTTP11},
            {"HTTP/1.0", HTTP10},
            {"HTTP/2.0", HTTP20},
            {"HTTP/3.0", HTTP30}
    };
    /* endregion */
    /* region Http Content Type Map */
    std::unordered_map<std::string, std::string> http_content_type_map
            {
                    {"html",    "text/html"},
                    {"htm",     "text/html"},
                    {"shtml",   "text/html"},
                    {"css",     "text/css"},
                    {"xml",     "text/xml"},
                    {"gif",     "image/gif"},
                    {"jpeg",    "image/jpeg"},
                    {"jpg",     "image/jpeg"},
                    {"js",      "application/javascript"},
                    {"atom",    "application/atom+xml"},
                    {"rss",     "application/rss+xml"},
                    {"mml",     "text/mathml"},
                    {"txt",     "text/plain"},
                    {"jad",     "text/vnd.sun.j2me.app-descriptor"},
                    {"wml",     "text/vnd.wap.wml"},
                    {"htc",     "text/x-component"},
                    {"png",     "image/png"},
                    {"tif",     "image/tiff"},
                    {"tiff",    "image/tiff"},
                    {"wbmp",    "image/vnd.wap.wbmp"},
                    {"ico",     "image/x-icon"},
                    {"jng",     "image/x-jng"},
                    {"bmp",     "image/x-ms-bmp"},
                    {"svg",     "image/svg+xml"},
                    {"svgz",    "image/svg+xml"},
                    {"webp",    "image/webp"},
                    {"woff",    "application/font-woff"},
                    {"woff2",   "application/font-woff"},
                    {"jar",     "application/java-archive"},
                    {"war",     "application/java-archive"},
                    {"ear",     "application/java-archive"},
                    {"json",    "application/json"},
                    {"hqx",     "application/mac-binhex40"},
                    {"doc",     "application/msword"},
                    {"pdf",     "application/pdf"},
                    {"ps",      "application/postscript"},
                    {"eps",     "application/postscript"},
                    {"ai",      "application/postscript"},
                    {"rtf",     "application/rtf"},
                    {"m3u8",    "application/vnd.apple.mpegurl"},
                    {"kml",     "application/vnd.google-earth.kml+xml"},
                    {"kmz",     "application/vnd.google-earth.kmz"},
                    {"xls",     "application/vnd.ms-excel"},
                    {"eot",     "application/vnd.ms-fontobject"},
                    {"ppt",     "application/vnd.ms-powerpoint"},
                    {"odg",     "application/vnd.oasis.opendocument.graphics"},
                    {"odp",     "application/vnd.oasis.opendocument.presentation"},
                    {"ods",     "application/vnd.oasis.opendocument.spreadsheet"},
                    {"odt",     "application/vnd.oasis.opendocument.text"},
                    {"pptx",    "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
                    {"xlsx",    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
                    {"docx",    "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
                    {"wmlc",    "application/vnd.wap.wmlc"},
                    {"7z",      "application/x-7z-compressed"},
                    {"cco",     "application/x-cocoa"},
                    {"jardiff", "application/x-java-archive-diff"},
                    {"jnlp",    "application/x-java-jnlp-file"},
                    {"run",     "application/x-makeself"},
                    {"pl",      "application/x-perl"},
                    {"pm",      "application/x-perl"},
                    {"prc",     "application/x-pilot"},
                    {"pdb",     "application/x-pilot"},
                    {"rar",     "application/x-rar-compressed"},
                    {"rpm",     "application/x-redhat-package-manager"},
                    {"sea",     "application/x-sea"},
                    {"sit",     "application/x-stuffit"},
                    {"tcl",     "application/x-tcl"},
                    {"tk",      "application/x-tcl"},
                    {"der",     "application/x-x509-ca-cert"},
                    {"pem",     "application/x-x509-ca-cert"},
                    {"crt",     "application/x-x509-ca-cert"},
                    {"xpi",     "application/x-xpinstall"},
                    {"xhtml",   "application/xhtml+xml"},
                    {"xspf",    "application/xspf+xml"},
                    {"zip",     "application/zip"},
                    {"bin",     "application/octet-stream"},
                    {"exe",     "application/octet-stream"},
                    {"dll",     "application/octet-stream"},
                    {"deb",     "application/octet-stream"},
                    {"dmg",     "application/octet-stream"},
                    {"iso",     "application/octet-stream"},
                    {"img",     "application/octet-stream"},
                    {"msi",     "application/octet-stream"},
                    {"msp",     "application/octet-stream"},
                    {"msm",     "application/octet-stream"},
                    {"mid",     "audio/midi"},
                    {"midi",    "audio/midi"},
                    {"kar",     "audio/midi"},
                    {"mp3",     "audio/mpeg"},
                    {"ogg",     "audio/ogg"},
                    {"m4a",     "audio/x-m4a"},
                    {"ra",      "audio/x-realaudio"},
                    {"3gpp",    "video/3gpp"},
                    {"3gp",     "video/3gpp"},
                    {"ts",      "video/mp2t"},
                    {"mp4",     "video/mp4"},
                    {"mpeg",    "video/mpeg"},
                    {"mpg",     "video/mpeg"},
                    {"mov",     "video/quicktime"},
                    {"webm",    "video/webm"},
                    {"flv",     "video/x-flv"},
                    {"m4v",     "video/x-m4v"},
                    {"mng",     "video/x-mng"},
                    {"asx",     "video/x-ms-asf"},
                    {"asf",     "video/x-ms-asf"},
                    {"wmv",     "video/x-ms-wmv"},
                    {"avi",     "video/x-msvideo"},
            };
    /* endregion */
    /* region Http Error Template Html */
    const std::string http_error_template_html = "<!DOCTYPE html>\n"
                                              "<html lang=\"en\">\n"
                                              "<head>\n"
                                              "    <meta charset=\"UTF-8\">\n"
                                              "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                                              "    <title>Totoro Server Notice</title>\n"
                                              "    <style>\n"
                                              "        body {\n"
                                              "            font-family: Arial, sans-serif;\n"
                                              "            background-color: #f0f0f0;\n"
                                              "            margin: 0;\n"
                                              "            padding: 0;\n"
                                              "            text-align: center;\n"
                                              "        }\n"
                                              "        .error-container {\n"
                                              "            background-color: #fff;\n"
                                              "            border-radius: 5px;\n"
                                              "            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);\n"
                                              "            margin: 100px auto;\n"
                                              "            max-width: 400px;\n"
                                              "            padding: 20px;\n"
                                              "        }\n"
                                              "        h1 {\n"
                                              "            color: #e74c3c;\n"
                                              "        }\n"
                                              "        p {\n"
                                              "            color: #333;\n"
                                              "        }\n"
                                              "    </style>\n"
                                              "</head>\n";
    /* endregion */
    static const std::regex RequestLineRegex("^(.*)\\s(.*)\\s(.*)$");
    static const std::regex RequestParameterRegex("([^&]+)=([^&]*)&?");
    static const std::regex RequestFieldRegex("^(.*):\\s?(.*);?$");

    static auto getLine = [](std::stringstream &_stream, std::string &_line)
            -> std::basic_istream<char> & {
        auto &ret = std::getline(_stream, _line, '\r');
        _stream.ignore(1);
        return ret;
    };

    void Http::Clear() {
        request.header.Clear();
        request.body.Clear();
        response.header.Clear();
        response.body.Clear();
        request_header_text.clear();
        request_body_text.clear();
        response_header_text.clear();
        response_body_text.clear();
    }

    /* RequestHeader Impl */
    void Http::RequestHeader::Clear() {
        url.clear();
        boundary.clear();
        fields.clear();
        cookies.clear();
    }

    void Http::RequestHeader::parseParameters(std::string &&parameterText) {
        std::smatch match;
        auto pos = parameterText.cbegin();
        auto end = parameterText.cend();
        for (; std::regex_search(pos, end, match, RequestParameterRegex); pos = match.suffix().first) {
            parameters[match[1]] = match[2];
        }
    }

    bool Http::RequestHeader::Parse(std::string &requestHeaderData) {
        if (requestHeaderData.empty()) {
            MOLE_ERROR(HttpBaseChan, "request header empty");
            return false;
        }
        std::stringstream stream(requestHeaderData);
        std::string line;

        if (!getLine(stream, line)) {
            MOLE_ERROR(HttpBaseChan, "request header content can't get line");
            return false;
        }

        std::cmatch matches;
        if (!std::regex_match(line.c_str(), matches, RequestLineRegex)) {
            MOLE_ERROR(HttpBaseChan, "match first line failed");
            return false;
        }
        try {
            url = matches[2];
            method = ReverseHttpMethodMap.at(matches[1]);
            version = reverse_http_version_map.at(matches[3]);
        } catch (...) {
            MOLE_ERROR(HttpBaseChan, fmt::format("map not found {} or {}", matches[1].str(), matches[3].str()));
            return false;
        }

        auto pos = url.find('?');
        if (pos != std::string::npos) {
            parseParameters(url.substr(pos + 1));
            url.erase(pos);
        }

        while (getLine(stream, line)) {
            if (line.empty()) break;
            auto splitPos = line.find(':');
            if (splitPos == std::string::npos) {
                MOLE_ERROR(HttpBaseChan, "field has no :");
                return false;
            }
            std::stringstream fieldStream(line.substr(splitPos + 1));
            auto &field = fields[line.substr(0, splitPos)];
            while (std::getline(fieldStream, line, ',')) {
                line.erase(line.begin(), std::find_if_not(line.begin(), line.end(), ::isspace));
                field.emplace_back(std::move(line));
            }
        }

        for (const auto &cookie: fields["Cookie"]) {
            pos = cookie.find('=');
            cookies[cookie.substr(0, pos - 1)] = cookie.substr(pos + 1);
        }

        if (method != POST && method != PATCH && method != PUT) return true;
        getline(stream, requestHeaderData, '\0');

        auto &CT = fields["Content-Type"];
        if (CT.empty()) {
            MOLE_ERROR(HttpBaseChan, "not found content type");
            return -1;
        }
        auto splitPos = CT[0].find(';');
        if (splitPos != std::string::npos) {
            CT.emplace_back(CT[0].substr(splitPos + 1));
            CT[0] = CT[0].substr(0, splitPos);
        }
        if (CT[0] == "multipart/files-data" && method == POST || method == PATCH || method == PUT) {
            auto it = std::find_if(CT.begin() + 1, CT.end(), [](const std::string &field) {
                return field.find("boundary") != std::string::npos;
            });
            if (it == CT.end()) return false;
            pos = it->find('=');
            boundary = it->substr(pos + 1);
        }

        return true;
    }

    const HttpContentDataType &Http::RequestHeader::GetContentType() const {
        return fields.find("Content-Type")->second[0];
    }

    size_t Http::RequestHeader::GetContentLength() const {
        return stoul(fields.find("Content-Length")->second[0]);
    }

    const HttpCookieType &Http::RequestHeader::GetCookies() const { return cookies; }

    const HttpVersion &Http::RequestHeader::GetVersion() const { return version; }

    const std::string &Http::RequestHeader::GetUrl() const { return url; }

    const HttpHeaderFieldsType &Http::RequestHeader::GetFields() const { return fields; }

    const HttpParameterType &Http::RequestHeader::GetParameters() const { return parameters; }

    const std::string &Http::RequestHeader::GetBoundary() const { return boundary; }

    const HttpMethod &Http::RequestHeader::GetMethod() const { return method; }

    void Http::RequestHeader::SetMethod(HttpMethod _method) { method = _method; }

    void Http::RequestHeader::SetContentType(const std::string &contentType) {
        fields["Content-Type"] = {contentType};
    }

    void Http::RequestHeader::SetContentLength(size_t size) { fields["Content-Length"] = {std::to_string(size)}; }

    void Http::RequestHeader::SetCookie(const std::string &key, const std::string &value) {
        fields["Cookies"].emplace_back(fmt::format("{}={};", key, value));
    }

    void Http::RequestHeader::SetVersion(HttpVersion _version) { version = _version; }

    void Http::RequestHeader::SetUrl(const std::string &_url) { url = _url; }

    void Http::RequestHeader::SetParameters(const std::string &key, const std::string &value) {
        parameters.insert({key, value});
    }

    void Http::RequestHeader::SetField(const std::string &key,
                                       const std::vector<std::string> &values) { fields[key] = values; }

    void Http::RequestHeader::SetBoundary(const std::string &_boundary) { boundary = _boundary; }

    std::string Http::RequestHeader::toString() {
        std::string requestHeaderData;
        if (!parameters.empty()) {
            bool isFirst = true;
            for (const auto &parameter: parameters) {
                if (isFirst) {
                    isFirst = false;
                    url += '?';
                }
                else url += '&';
                url += fmt::format("{}={}", parameter.first, parameter.second);
            }
        }
        if (!boundary.empty()) {
            fields["Content-Type"].emplace_back("boundary=" + boundary);
        }
        requestHeaderData += fmt::format("{} {} {}\r\n", HttpMethodMap.at(method), url, http_version_map.at(version));
        for (const auto &field: fields) {
            requestHeaderData += fmt::format("{}:{}\r\n", field.first,
                                             fmt::join(field.second.begin(), field.second.end(), ","));
        }
        requestHeaderData += "\r\n";
        return requestHeaderData;
    }

    /* RequestBody Impl */
    void Http::RequestBody::Clear() {
        files.clear();
    }

    bool Http::RequestBody::Parse(const std::string &requestBodyData, const RequestHeader &header) {
        if (requestBodyData.empty()) {
            MOLE_ERROR(HttpBaseChan, "request body empty");
            return false;
        }
        std::stringstream stream(requestBodyData);
        std::string line;
        std::string fileName;
        std::string data;
        std::string name;

        const std::string &type = header.GetContentType();
        if (type == "multipart/form-data") {
            const std::string &boundary = header.GetBoundary();
            const std::string another = "--" + boundary;
            const std::string end = "--" + boundary + "--";
            std::string contentType;
            while (getLine(stream, line)) {
                if (line == end) {
                    files[name] = {std::move(contentType), std::move(fileName), std::move(data)};
                    break;
                }
                if (line == another) {
                    if (!name.empty())
                        files[name] = {std::move(contentType), std::move(fileName), std::move(data)};
                    continue;
                }
                if (line.empty()) {
                    auto pos = static_cast<size_t>(stream.tellg());
                    auto endPos = requestBodyData.find(another, pos);
                    data = requestBodyData.substr(pos, endPos - pos);
                    stream.seekg(static_cast<std::streampos>((std::streamoff) endPos));
                    data.pop_back();
                    data.pop_back();
                    continue;
                }
                if (line.substr(0, 12) == "Content-Type") {
                    contentType = line.substr(line.find(':') + 1);
                    contentType.erase(contentType.begin(),
                                      std::find_if_not(contentType.begin(), contentType.end(), ::isspace));
                    contentType.erase((std::find_if_not(contentType.rbegin(), contentType.rend(),
                                                        ::isspace).base(), contentType.end()));
                    continue;
                }
                if (line.substr(0, 19) == "Content-Disposition") {
                    auto namePos = line.find("name=");
                    auto filePos = line.find("filename=");
                    if (namePos == std::string::npos) {
                        MOLE_ERROR(HttpBaseChan, "request body multipart-files has no name field");
                        return false;
                    }
                    auto splitPos = line.find(';', namePos + 6);
                    name = line.substr(namePos + 6, line.find(';', namePos + 6) - namePos - 7);
                    if (splitPos == std::string::npos) name.pop_back();
                    if (filePos == std::string::npos) continue;
                    splitPos = line.find(';', filePos + 10);
                    fileName = line.substr(filePos + 10, splitPos - filePos - 11);
                    if (splitPos == std::string::npos) fileName.pop_back();
                }
            }
        } else if (type == "application/x-www-form-urlencoded") {
            while (getline(stream, line, '&')) {
                auto equalPos = line.find('=');
                if (equalPos == std::string::npos) {
                    MOLE_ERROR(HttpBaseChan, "char '=' not found");
                    return false;
                }
                name = line.substr(0, equalPos);
                data = line.substr(equalPos + 1);
                form[name] = std::move(data);
            }
        } else if (type == "application/json") {
            json = Json::parse(requestBodyData);
            if (json.empty()) return false;
        } else {
            return false;
        }
        return true;
    }

    const HttpMultiPartType &Http::RequestBody::GetFiles() const { return files; }

    bool Http::RequestBody::DownloadFilesField(const std::string &fieldName,
                                               const std::string &destFilePath,
                                               std::string fileName) const {
        auto file = files.find(fieldName);
        if (file == files.end()) return false;
        if (file->second.file_name.empty()) return false;
        if (fileName.empty()) fileName = file->second.file_name;
        std::ofstream out(destFilePath + "/" + fileName, std::ios::out);
        if (!out.is_open()) return false;
        out.write(file->second.data.c_str(), (std::streamsize) file->second.data.size());
        out.close();
        return true;
    }

    std::string Http::RequestBody::GetFilesFieldData(const std::string &name) {
        auto field = files.find(name);
        if (field == files.end()) return {};
        return field->second.data;
    }

    std::string Http::RequestBody::GetFilesFieldFileName(const std::string &name) {
        auto field = files.find(name);
        if (field == files.end()) return {};
        return field->second.file_name;
    }

    std::string Http::RequestBody::GetFilesFieldContentType(const std::string &name) {
        auto field = files.find(name);
        if (field == files.end()) return {};
        return field->second.content_type;
    }

    const Json &Http::RequestBody::GetJson() const { return json; }

    const HttpFormType &Http::RequestBody::GetForm() const { return form; }

    std::string Http::RequestBody::GetFormField(const std::string &name) {
        auto it = form.find(name);
        if (it == form.end()) return {};
        return it->second;
    }

    void Http::RequestBody::SetJson(const Json &_json) { json = _json; }

    void Http::RequestBody::SetForm(const HttpFormType &_form) { form = _form; }

    void Http::RequestBody::SetFormField(const std::string &key, const std::string &value) { form[key] = value; }

    void Http::RequestBody::SetFilesFieldData(const std::string &name,
                                              const HttpMultiPartDetail &detail) { files[name] = detail; }

    std::string Http::RequestBody::toString(const RequestHeader &header) const {
        std::string requestBodyData;
        const auto &CT = header.GetContentType();
        if (CT == "multipart/form-data") {
            const std::string &boundary = header.GetBoundary();
            std::string another = "--" + boundary;
            std::string end = "--" + boundary + "--";
            for (const auto &file: files) {
                requestBodyData += another;
                requestBodyData += fmt::format("Content-Disposition:name={};", file.first);
                if (!file.second.file_name.empty()) requestBodyData += "filename=" + file.second.file_name;
                requestBodyData += "\r\n";
                if (!file.second.content_type.empty()) {
                    requestBodyData += "Content-Type:" + file.second.content_type + "\r\n";
                }
                requestBodyData += "\r\n";
                requestBodyData += file.second.data;
            }
            requestBodyData += end + "\r\n";
        } else if (CT == "application/x-www-form-urlencoded") {
            bool isFirst = true;
            for (const auto &formItem: form) {
                if (isFirst) { isFirst = false; }
                else requestBodyData += '&';
                requestBodyData += fmt::format("{}={}", formItem.first, formItem.second);
            }
        } else if (CT == "application/json") {
            requestBodyData += nlohmann::to_string(json);
        }
        return requestBodyData;
    }
    /*
     * "HTTP/1.1 200 OK\r\n
     * Connection: keep-alive\r\n
     * Content-Encoding: gzip\r\n
     * Content-Security-Policy: frame-ancestors 'self' https://chat.baidu.com http://mirror-chat.baidu.com https://fj-chat.baidu.com https://hba-chat.baidu.com https://hbe-chat.baidu.com https://njjs-chat.baidu.com https://nj-chat.baidu.com https://hna-chat.baidu.com https://hnb-chat.baidu.com http://debug.baidu-int.com;\r\n
     * Content-Type: text/html; charset=utf-8\r\n
     * Date: Fri, 15 Sep 2023 07:01:10 GMT\r\n
     * Server: BWS/1.1\r\n
     * Traceid: 1694761270237424871414672100593762323040\r\n
     * X-Ua-Compatible: IE=Edge,chrome=1\r\n
     * Transfer-Encoding: chunked\r\n
     * \r\n
     * daa\r\n\
     * 037\213\b\000\000\000\000\000\000\003\324\\{\217\033\327u\377\177?ŘD\260R:\303\345\360\265\\R\253T\221-\304(\202\024\260\003$\200\001b\036wȁ\206\034ff\270\017\021\v\330I]'m\355\3040\222\242\265\2216\001R\264@k;\250\333\030vl\177\230hW\322_\375\n=\347\334{g\356\235\031\356R+\365\021ђw\207\367y\356y\376ιs\353\205\027\277s\367\325\357\377\371K\306,\233G\267o\275`Y\257\274z\347\325\357\276b|\347\317,\353\366-\376t\306\034\377\366\2559\313\034h\226--\366\203Uxtظ\033/2\266ȬWO\227\254ax\374\267\303F\306N\262=\3548\366fN\222\262\354p\225\005ְQ3\302\367\254\357ޱ\356\306\363\245\223\205n\244\016\362\362K\207̟2ӛ%\361\234\035ڲw>\213\023\035;\247i\303X8\360u#a\001K\022\226\310f\374i6csfyq\024'\312\372\232\001\375ћ\372,\365\222p\231\205\361Biz\376\346??\372ُ\236\374\372/\317\337\374\361\243\277\377\213\207\237\376\333\305/\336\272\370\331\a\217>\371\365\371\357\177~\361\336;\177x\375\215\307o}r\376W\357?\374\354\235\307\037\376ˣ/\336\275\370\370\335\213\367?y\370\345W\027o\377\356\374\203\217\037\277\363\273\363\237\376\342\341W\277\272x\343\243\377\372\375\337\\\374\344\313\363\037\177|\361\223\327/~\373\303?\274\376\303G\177\367\345\371g\377\364\370?\337|\374\325[\347o\377\350\341g_\345\263\300PO~\365\037\027?\377\370\342\355\017\317?{\017\372\236\377\364\243\207\237\377\346\321/\377\365\311\337~\302\307y\364\376\247\347o\376;t\341Kz\364\371{\027\277\374\000\206\205\255E\341⾑\260谑\316\342$\363V\231\021\002\355\032\306\fHu\330\300cLG{{\307\307\307-\327\t\375Uˋ\347{\201s\204\215Z\360O\303\310\340T\017\033\341ܙ\262\275\023\213w\336\323\006fN\342\315dCg\271\214B\017\3161^\354\305K\266H\351[\205\254\177r2\217\344\364", <incomplete sequence \342>
     */
    /* ResponseHeader Impl */
    bool Http::ResponseHeader::Parse(std::string &responseHeaderData) {
        if (responseHeaderData.empty()) {
            MOLE_ERROR(HttpBaseChan, "request header empty");
            return false;
        }
        std::stringstream stream(responseHeaderData);
        std::string line;

        if (!getLine(stream, line)) {
            MOLE_ERROR(HttpBaseChan, "request header content can't get line");
            return false;
        }
        auto pos = line.find(' ');
        if (pos == std::string::npos) {
            MOLE_ERROR(HttpBaseChan, "parse response line failed");
            return false;
        }
        auto secondPos = line.find(' ', pos + 1);
        if (secondPos == std::string::npos) {
            MOLE_ERROR(HttpBaseChan, "parse response line failed");
            return false;
        }
        status = (HttpStatus) stoi(line.substr(pos + 1, secondPos - pos - 1));
        version = reverse_http_version_map.at(line.substr(0, pos));

        while (getLine(stream, line)) {
            if (line.empty()) break;
            auto splitPos = line.find(':');
            if (splitPos == std::string::npos) {
                MOLE_ERROR(HttpBaseChan, "field has no :");
                return false;
            }
            std::stringstream fieldStream(line.substr(splitPos + 1));
            auto &field = fields[line.substr(0, splitPos)];
            while (std::getline(fieldStream, line, ',')) {
                line.erase(line.begin(), std::find_if_not(line.begin(), line.end(), ::isspace));
                field.emplace_back(std::move(line));
            }
        }
        pos = static_cast<size_t>(stream.tellg());
        responseHeaderData.erase(responseHeaderData.begin(), responseHeaderData.begin() + pos);

        return true;
    }


    std::string Http::ResponseHeader::toString() {
        std::string responseBodyText = fmt::format("{} {} {}\r\n",
                                                   http_version_map.at(version),
                                                   std::to_string((uint32_t) status),
                                                   http_status_map.at(status));
        for (auto &field: fields) {
            std::string values;
            if (field.second.size() == 1) values = fmt::format("{}", field.second[0]);
            else if (field.second.empty()) values = "";
            else {
                for (auto &value: field.second) {
                    values += fmt::format("{},", std::move(value));
                }
            }
            responseBodyText += fmt::format("{}:{}\r\n", field.first, std::move(values));
        }
        responseBodyText += "\r\n";
        return responseBodyText;
    }

    void Http::ResponseHeader::Clear() { fields.clear(); }

    void Http::ResponseHeader::SetVersion(HttpVersion _version) { version = _version; }

    void Http::ResponseHeader::SetStatus(HttpStatus _status) { status = _status; }

    void Http::ResponseHeader::SetContentType(const std::string &contentType) {
        auto &field = fields["Content-Type"];
        if (field.empty())
            field.emplace_back(contentType);
        else
            field[0] = contentType;
    }

    void Http::ResponseHeader::SetContentLength(size_t length) {
        auto &field = fields["Content-Length"];
        if (field.empty())
            field.emplace_back(std::to_string(length));
        else
            field[0] = std::to_string(length);
    }

    void Http::ResponseHeader::SetCookie(const std::string &cookieKey,
                                         const std::string &cookieValue) {
        auto &field = fields["Set-Cookie"];
        field.emplace_back(fmt::format("{}={}", cookieKey, cookieValue));
    }

    void Http::ResponseHeader::SetField(const std::string &fieldKey,
                                        const std::vector<std::string> &fieldValues) {
        fields[fieldKey] = fieldValues;
    }

    const HttpContentDataType &Http::ResponseHeader::GetContentType() const {
        return fields.find("Content-Type")->second[0];
    }

    size_t Http::ResponseHeader::GetContentLength() const {
        auto field = fields.begin();
        if ((field = fields.find("Content-Length")) == fields.end()) return 0;
        return stoll(field->second[0]);
    }

    std::string Http::ResponseHeader::GetTransferEncoding() const {
        auto field = fields.begin();
        if ((field = fields.find("Transfer-Encoding")) == fields.end()) return {};
        return field->second[0];
    }

    HttpVersion Http::ResponseHeader::GetVersion() const { return version; }

    HttpStatus Http::ResponseHeader::GetStatus() const { return status; }

    const HttpHeaderFieldsType &Http::ResponseHeader::GetFields() const { return fields; }

    /* ResponseBody Impl */
    void Http::ResponseBody::SetResourcePath(const std::string &_resourcePath) {
        resourcePath = _resourcePath;
    }

    void Http::ResponseBody::SetData(std::string &_data) { data = std::move(_data); }

    void Http::ResponseBody::SetData(std::string &&_data) { data = std::move(_data);  }

    void Http::ResponseBody::Clear() {
        resourcePath.clear();
        data.clear();
    }

    const std::string &Http::ResponseBody::GetResourcePath() const { return resourcePath; }

    std::string&& Http::ResponseBody::GetData() { return std::move(data); }

} // totoro