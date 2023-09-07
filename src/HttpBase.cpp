#include "HttpBase.h"

#include <utility>
#include <sstream>
#include <regex>
#include "fmt/format.h"
static const std::string HttpBaseChan = "HttpBase";
namespace totoro {
    static const std::regex RequestLineRegex("^(.*) (.*) (.*)$");
    static const std::regex RequestParameterRegex("([^&]+)=([^&]*)&?");
    static const std::regex RequestFieldRegex("^(.*?):\\s?(.*?);?$");

    static auto getLine = [](std::stringstream& _stream,std::string& _line) -> std::basic_istream<char>&{
        auto& ret = std::getline(_stream,_line,'\r');
        _stream.ignore(1);
        return ret;
    };

    /* Connection Protected Impl */
    int HttpBase::ReadCallback() {
        return Connection::ReadCallback();
    }

    int HttpBase::AfterReadCallback() {
        return Connection::AfterReadCallback();
    }

    int HttpBase::WriteCallback() {
        return Connection::WriteCallback();
    }

    int HttpBase::AfterWriteCallback() {
        return Connection::AfterWriteCallback();
    }

    /* RequestHeader Impl */
    void HttpBase::RequestHeader::Clear() {
        url.clear();
        boundary.clear();
        fields.clear();
        cookies.clear();
    }

    void HttpBase::RequestHeader::parseParameters(std::string&& parameterText) {
        std::smatch match;
        auto pos = parameterText.cbegin();
        auto end = parameterText.cend();
        for(;std::regex_search(pos,end,match,RequestParameterRegex);pos = match.suffix().first){
            parameters[match[1]] = match[2];
        }
    }

    bool HttpBase::RequestHeader::Parse(const std::string &requestHeaderData) {
        if(requestHeaderData.empty()) {
            LOG_ERROR(HttpBaseChan,"request header empty");
            return false;
        }
        std::stringstream stream(requestHeaderData);
        std::string line;

        if(!getLine(stream,line)) {
            LOG_ERROR(HttpBaseChan,"request header content can't get line");
            return false;
        }

        std::cmatch matches;
        if(!std::regex_match(line.c_str(), matches, RequestLineRegex)){
            LOG_ERROR(HttpBaseChan,"match first line failed");
            return false;
        }
        try{
            url = matches[2];
            method = ReverseHttpMethodMap.at(matches[1]);
            version = ReverseHttpVersionMap.at(matches[3]);
        }catch(...){
            LOG_ERROR(HttpBaseChan,fmt::format("map not found {} or {}",matches[1].str(),matches[3].str()));
            return false;
        }

        auto pos = url.find('?');
        if(pos != std::string::npos){
            parseParameters(url.substr(pos+1));
            url.erase(pos);
        }

        while(getLine(stream,line)){
            if(line.empty()) break;
            if(!std::regex_match(line.c_str(),matches,RequestFieldRegex)){
                LOG_ERROR(HttpBaseChan,"field match failed");
                return false;
            }
            std::stringstream fieldStream(matches[2]);
            auto& field = fields[matches[1]];
            while(std::getline(fieldStream,line,';')) {
                line.erase(line.begin(),std::find_if_not(line.begin(),line.end(),::isspace));
                line.erase((std::find_if_not(line.rbegin(),line.rend(),::isspace).base(),line.end()));
                field.emplace_back(std::move(line));
            }
        }

        for(const auto& cookie : fields["Cookie"]){
            pos = cookie.find('=');
            cookies[cookie.substr(0,pos-1)] = cookie.substr(pos+1);
        }

        if(method == POST || method == PATCH || method == PUT){
            auto& CT = fields["Content-Type"];
            auto it = std::find_if(CT.begin(), CT.end(),[](const std::string& field){
                return field.find("boundary") != std::string::npos;
            });
            if(it == CT.end()) return false;
            pos = it->find('=');
            boundary = it->substr(pos + 1);
        }

        return true;
    }

    const std::string &HttpBase::RequestHeader::GetContentType() const {
        return fields.find("Content-Type")->second[0];
    }

    size_t HttpBase::RequestHeader::GetContentLength() const {
        return stoul(fields.find("Content-Length")->second[0]);
    }

    const HttpCookieType& HttpBase::RequestHeader::GetCookies() const { return cookies; }

    const HttpVersion &HttpBase::RequestHeader::GetVersion() const { return version; }

    const std::string &HttpBase::RequestHeader::GetUrl() const { return url; }

