#ifndef TOTOROSERVER_ACCEPTOR_H
#define TOTOROSERVER_ACCEPTOR_H

#include <cstring>
#include "Epoller.h"

namespace totoro {
    template<class T>
    class Acceptor {
        std::vector<Epoller<T>> reactors;
        TCPSocket listenSocket              {};
        bool& isStop                        ;
    public:
        Acceptor(const std::string& ip,short port,int reactorCount,bool& _isStop,bool _et = false,
                 bool _oneShot = true,bool _noneBlock = false){
            isStop = _isStop;
            for(int i=0;i<reactorCount;i++){
                reactors.emplace_back(ip,port,_isStop,_et,_oneShot,_noneBlock);
            }
            if(!listenSocket.Init(ip,port)){
                LOG_ERROR("Epoller","init listen socket failed");
                exit(-1);
            }
            if(!listenSocket.Bind()){
                LOG_ERROR("Epoller","bind listen socket failed");
                exit(-1);
            }
            if(!listenSocket.Listen()){
                LOG_ERROR("Epoller","listen socket failed");
                exit(-1);
            }
        }

        void Run(int timeOut){
            for(auto& reactor : reactors) {
                std::thread(Epoller<T>::Poll,&reactor,timeOut).detach();
            }
            TCPSocket tcpSocket;
            int index = 0;
            while(!isStop){
                if(!listenSocket.Accept(tcpSocket)){
                    LOG_ERROR("Acceptor",strerror(errno));
                    exit(-1);
                }
                reactors[(index++)%reactors.size()].AddConnection(tcpSocket);
            }
        }
    };

} // totoro

#endif //TOTOROSERVER_ACCEPTOR_H
