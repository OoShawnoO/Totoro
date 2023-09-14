#ifndef TOTOROSERVER_TEST_H
#define TOTOROSERVER_TEST_H

#include <cstring>
#include <fcntl.h>
#include "core/AsyncLogger.h"
#include "core/Socket.h"
#include "Connection.h"
#include "core/Acceptor.h"
#include "core/Epoller.h"
#include "HttpBase.h"

#define ASSERT(data,right) do{      \
    if(data == right){              \
        LOG_INFO("TEST","PASS");    \
    }else{                          \
        LOG_ERROR("TEST","FAILED");  \
    }                               \
}while(0)

#define SENDER_IP "127.0.0.1"
#define SENDER_PORT 9999
#define RECVER_IP "127.0.0.1"
#define RECVER_PORT 1234

using namespace totoro;

enum TestType{
    None,Header,File
};

void TcpSendThread(std::string data,TestType type){
    TCPSocket tcpSocket;
    tcpSocket.Init(SENDER_IP,SENDER_PORT);
    tcpSocket.Bind();
    tcpSocket.Listen();

    TCPSocket clientSocket;
    if(!tcpSocket.Accept(clientSocket)){
        LOG_ERROR("Accept",strerror(errno));
        return;
    }
    switch(type){
        case None : { clientSocket.Send(data,data.size()); break;}
        case Header : { clientSocket.SendWithHeader(data.c_str(),data.size()); break;}
        case File : { clientSocket.SendFileWithHeader("test/test.png"); break; }
    }

    tcpSocket.Close();
    clientSocket.Close();
}

void UdpSendThread(std::string data,TestType type){
    UDPSocket udpSocket;
    udpSocket.Init(SENDER_IP,SENDER_PORT);
    udpSocket.Bind();
    udpSocket.SetDestAddr(RECVER_IP,RECVER_PORT);
    switch(type){
        case None : { udpSocket.Send(data,data.size()); break;}
        case Header : { udpSocket.SendWithHeader(data.c_str(),data.size()); break;}
        case File : { udpSocket.SendFileWithHeader("test/test.png"); break; }
    }
    udpSocket.Close();
}


void TEST_TcpSendRecvString(){
    std::thread t(TcpSendThread,"12345678910",None);
    usleep(1000);
    TCPSocket tcpSocket;
    std::string s;
    tcpSocket.Init();
    tcpSocket.Connect(SENDER_IP,SENDER_PORT);
    tcpSocket.Recv(s,11);
    ASSERT(s,"12345678910");
    t.join();
    tcpSocket.Close();
}

void TEST_TcpSendRecvBytes(){
    char buffer[11] = {'a','\0','b','\0','c','\0','d','\0','e','\0','f'};
    std::thread t(TcpSendThread,std::string(buffer,11),Header);
    usleep(1000);
    TCPSocket tcpSocket;
    std::string s;
    tcpSocket.Init();
    tcpSocket.Connect(SENDER_IP,SENDER_PORT);
    tcpSocket.RecvWithHeader(s);
    ASSERT(s,std::string(buffer,11));
    t.join();
    tcpSocket.Close();
}

void TEST_TcpSendRecvFile(){
    std::thread t(TcpSendThread,"",File);
    TCPSocket tcpSocket;
    usleep(1000);
    std::string s;
    tcpSocket.Init();
    tcpSocket.Connect(SENDER_IP,SENDER_PORT);
    tcpSocket.RecvFileWithHeader("test/test-tcp.png");
    struct stat s1{},s2{};
    int f1 = open("test/test.png",O_RDONLY);
    int f2 = open("test/test-tcp.png",O_RDONLY);
    fstat(f1,&s1);
    fstat(f2,&s2);
    ASSERT(s1.st_size,s2.st_size);
    t.join();
    close(f1);close(f2);
    tcpSocket.Close();
}

void TEST_UdpSendRecvString(){
    UDPSocket udpSocket;
    udpSocket.Init(RECVER_IP,RECVER_PORT);
    udpSocket.Bind();
    std::string s;
    std::thread t(UdpSendThread,"12345678910",None);
    udpSocket.Recv(s,11);
    ASSERT(s,"12345678910");
    t.join();
}

void TEST_UdpSendRecvBytes(){
    UDPSocket udpSocket;
    udpSocket.Init(RECVER_IP,RECVER_PORT);
    udpSocket.Bind();
    std::string s;
    char buffer[11] = {'a','\0','b','\0','c','\0','d','\0','e','\0','f'};
    std::thread t(UdpSendThread,std::string(buffer,11),Header);
    udpSocket.RecvWithHeader(s);
    ASSERT(s,std::string(buffer,11));
    t.join();
}

void TEST_UdpSendRecvFile(){
    UDPSocket udpSocket;
    std::string s;
    std::thread t(UdpSendThread,"",File);
    udpSocket.Init(RECVER_IP,RECVER_PORT);
    udpSocket.Bind();
    udpSocket.RecvFileWithHeader("test/test-udp.png");
    struct stat s1{},s2{};
    int f1 = open("test/test.png",O_RDONLY);
    int f2 = open("test/test-udp.png",O_RDONLY);
    fstat(f1,&s1);
    fstat(f2,&s2);
    ASSERT(s1.st_size,s2.st_size);
    t.join();
    close(f1);close(f2);
}

void TEST_Acceptor(){
    bool isStop{false};
    const auto& conf = Configure::Get()["SERVER"][0];
    Acceptor<HttpBase> acceptor(isStop,conf);
    acceptor.Run();
}

#endif //TOTOROSERVER_TEST_H
