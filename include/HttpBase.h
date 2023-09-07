#ifndef TOTORO_HTTPBASE_H
#define TOTORO_HTTPBASE_H

#include "Connection.h"

namespace totoro {
    // HTTP 内容类型 / HTTP Content Type
    using HttpContentType = std::string;
    // HTTP 内容数据类型 / HTTP Content Data Type
    using HttpContentDataType = std::string;
    // HTTP 请求参数类型 / HTTP Request Parameter Type
    using HttpParameterType = std::unordered_map<std::string,std::string>;
    // HTTP 头字段类型 / HTTP Header Fields Type
    using HttpHeaderFieldsType = std::unordered_map<std::string,std::vector<std::string>>;
    // HTTP 多分界数据类型 / HTTP Multiple-part Data Type -> array[0] content-type / array[1] filename / array[2] data
    using HttpMultiPartType = std::unordered_map<std::string,std::array<std::string,3>>;
    // HTTP Cookie 类型 / HTTP Cookie Data Type
    using HttpCookieType = std::unordered_map<std::string,std::string>;
    /* region Http Method Map */
    enum HttpMethod { GET,POST,PUT,PATCH,DELETE,TRACE,HEAD,OPTIONS,CONNECT };
    const std::unordered_map<HttpMethod,std::string> HttpMethodMap{
            {GET,"GET"},{POST,"POST"},{PUT,"PUT"},{PATCH,"PATCH"},{DELETE,"DELETE"},
            {HEAD,"HEAD"},{TRACE,"TRACE"},{OPTIONS,"OPTIONS"},{CONNECT,"CONNECT"}
    };
    const std::unordered_map<std::string,HttpMethod> ReverseHttpMethodMap{
            {"GET",GET},{"POST",POST},{"PUT",PUT},{"PATCH",PATCH},{"DELETE",DELETE},
            {"TRACE",TRACE},{"HEAD",HEAD},{"OPTIONS",OPTIONS},{"CONNECT",CONNECT},
    };
    /* endregion */
    /* region Http Status Map */
    enum class HttpStatus : int32_t {
        Continue = 100,
        Switching_Protocols = 101,
        Processing = 102,
        OK = 200,
        Created = 201,
        Accepted = 202,
        Non_Authoritative_information = 203,
        No_Content = 204,
        Reset_Content = 205,
        Partial_Content = 206,
        Multi_Status = 207,
        Multiple_Choice = 300,
        Moved_Permanently = 301,
        Move_Temporarily = 302,
        See_Other = 303,
        Not_Modified = 304,
        Use_Proxy = 305,
        Switch_Proxy = 306,
        Temporary_Redirect = 307,
        Bad_Request = 400,
        Unauthorized = 401,
        Payment_Required = 402,
        Forbidden = 403,
        Not_Found = 404,
        Method_Not_Allowed = 405,
        Not_Acceptable = 406,
        Proxy_Authentication_Required = 407,
        Request_Timeout = 408,
        Conflict = 409,
        Gone = 410,
        Length_Required = 411,
        Precondition_Failed = 412,
        Request_Entity_Too_Large = 413,
        Request_URI_Too_Long = 414,
        Unsupported_Media_Type = 415,
        Requested_Range_Not_Satisfiable = 416,
        Expectation_Failed = 417,
        I_Am_A_Teapot = 418,
        Misdirected_Request = 421,
        Unprocessable_Entity = 422,
        Locked = 423,
        Failed_Dependency = 422,
        Too_Early = 425,
        Upgrade_Required = 426,
        Retry_With = 449,
        Unavailable_For_Legal_Reasons = 451
    };
    const std::unordered_map<HttpStatus, std::string> HttpStatusMap{
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
                    {HttpStatus::Unavailable_For_Legal_Reasons,   "Unavailable For Legal Reasons"}
    };
    /* endregion */
    /* region Http Version Map */
    enum HttpVersion { HTTP11,HTTP10,HTTP20,HTTP30};
    std::unordered_map<HttpVersion,std::string> HttpVersionMap{
            {HTTP11,"HTTP/1.1"},{HTTP10,"HTTP/1.0"},{HTTP20,"HTTP/2.0"},{HTTP30,"HTTP/3.0"}
    };
    std::unordered_map<std::string,HttpVersion> ReverseHttpVersionMap{
            {"HTTP/1.1",HTTP11},{"HTTP/1.0",HTTP10},{"HTTP/2.0",HTTP20},{"HTTP/3.0",HTTP30}
    };
    /* endregion */
    /* region Http Content Type Map */
    std::unordered_map<std::string,std::string> HttpContentTypeMap
            {
                    {"html", "text/html"},
                    {"htm", "text/html"},
                    {"shtml", "text/html"},
                    {"css", "text/css"},
                    {"xml", "text/xml"},
                    {"gif", "image/gif"},
                    {"jpeg", "image/jpeg"},
                    {"jpg", "image/jpeg"},
                    {"js", "application/javascript"},
                    {"atom", "application/atom+xml"},
                    {"rss", "application/rss+xml"},
                    {"mml", "text/mathml"},
                    {"txt", "text/plain"},
                    {"jad", "text/vnd.sun.j2me.app-descriptor"},
                    {"wml", "text/vnd.wap.wml"},
                    {"htc", "text/x-component"},
                    {"png", "image/png"},
                    {"tif", "image/tiff"},
                    {"tiff", "image/tiff"},
                    {"wbmp", "image/vnd.wap.wbmp"},
                    {"ico", "image/x-icon"},
                    {"jng", "image/x-jng"},
                    {"bmp", "image/x-ms-bmp"},
                    {"svg", "image/svg+xml"},
                    {"svgz", "image/svg+xml"},
                    {"webp", "image/webp"},
                    {"woff", "application/font-woff"},
                    {"woff2","application/font-woff"},
                    {"jar", "application/java-archive"},
                    {"war", "application/java-archive"},
                    {"ear", "application/java-archive"},
                    {"json", "application/json"},
                    {"hqx", "application/mac-binhex40"},
                    {"doc", "application/msword"},
                    {"pdf", "application/pdf"},
                    {"ps", "application/postscript"},
                    {"eps", "application/postscript"},
                    {"ai", "application/postscript"},
                    {"rtf", "application/rtf"},
                    {"m3u8", "application/vnd.apple.mpegurl"},
                    {"kml", "application/vnd.google-earth.kml+xml"},
                    {"kmz", "application/vnd.google-earth.kmz"},
                    {"xls", "application/vnd.ms-excel"},
                    {"eot", "application/vnd.ms-fontobject"},
                    {"ppt", "application/vnd.ms-powerpoint"},
                    {"odg", "application/vnd.oasis.opendocument.graphics"},
                    {"odp", "application/vnd.oasis.opendocument.presentation"},
                    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
                    {"odt", "application/vnd.oasis.opendocument.text"},
                    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
                    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
                    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
                    {"wmlc", "application/vnd.wap.wmlc"},
                    {"7z", "application/x-7z-compressed"},
                    {"cco", "application/x-cocoa"},
                    {"jardiff", "application/x-java-archive-diff"},
                    {"jnlp", "application/x-java-jnlp-file"},
                    {"run", "application/x-makeself"},
                    {"pl", "application/x-perl"},
                    {"pm", "application/x-perl"},
                    {"prc", "application/x-pilot"},
                    {"pdb", "application/x-pilot"},
                    {"rar", "application/x-rar-compressed"},
                    {"rpm", "application/x-redhat-package-manager"},
                    {"sea", "application/x-sea"},
                    {"sit", "application/x-stuffit"},
                    {"tcl", "application/x-tcl"},
                    {"tk", "application/x-tcl"},
                    {"der", "application/x-x509-ca-cert"},
                    {"pem", "application/x-x509-ca-cert"},
                    {"crt", "application/x-x509-ca-cert"},
                    {"xpi", "application/x-xpinstall"},
                    {"xhtml", "application/xhtml+xml"},
                    {"xspf", "application/xspf+xml"},
                    {"zip", "application/zip"},
                    {"bin", "application/octet-stream"},
                    {"exe", "application/octet-stream"},
                    {"dll", "application/octet-stream"},
                    {"deb", "application/octet-stream"},
                    {"dmg", "application/octet-stream"},
                    {"iso", "application/octet-stream"},
                    {"img", "application/octet-stream"},
                    {"msi", "application/octet-stream"},
                    {"msp", "application/octet-stream"},
                    {"msm", "application/octet-stream"},
                    {"mid", "audio/midi"},
                    {"midi", "audio/midi"},
                    {"kar", "audio/midi"},
                    {"mp3", "audio/mpeg"},
                    {"ogg", "audio/ogg"},
                    {"m4a", "audio/x-m4a"},
                    {"ra", "audio/x-realaudio"},
                    {"3gpp", "video/3gpp"},
                    {"3gp", "video/3gpp"},
                    {"ts", "video/mp2t"},
                    {"mp4", "video/mp4"},
                    {"mpeg", "video/mpeg"},
                    {"mpg", "video/mpeg"},
                    {"mov", "video/quicktime"},
                    {"webm", "video/webm"},
                    {"flv", "video/x-flv"},
                    {"m4v", "video/x-m4v"},
                    {"mng", "video/x-mng"},
                    {"asx", "video/x-ms-asf"},
                    {"asf", "video/x-ms-asf"},
                    {"wmv", "video/x-ms-wmv"},
                    {"avi", "video/x-msvideo"},
            };
    /* endregion */
    class HttpBase : public Connection {
        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
    protected:
        // 请求头信息 / Request Header Information
        struct RequestHeader{
        private:
            // HTTP版本 / Http Version
            HttpVersion version;
            // 请求方法 / Request Method
            HttpMethod method;
            // 路由路径 / Route Path
            std::string url;
            // 请求参数 / Request Parameters
            HttpParameterType parameters;
            // 请求字段 / Request Fields
            HttpHeaderFieldsType fields;
            // Cookie / Cookie
            HttpCookieType cookies;
            // 多分界数据分界线 / Multiple-part Boundary
            std::string boundary;
            void parseParameters(std::string&& parameterText);
        public:
            bool Parse(const std::string& requestHeaderData);
            const std::string& GetContentType() const;
            size_t GetContentLength() const;
            const HttpCookieType& GetCookies() const;
            const HttpVersion& GetVersion() const;
            const std::string& GetUrl() const;
            const HttpParameterType& GetParameters() const;
            const HttpHeaderFieldsType& GetFields() const;
            void Clear();
        }requestHeader;
        // 请求体信息 / Request Body Information
        struct RequestBody{
            friend struct RequestHeader;
        private:
            // 多分界数据文件 / Multiple-part Files
            HttpMultiPartType form;
        public:
            bool Parse(const std::string& requestBodyData,const std::string& boundary);
            const HttpMultiPartType& GetForm() const;
            bool DownloadFile(const std::string& fieldName,const std::string& destFilePath) const;
            void Clear();
        }requestBody;
        // 响应头信息 / Response Header Information
        struct ResponseHeader{
        private:
            // HTTP版本 / Http Version
            HttpVersion version;
            // HTTP状态码 / Http Status
            HttpStatus status;
            // 响应头字段 / Response Header Fields
            HttpHeaderFieldsType fields;

