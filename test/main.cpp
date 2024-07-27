/**
  ******************************************************************************
  * @file           : main.cpp
  * @author         : huzhida
  * @brief          : None
  * @date           : 2024/7/27
  ******************************************************************************
  */

#include <gtest/gtest.h>
#include "core/Server.h"
#include "http/HttpServer.h"
#include "http/HttpClient.h"
#include <filesystem>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999
#define HTTP_URL "http://www.baidu.com"
#define HTTPS_URL "https://www.baidu.com"
#define PROXY_IP "172.22.96.1"
#define PROXY_PORT 7890
#define HTTP_UNABLE_URL "http://www.google.com"
#define HTTPS_UNABLE_URL "https://www.google.com"

using namespace totoro;

void ReuseAddrAndPort(SOCKET sock) {
    int yes = 1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    setsockopt(sock,SOL_SOCKET,SO_REUSEPORT,&yes,sizeof(yes));
}


TcpListener GetTcpListener() {
    TcpListener listener(SERVER_IP,SERVER_PORT);
    listener.Bind();
    listener.Listen();
    ReuseAddrAndPort(listener.Sock());
    return listener;
}

TEST(TCP,send_recv) {
    std::thread server_thread([] {
        auto listener = GetTcpListener();

        TcpSocket sock;
        ASSERT_TRUE(listener.Accept(sock));

        std::string buffer;
        sock.Recv(buffer,12);

        ASSERT_EQ(buffer.size(),12);
        ASSERT_EQ(buffer,"i am client!");

        buffer = "i am server!";

        sock.SendAll(buffer);

        listener.Close();
        sock.Close();
    });
    usleep(200);
    std::string data  = "i am client!";
    TcpClient client;
    ASSERT_TRUE(client.Connect(SERVER_IP,SERVER_PORT));

    client.SendAll(data);

    client.Recv(data,12);
    ASSERT_EQ(data.size(),12);
    ASSERT_EQ(data,"i am server!");


    server_thread.join();
    client.Close();
}

TEST(TCP,recv_until) {
    std::thread server_thread([] {
        auto listener = GetTcpListener();
        ReuseAddrAndPort(listener.Sock());
        TcpSocket sock;
        ASSERT_TRUE(listener.Accept(sock));

        std::string buffer;
        sock.RecvUntil(buffer,"\r\n");

        ASSERT_EQ(buffer.size(),14);
        ASSERT_EQ(buffer,"i am client!\r\n");

        buffer = "i am server!\r\n666";

        sock.SendAll(buffer);

        listener.Close();
        sock.Close();
    });
    usleep(200);
    std::string data  = "i am client!\r\n666";
    TcpClient client;
    ASSERT_TRUE(client.Connect(SERVER_IP,SERVER_PORT));

    client.SendAll(data);

    client.RecvUntil(data,"\r\n");
    ASSERT_EQ(data.size(),14);
    ASSERT_EQ(data,"i am server!\r\n");

    server_thread.join();
    client.Close();

}

TEST(TCP,send_recv_file) {

    auto size = std::filesystem::file_size("test/test.png");

    std::thread server_thread([=] {
        auto listener = GetTcpListener();
        ReuseAddrAndPort(listener.Sock());
        TcpSocket sock;
        ASSERT_TRUE(listener.Accept(sock));

        std::string buffer;
        ASSERT_EQ(sock.Recv(buffer,size),size);

        sock.Close();
        listener.Close();
    });
    usleep(200);
    TcpClient client;
    ASSERT_TRUE(client.Connect(SERVER_IP,SERVER_PORT));

    client.SendFile("test/test.png");

    server_thread.join();
    client.Close();
}

TEST(SSL,send_recv) {

    std::thread server_thread([] {
        auto listener = GetTcpListener();
        SSLSocket ssl;
        ASSERT_TRUE(listener.Accept(ssl));

        ssl.InitSSL();

        std::string buffer;
        ssl.Recv(buffer,12);

        ASSERT_EQ(buffer.size(),12);
        ASSERT_EQ(buffer,"i am client!");

        buffer = "i am server!";

        ssl.SendAll(buffer);

        listener.Close();
        ssl.Close();
    });
    usleep(200);
    std::string data  = "i am client!";
    SSLSocket client;
    ASSERT_TRUE(client.Connect(SERVER_IP,SERVER_PORT));


    client.SendAll(data);

    client.Recv(data,12);
    ASSERT_EQ(data.size(),12);
    ASSERT_EQ(data,"i am server!");

    server_thread.join();
    client.Close();
}

