# Totoro

- 单Reactor/主从Reactor模型
- Single Reactor / Master-slave Reactor Model
- 兼容Linux系统C++ 11以上版本
- Compatible with Linux / C++ 11 or above
- 使用信号驱动IO + 用户态异步IO模式
- Signal Driven IO + User Model Async IO

### Third Party: 
> nlohmann/json
> - github:https://github.com/nlohmann/json 
> - include/Json.hpp

> fmtlib/fmt 
> - github:https://github.com/fmtlib/fmt
> - include: thirdparty/fmt/src/include/
> - lib:thirdparty/fmt/build/libfmt.a


### Usage:
- 克隆项目
- clone project
```shell 
git clone https://github.com/OoShawnoO/Totoro.git
```
- 由于使用为HTTPS使用了SSL,Totoro依赖于openssl 
- because of using SSL for HTTPS,Totoro is dependent on openssl.
```shell
# Ubuntu
sudo apt install -y openssl && sudo apt install -y libssl-dev 
```

#### Server
```c++
// 最简单的echo服务器
#include "core/Server.h"

class echo : public totoro::Connection {
protected:
  void Handler() override {
      std::string data;
      Recv(data,1024);
      // 通过Submit可以延迟发送，等待后续可能存在的数据进行拼接发送
      Submit(std::move(data));
      // 也可以直接使用
      // SendAll(data);
  }
};

int main() {
    totoro::Server<echo> server("127.0.0.1",9999);
    server.Run();
}
```

#### Http Server
```c++
// 最简单的Http Server
#include "http/HttpServer.h"


int main() {
    totoro::HttpServer server("127.0.0.1",9999);

    server.Get("/",[](const totoro::Http::HttpRequest & request, totoro::Http::HttpResponse & response) -> bool {
//        response.header.SetContentType("text/plain");
//        response.body.SetData("123");

        response.header.SetContentType("application/json");
        response.body.SetResourcePath("etc/config.json");

        return true;
    });

    server.Run();

}
```

#### Http Client
- 提供方法 / Provide Methods

- 请求参数 / Request Parameters
    ```c++ 
    /**
     * @brief 请求相关参数 \n Parameters for request
     */
     struct HttpRequestParameters{
         // 请求完整url / Request complete url
         std::string                                url{};
         // 请求头字段 / Request header fields
         HttpHeaderFieldsType                       headers{};
         // 请求参数 / Request parameters
         HttpParameterType                          parameters{};
         // 请求cookies / Request cookies
         HttpCookieType                             cookies{};
         // 请求代理 / Request proxy
         std::unordered_map<std::string,
         std::pair<std::string,unsigned short>>     proxies{};
         // 请求文件 / Request files
         HttpMultiPartType                          files{};
         // 请求表单 / Request form
         HttpFormType                               forms{};
         // 请求json / Request json
         nlohmann::json                             json{};
         // 超时时长(ms) / timeout duration(ms)
         int                                        timeout{-1};
     };
    ```
- Get 方法 / Method Get
    ```c++
    /* HTTP 请求*/
    HttpRequestParameters params;
    params.url = "http://www.baidu.com";
    HttpClient client;
    if(!client.Get(params)) { return false; }
    client.GetResponseContent();
    
    /* HTTPS 请求 */
    HttpRequestParameters params;
    params.url = "https://www.baidu.com";
    HttpClient client;
    if(!client.Get(params)) { return false; }
    client.GetResponseContent();
    ```
- Post 方法 / Method Post
    ```c++
    /* HTTP 请求*/
    HttpClient client;
    HttpRequestParameters parameters;
    parameters.url = "http://127.0.0.1:8888";
    parameters.forms = {{"a","1"},{"b","2"}};
    if(!client.Post(parameters)) { return false; }
    client.GetResponseContent();
    /* HTTPS 请求 */
    HttpClient client;
    HttpRequestParameters parameters;
    parameters.url = "https://127.0.0.1:8888";
    parameters.forms = {{"a","1"},{"b","2"}};
    if(!client.Post(parameters)) { return false; }
    client.GetResponseContent();
    ```