            std::string toString();
        public:
            void SetVersion(HttpVersion version);
            void SetStatus(HttpStatus status);
            void SetContentType(const std::string& contentType);
            void SetContentLength(size_t length);
            void SetCookie(const std::string& cookieKey,const std::string& cookieValue);
            void SetField(const std::string& fieldKey,const std::vector<std::string>& fieldValues);
            void Clear();
        }responseHeader;
        // 响应体信息 / Response Body Information
        struct ResponseBody{
        private:
            // 静态资源文件路径 / Static Resource Path
            std::string resourcePath;
            // 响应体数据 / Response Body Data
            std::string data;
        public:
            void SetResourcePath(const std::string& resourcePath);
            void SetData(std::string data);
            void Clear();
        }responseBody;
        // Get 方法处理 / Get Request Handler
        virtual bool GetHandler();
        // Post 方法处理 / Post Request Handler
        virtual bool PostHandler();
        // Put 方法处理 / Put Request Handler
        virtual bool PutHandler();
        // Patch 方法处理 / Patch Request Handler
        virtual bool PatchHandler();
        // Delete 方法处理 / Delete Request Handler
        virtual bool DeleteHandler();
        // Trace 方法处理 / Trace Request Handler
        virtual bool TraceHandler();
        // Head 方法处理 / Head Request Handler
        virtual bool HeadHandler();
        // Options 方法处理 / Options Request Handler
        virtual bool OptionsHandler();
        // Connect 方法处理 / Connect Request Handler
        virtual bool ConnectHandler();

    };

} // totoro

#endif //TOTORO_HTTPBASE_H
