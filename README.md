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
