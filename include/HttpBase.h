#ifndef TOTORO_HTTPBASE_H
#define TOTORO_HTTPBASE_H

#include "Connection.h"

namespace totoro {
    // HTTP Form 详细信息 / HTTP Form Detail
    struct HttpMultiPartDetail{
        std::string contentType;
        std::string fileName;
        std::string data;
    };
    // HTTP 内容类型 / HTTP Content Type
    using HttpContentType = std::string;
    // HTTP 内容数据类型 / HTTP Content Data Type
    using HttpContentDataType = std::string;
    // HTTP 请求参数类型 / HTTP Request Parameter Type
    using HttpParameterType = std::unordered_map<std::string,std::string>;
    // HTTP 头字段类型 / HTTP Header Fields Type
    using HttpHeaderFieldsType = std::unordered_map<std::string,std::vector<std::string>>;
    // HTTP 表单数据类型 / HTTP Form FieldsType
    using HttpFormType = std::unordered_map<std::string,std::string>;
    // HTTP 多分界数据类型 / HTTP Multiple-part Data Type
    using HttpMultiPartType = std::unordered_map<std::string,HttpMultiPartDetail>;
    // HTTP Cookie 类型 / HTTP Cookie Data Type
    using HttpCookieType = std::unordered_map<std::string,std::string>;

    enum HttpMethod { GET,POST,PUT,PATCH,DELETE,TRACE,HEAD,OPTIONS,CONNECT };
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
    enum HttpVersion { HTTP11,HTTP10,HTTP20,HTTP30};

    extern const std::unordered_map<HttpStatus, std::string> HttpStatusMap;
    extern std::unordered_map<HttpVersion,std::string> HttpVersionMap;
    extern std::unordered_map<std::string,HttpVersion> ReverseHttpVersionMap;
    extern std::unordered_map<std::string,std::string> HttpContentTypeMap;
    extern const std::string HttpErrorTemplateHtml;

    class HttpBase : public Connection {
    protected:
        int ReadCallback() override;
        int AfterReadCallback() override;
        int WriteCallback() override;
        int AfterWriteCallback() override;
        virtual void RenderStatus(HttpStatus status);
        virtual bool SendResponseHeader();
        virtual bool SendResponseBody();
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
            bool Parse(std::string& requestHeaderData);
            const HttpMethod& GetMethod() const;
            const std::string& GetContentType() const;
            size_t GetContentLength() const;
            const HttpCookieType& GetCookies() const;
            const HttpVersion& GetVersion() const;
            const std::string& GetUrl() const;
            const HttpParameterType& GetParameters() const;
            const HttpHeaderFieldsType& GetFields() const;
            const std::string& GetBoundary() const;
            void Clear();
        }requestHeader;
        // 请求体信息 / Request Body Information
        struct RequestBody{
            friend struct RequestHeader;
        private:
            // 表单数据 / Form Data
            HttpFormType form;
            // 多分界数据文件 / Multiple-part Files
            HttpMultiPartType files;
            // Json 数据 / Json Data
            Json json;
        public:
            bool Parse(const std::string& requestBodyData,const RequestHeader& header);
            const Json& GetJson() const;
            const HttpFormType& GetForm() const;
            std::string GetFormField(const std::string& name);
            const HttpMultiPartType& GetFiles() const;
            std::string GetFilesFieldData(const std::string& name);
            std::string GetFilesFieldFileName(const std::string& name);
            std::string GetFilesFieldContentType(const std::string& name);
            bool DownloadFilesField(const std::string& fieldName,const std::string& destFilePath,std::string fileName = "") const;
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
        public:
            void SetVersion(HttpVersion version);
            void SetStatus(HttpStatus status);
            void SetContentType(const std::string& contentType);
            void SetContentLength(size_t length);
            void SetCookie(const std::string& cookieKey,const std::string& cookieValue);
            void SetField(const std::string& fieldKey,const std::vector<std::string>& fieldValues);
            std::string toString();
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
            const std::string& GetResourcePath();
            void SetData(std::string& data);
            const std::string& GetData();
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
