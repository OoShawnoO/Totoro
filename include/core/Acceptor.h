#ifndef TOTOROSERVER_ACCEPTOR_H
#define TOTOROSERVER_ACCEPTOR_H

#include <cstring>
#include <climits>
#include "Epoller.h"
#include "IPFilter.h"

static const std::string AcceptorChan = "Acceptor";
namespace totoro {
    /**
     * @brief 负责监听新连接并分发给epoller \n response for accept new connection and dispatch new connection to epoller
     * @tparam T Connection类或继承自Connection类 \n Connection or derived from Connection.
     */
    template<class T>
    class Acceptor {
        bool& isStop                        ;
        IPFilter filter                     {};
        TCPSocket listenSocket              {};
        std::deque<Epoller<T>> reactors     {};

        Epoller<T>& RoundRobin(){
            static int index = 0;
            return reactors[++index % reactors.size()];
        }
        Epoller<T>& Minimum(){
            int minCount = INT_MAX;
            Epoller<T>* cur = nullptr;
            for(auto& reactor : reactors){
                if(reactor.CurrentConnectionCount() < minCount) cur = &reactor;
            }
            return *cur;
        }
    public:
        Acceptor(const std::string& ip,short port,int reactorCount,bool& _isStop,
                 const std::vector<in_addr_t>& _bannedIPs = {},const std::vector<in_addr_t>& _allowedIPs = {},
                 int timeOut = -1,bool _et = true,bool _oneShot = true,bool _noneBlock = false)
                 :isStop(_isStop){
            if(reactorCount <= 0) {
                LOG_FATAL("Epoller","reactor count must > 0");
                exit(-1);
            }
            for(int i=0;i<reactorCount;i++){
                reactors.emplace_back(_isStop,&filter,_et,_oneShot,_noneBlock);
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

            for(const auto& bannedIP : _bannedIPs) filter.AddBan(bannedIP);
            for(const auto& allowedIP : _allowedIPs) filter.AddAllow(allowedIP);

            for(auto& reactor : reactors) {
                std::thread(Epoller<T>::Poll,&reactor,timeOut).detach();
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
