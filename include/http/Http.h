#ifndef TOTORO_HTTP_H
#define TOTORO_HTTP_H

#include <list>
#include "core/Connection.h"     /* Connection */

namespace totoro {
    // HTTP Form 详细信息 / HTTP Form Detail
    struct HttpMultiPartDetail {
        std::string content_type;
        std::string file_name;
        std::string data;
    };
    // HTTP 内容数据类型 / HTTP Content Data Type
    using HttpContentDataType = std::string;
    // HTTP 请求参数类型 / HTTP Request Parameter Type
    using HttpParameterType = std::unordered_map<std::string, std::string>;
    // HTTP 头字段类型 / HTTP Header Fields Type
    using HttpHeaderFieldsType = std::unordered_map<std::string, std::vector<std::string>>;
    // HTTP 表单数据类型 / HTTP Form FieldsType
    using HttpFormType = std::unordered_map<std::string, std::string>;
    // HTTP 多分界数据类型 / HTTP Multiple-part Data Type
    using HttpMultiPartType = std::unordered_map<std::string, HttpMultiPartDetail>;
    // HTTP Cookie 类型 / HTTP Cookie Data Type
    using HttpCookieType = std::unordered_map<std::string, std::string>;

    enum HttpMethod {
        GET, POST, PUT, PATCH, DELETE, TRACE, HEAD, OPTIONS, CONNECT
    };
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
        Unavailable_For_Legal_Reasons = 451,
        Internal_Server_Error = 500
    };
    enum HttpVersion {
        HTTP11, HTTP10, HTTP20, HTTP30
    };

    extern const std::unordered_map<HttpStatus, std::string> http_status_map;
    extern std::unordered_map<HttpVersion, std::string> http_version_map;
    extern std::unordered_map<std::string, HttpVersion> reverse_http_version_map;
    extern std::unordered_map<std::string, std::string> http_content_type_map;
    extern const std::string http_error_template_html;

    /**
     * @brief 负责HTTP连接相关事务 / Response HTTP connection transactions
     */

    class Http {
    public:
        // 请求头信息 / Request Header Information
        struct RequestHeader {
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

            // 解析url参数 / Parse url parameters
            void parseParameters(std::string &&parameterText);

        public:
            // 解析请求头 / Parse request header
            bool Parse(std::string &requestHeaderData);

            // 获取请求方法 / Get request method
            const HttpMethod &GetMethod() const;

            // 设置请求方法 / Set request method
            void SetMethod(HttpMethod method);

            // 获取请求内容类型 / Get request content type
            const HttpContentDataType &GetContentType() const;

            // 设置请求内容类型 / Set request content type
            void SetContentType(const std::string &contentType);

            // 获取请求内容长度 / Get request content length
            size_t GetContentLength() const;

            // 设置请求内容长度 / Set request content length
            void SetContentLength(size_t size);

            // 获取请求cookies / Get request cookies
            const HttpCookieType &GetCookies() const;

            // 设置请求cookies / Set reqeust cookies
            void SetCookie(const std::string &key, const std::string &value);

            // 获取请求HTTP版本 / Get request HTTP version
            const HttpVersion &GetVersion() const;

            // 设置请求HTTP版本 / Set reqeust HTTP version
            void SetVersion(HttpVersion version);

            // 获取请求url / Get request url
            const std::string &GetUrl() const;

            // 设置请求url / Set request url
            void SetUrl(const std::string &url);

            // 获取请求参数 / Get request parameters
            const HttpParameterType &GetParameters() const;

            // 设置请求参数 / Set request parameters
            void SetParameters(const std::string &key, const std::string &value);

            // 获取请求字段 / Get request fields
            const HttpHeaderFieldsType &GetFields() const;

            // 设置请求字段 / Set request field
            void SetField(const std::string &key, const std::vector<std::string> &values);

            // 获取多分界数据分界线 / Get request multi-part boundary
            const std::string &GetBoundary() const;

            // 设置多分界数据分界线 / Set request multi-part boundary
            void SetBoundary(const std::string &boundary);

            // 字符串化 / To string
            std::string toString();

            // 清除 / Clear
            void Clear();
        };

        // 请求体信息 / Request Body Information
        struct RequestBody {
            friend struct RequestHeader;
        private:
            // 表单数据 / Form Data
            HttpFormType form;
            // 多分界数据文件 / Multiple-part Files
            HttpMultiPartType files;
            // Json 数据 / Json Data
            Json json;
        public:
            // 解析请求体 / Parse request body
            bool Parse(const std::string &requestBodyData, const RequestHeader &header);

            // 获取请求体Json数据 / Get request body Json data
            const Json &GetJson() const;

            // 设置请求体Json数据 / Set request body Json data
            void SetJson(const Json &json);

            // 获取请求体Form数据 / Get request body Form data
            const HttpFormType &GetForm() const;

            void SetForm(const HttpFormType &form);

            // 获取请求体Form数据字段 / Get request body Form field data
            std::string GetFormField(const std::string &name);

            // 设置请求体Form数据字段 / Set request body Form field data
            void SetFormField(const std::string &key, const std::string &value);

            // 获取请求体文件 / Get request body files
            const HttpMultiPartType &GetFiles() const;

            // 获取请求体文件数据 / Get request body file data
            std::string GetFilesFieldData(const std::string &name);

            // 设置请求体文件数据 / Set request body file data
            void SetFilesFieldData(const std::string &name, const HttpMultiPartDetail &detail);

            // 获取请求体对应name字段文件名数据 / Get request body file data cross name
            std::string GetFilesFieldFileName(const std::string &name);

            // 获取请求体对应name字段文件内容 / Get request body file content cross name
            std::string GetFilesFieldContentType(const std::string &name);

            // 下载请求体中文件 / Download files in request body
            bool DownloadFilesField(const std::string &fieldName, const std::string &destFilePath,
                                    std::string fileName = "") const;

            // 字符串化 / To string
            std::string toString(const RequestHeader &header) const;

            void Clear();
        };

        // 响应头信息 / Response Header Information
        struct ResponseHeader {
        private:
            // HTTP版本 / Http Version
            HttpVersion version;
            // HTTP状态码 / Http Status
            HttpStatus status;
            // 响应头字段 / Response Header Fields
            HttpHeaderFieldsType fields;
            //
        public:
            // 解析响应头 / Parse response header
            bool Parse(std::string &responseHeaderData);

            // 获取请求内容类型 / Get request content type
            const HttpContentDataType &GetContentType() const;

            // 获取请求内容长度 / Get request content length
            size_t GetContentLength() const;

            // 获取传输编码 / Get transfer encoding
            std::string GetTransferEncoding() const;

            // 获取HTTP版本 / Get HTTP version
            HttpVersion GetVersion() const;

            // 设置HTTP版本 / Set HTTP version
            void SetVersion(HttpVersion version);

            // 获取状态码 / Get Status
            HttpStatus GetStatus() const;

            // 设置状态码 / Set Status
            void SetStatus(HttpStatus status);

            // 设置内容类型 / Set content type
            void SetContentType(const std::string &contentType);

            // 设置内容长度 / Set content length
            void SetContentLength(size_t length);

            // 设置cookie / Set cookie
            void SetCookie(const std::string &cookieKey, const std::string &cookieValue);

            // 获取字段 / Get fields
            const HttpHeaderFieldsType &GetFields() const;

            // 设置字段 / Set field
            void SetField(const std::string &fieldKey, const std::vector<std::string> &fieldValues);

            // 字符串化 / To string
            std::string toString();

            void Clear();
        };

        // 响应体信息 / Response Body Information
        struct ResponseBody {
        private:
            // 静态资源文件路径 / Static Resource Path
            std::string resourcePath;
            // 响应体数据 / Response Body Data
            std::string data;
        public:
            // 设置文件资源路径 / Set resource path
            void SetResourcePath(const std::string &resourcePath);

            // 获取文件资源路径 / Get resource path
            const std::string &GetResourcePath() const;

            // 设置传输数据 / Set transport data
            void SetData(std::string &data);

            // 设置传输数据 / Set transport data
            void SetData(std::string &&data);

            // 获取传输数据 / Get transport data
            std::string&& GetData();

            void Clear();
        };

        // 请求信息 / Request Information
        struct HttpRequest {
            // 请求头 / Request header
            RequestHeader header;
            // 请求体 / Request body
            RequestBody body;
        };

        // 响应信息 / Response Information
        struct HttpResponse {
            // 响应头
            ResponseHeader header;
            // 响应体
            ResponseBody body;
        };

        void Clear();
    protected:
        // 请求信息 / Request Information
        HttpRequest request{};
        // 响应信息 / Response Information
        HttpResponse response{};
        // 请求头报文 / Request header text
        std::string request_header_text;
        // 请求体报文 / Request body text
        std::string request_body_text;
        // 响应头报文 / Response header text
        std::string response_header_text;
        // 响应体报文 / Response body text
        std::string response_body_text;

    };

} // totoro

#endif //TOTORO_HTTP_H
