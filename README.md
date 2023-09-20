# TotoroServer

Reconfiguration for conv_event
https://github.com/OoShawnoO/conv_event

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

#### Http Server
使用配置文件 etc/config.json进行初始化 / Using Configure etc/config.json to Initialize

- HTTP  正向代理 / HTTP Forward Proxy
  ```c++
  bool isStop{false};
  // 将从etc/config.json中选取SERVER字段中第一个配置进行初始化服务器
  // it will choose etc/config.json first of field 'SERVER' json object ot initialize server
  const auto& conf = Configure::Get()["SERVER"][0];
  HttpForwardServer acceptor(isStop,conf);
  acceptor.Run();
  ```
- HTTP  反向代理 / HTTP Reverse Proxy
  ```c++
  bool isStop{false};
  // 将从etc/config.json中选取SERVER字段中第一个配置进行初始化服务器
  // it will choose etc/config.json first of field 'SERVER' json object ot initialize server
  const auto& conf = Configure::Get()["SERVER"][0];
  // 将从etc/config.json中选取HTTP_REVERSE_PROXY字段中寻找对应反向代理端口,并转发至目的服务器
  // it will choose etc/config.json field 'HTTP_REVERSE_PROXY' and then find field at port that need reverse proxy,forward to destination server
  HttpReverseServer acceptor(isStop,conf);
  acceptor.Run();
  ```
- HTTPS 正向代理 / HTTPS Forward Proxy
```c++
  bool isStop{false};
  // 将从etc/config.json中选取SERVER字段中第一个配置进行初始化服务器
  // it will choose etc/config.json first of field 'SERVER' json object ot initialize server
  const auto& conf = Configure::Get()["SERVER"][0];
  HttpsForwardServer acceptor(isStop,conf);
  acceptor.Run();
  ```
- HTTPS 反向代理 / HTTPS Reverse Proxy
  ```c++
  bool isStop{false};
  // 将从etc/config.json中选取SERVER字段中第一个配置进行初始化服务器
  // it will choose etc/config.json first of field 'SERVER' json object ot initialize server
  const auto& conf = Configure::Get()["SERVER"][0];
  // 将从etc/config.json中选取HTTP_REVERSE_PROXY字段中寻找对应反向代理端口,并转发至目的服务器
  // it will choose etc/config.json field 'HTTPS_REVERSE_PROXY' and then find field at port that need reverse proxy,forward to destination server
  HttpReverseServer acceptor(isStop,conf);
  acceptor.Run();
  ```