    const HttpHeaderFieldsType &HttpBase::RequestHeader::GetFields() const { return fields; }

    const HttpParameterType &HttpBase::RequestHeader::GetParameters() const { return parameters; }

    /* RequestBody Impl */
    void HttpBase::RequestBody::Clear() {
        form.clear();
    }

    bool HttpBase::RequestBody::Parse(const std::string &requestBodyData,const std::string& boundary) {
        if(requestBodyData.empty() || boundary.empty()) {
            LOG_ERROR(HttpBaseChan,"request body empty or boundary not found");
            return false;
        }
        const std::string another = "--" + boundary;
        const std::string end = "--" + boundary + "--";
        std::stringstream stream(requestBodyData);
        std::string line;

        std::string name;
        std::string contentType;
        std::string fileName;
        std::string data;
        while(getLine(stream,line)){
            if(line == end) {
                form[name] = {std::move(contentType),std::move(fileName),std::move(data)};
                break;
            }
            if(line == another){
                if(!name.empty())
                    form[name] = {std::move(contentType),std::move(fileName),std::move(data)};
            }
        }

        return false;
    }

    const HttpMultiPartType &HttpBase::RequestBody::GetForm() const { return form; }

    bool HttpBase::RequestBody::DownloadFile(const std::string &fieldName, const std::string &destFilePath) const {
        auto file = form.find(fieldName);
        if(file == form.end()) return false;

        std::ofstream out(destFilePath + file->second[2],std::ios::out);
        if(!out.is_open()) return false;
        out.write(file->second[3].c_str(),(std::streamsize)file->second[3].size());
        return true;
    }

    /* ResponseHeader Impl */
    std::string HttpBase::ResponseHeader::toString() {
        std::string responseBodyText = fmt::format("{} {} {}\r\n",
                                                   HttpVersionMap.at(version),
                                                   std::to_string((uint32_t)status),
                                                   HttpStatusMap.at(status));
        for(auto& field : fields){
            std::string values;
            for(auto& value : field.second){
                values += fmt::format("{};",std::move(value));
            }
            responseBodyText += fmt::format("{}:{}\r\n",field.first,std::move(values));
        }
        responseBodyText += "\r\n";

        return responseBodyText;
    }

    void HttpBase::ResponseHeader::Clear() { fields.clear(); }

    void HttpBase::ResponseHeader::SetVersion(HttpVersion _version) { version = _version; }

    void HttpBase::ResponseHeader::SetStatus(HttpStatus _status) { status = _status; }

    void HttpBase::ResponseHeader::SetContentType(const std::string &contentType) {
        auto& field = fields["Content-Type"];
        if(field.empty())
            field.emplace_back(contentType);
        else
            field[0] = contentType;
    }

    void HttpBase::ResponseHeader::SetContentLength(size_t length) {
        auto& field = fields["Content-Length"];
        if(field.empty())
            field.emplace_back(std::to_string(length));
        else
            field[0] = std::to_string(length);
    }

    void HttpBase::ResponseHeader::SetCookie(const std::string &cookieKey,
                                             const std::string &cookieValue) {
        auto& field = fields["Set-Cookie"];
        field.emplace_back(fmt::format("{}={}",cookieKey,cookieValue));
    }

    void HttpBase::ResponseHeader::SetField(const std::string &fieldKey,
                                            const std::vector<std::string> &fieldValues) {
        fields[fieldKey] = fieldValues;
    }

    /* ResponseBody Impl */
    void HttpBase::ResponseBody::SetResourcePath(const std::string &_resourcePath) {
        resourcePath = _resourcePath;
    }

    void HttpBase::ResponseBody::SetData(std::string _data) { data = std::move(_data); }

    void HttpBase::ResponseBody::Clear() {
        resourcePath.clear();
        data.clear();
    }
    /* HttpBase Protected Impl */
    bool HttpBase::GetHandler() {
        return false;
    }

    bool HttpBase::PostHandler() {
        return false;
    }

    bool HttpBase::PutHandler() {
        return false;
    }

    bool HttpBase::PatchHandler() {
        return false;
    }

    bool HttpBase::DeleteHandler() {
        return false;
    }

    bool HttpBase::TraceHandler() {
        return false;
    }

    bool HttpBase::HeadHandler() {
        return false;
    }

    bool HttpBase::OptionsHandler() {
        return false;
    }

    bool HttpBase::ConnectHandler() {
        return false;
    }
} // totoro