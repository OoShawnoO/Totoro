/**
  ******************************************************************************
  * @file           : main.cpp
  * @author         : huzhida
  * @brief          : None
  * @date           : 2024/5/19
  ******************************************************************************
  */

#include <iostream>
#include "core/Server.h"
#include "http/HttpServer.h"
#include "http/HttpClient.h"


int main() {
//    totoro::HttpsServer server("127.0.0.1",9999);
//
//    server.Get("/",[](const totoro::Http::HttpRequest & request, totoro::Http::HttpResponse & response) -> bool {
////        response.header.SetContentType("text/plain");
////        response.body.SetData("123");
//
//        response.header.SetContentType("application/json");
//        response.body.SetResourcePath("etc/config.json");
//
//        return true;
//    });
//
//    server.Run();

    totoro::HttpClient client;

    totoro::HttpRequestParameters parameters;
    parameters.url = "https://www.google.com";
    parameters.proxies = {{"https",{"172.22.96.1",7890}}};
    if(client.Get(parameters)) {
       std::cout << client.GetResponseBodyText() << std::endl;
    }


}