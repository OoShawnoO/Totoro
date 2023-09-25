#ifndef TOTOROSERVER_ACCEPTOR_H
#define TOTOROSERVER_ACCEPTOR_H

#include <cstring>
#include <climits>
#include <csignal>
#include "Epoller.h"
#include "IPFilter.h"

static const std::string AcceptorChan = "Acceptor";
namespace totoro {
    /**
     * @brief 负责监听新连接并分发给epoller \n response for accept new connection and dispatch new connection to epoller
     * @tparam T Connection类或继承自Connection类 \n Connection or derived from Connection.
     */
    template<typename E>
    class Acceptor {
    protected:
        bool& isStop                        ;
        IPFilter filter                     {};
        TCPSocket listenSocket              {};
        std::deque<E> reactors              {};
        std::string ip                      {};
        unsigned short port                 {0};
        /* 轮询式reactor处理新连接 / Use round-robin algorithm to process new connection */
        E& RoundRobin(){
            static int index = 0;
            return reactors[++index % reactors.size()];
        }
        /* 使用当前最小连接数reactor处理新连接 / Use current least connection count reactor to process new connection */
        E& Minimum(){
            int minCount = INT_MAX;
            E* cur = nullptr;
            for(auto& reactor : reactors){
                if(reactor.CurrentConnectionCount() < minCount) cur = &reactor;
            }
            return *cur;
        }
    public:
        Acceptor(bool& _isStop,const Json& config)
                 :isStop(_isStop){
            ip = config["ip"];
            port =config["port"];
            int epollTimeout = config["epollTimeout"];
            int reactorNum = config["reactorNum"];
            bool edgeTriggle = config["edgeTriggle"];
            bool oneShot = config["oneShot"];
            bool noneBlock = config["noneBlock"];
            auto allow = config["allow"];
            auto deny = config["deny"];

            if(reactorNum <= 0) {
                LOG_FATAL("Epoller","reactor count must > 0");
                exit(-1);
            }
            for(int i=0;i<reactorNum;i++){
                reactors.emplace_back(_isStop,&filter,edgeTriggle,oneShot,noneBlock);
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

            for(const auto& bannedIP : deny) filter.AddBan(inet_addr(std::string(bannedIP).c_str()));
            for(const auto& allowedIP : allow) filter.AddAllow(inet_addr(std::string(allowedIP).c_str()));
            for(auto& reactor : reactors) {
                std::thread(E::Poll,&reactor,epollTimeout).detach();
            }
        }

        void Run(){
            TCPSocket tcpSocket;
            while(!isStop){
                if(!listenSocket.Accept(tcpSocket)){
                    LOG_ERROR(AcceptorChan,strerror(errno));
                    exit(-1);
                }

                if(filter.isBanned(tcpSocket.DestAddr().sin_addr.s_addr)
                || !filter.isAllowed(tcpSocket.DestAddr().sin_addr.s_addr)){
                    LOG_INFO(AcceptorChan,"connection not allowed.");
                    tcpSocket.Close();
                    continue;
                }

                Minimum().AddConnection(tcpSocket);
            }
        }
    };

} // totoro

#endif //TOTOROSERVER_ACCEPTOR_H