TEST(SSL,recv_util) {

    std::thread server_thread([] {
        auto listener = GetTcpListener();
        SSLSocket ssl;
        ASSERT_TRUE(listener.Accept(ssl));
        ssl.InitSSL();

        std::string buffer;
        ssl.RecvUntil(buffer,"\r\n");
        ASSERT_EQ(buffer.size(),14);
        ASSERT_EQ(buffer,"i am client!\r\n");
        buffer = "i am server!\r\n666";
        ssl.SendAll(buffer);

        listener.Close();
        ssl.Close();
    });
    usleep(200);
    std::string data  = "i am client!\r\n666";
    SSLSocket client;
    ASSERT_TRUE(client.Connect(SERVER_IP,SERVER_PORT));

    client.SendAll(data);
    client.RecvUntil(data,"\r\n");
    ASSERT_EQ(data.size(),14);
    ASSERT_EQ(data,"i am server!\r\n");

    server_thread.join();
    client.Close();
}

TEST(SSL,send_recv_file) {
    auto size = std::filesystem::file_size("test/test.png");

    std::thread server_thread([=] {
        auto listener = GetTcpListener();
        SSLSocket ssl;
        ASSERT_TRUE(listener.Accept(ssl));
        ssl.InitSSL();

        std::string buffer;
        ASSERT_EQ(ssl.Recv(buffer,size),size);

        listener.Close();
        ssl.Close();
    });
    usleep(200);
    std::string data  = "i am client!\r\n666";
    SSLSocket client;
    ASSERT_TRUE(client.Connect(SERVER_IP,SERVER_PORT));

    client.SendFile("test/test.png");

    server_thread.join();
    client.Close();

}

TEST(HTTP_CLIENT,get_http) {
    HttpClient client;
    HttpRequestParameters param;
    param.url = HTTP_URL;

    ASSERT_TRUE(client.Get(param));
    ASSERT_NE(client.GetResponseBodyText().size(),0);

}

TEST(HTTP_CLIENT,get_https) {
    HttpClient client;
    HttpRequestParameters param;
    param.url = HTTPS_URL;

    ASSERT_TRUE(client.Get(param));
    ASSERT_NE(client.GetResponseBodyText().size(),0);

}

TEST(HTTP_CLIENT,get_proxy_http) {
    HttpClient client;
    HttpRequestParameters param;
    param.url = HTTP_UNABLE_URL;
    param.timeout = 300;
    ASSERT_FALSE(client.Get(param));

    param.proxies = {{"http", {PROXY_IP, PROXY_PORT}}};
    ASSERT_TRUE(client.Get(param));
}

TEST(HTTP_CLIENT,get_proxy_https) {
    HttpClient client;
    HttpRequestParameters param;
    param.url = HTTPS_UNABLE_URL;
    param.timeout = 300;
    ASSERT_FALSE(client.Get(param));

    param.proxies = {{"https", {PROXY_IP, PROXY_PORT}}};
    ASSERT_TRUE(client.Get(param));
}

TEST(HTTP_SERVER,request) {
    auto* server = new HttpServer(SERVER_IP,SERVER_PORT);
    std::thread server_thread(
            [=]{
                server->Get("/test",[](const Http::HttpRequest & request, Http::HttpResponse & response) -> bool {
                    response.header.SetContentType("text/plain");
                    response.header.SetContentLength(3);
                    response.body.SetData("123");
                    return true;
                });
                server->Run();
            }
    );
    usleep(10000);

    HttpClient client;
    HttpRequestParameters parameters;
    parameters.url = "http://127.0.0.1:9999/test";
    ASSERT_TRUE(client.Get(parameters));
    ASSERT_EQ(client.GetResponseBodyText(),"123");

    server->Close();
    server_thread.join();

}

int main () {
    testing::InitGoogleTest();
    hzd::Mole::enable(false);
    return RUN_ALL_TESTS();
}