#ifndef TOTORO_FORWARDEPOLLER_H
#define TOTORO_FORWARDEPOLLER_H

#include "Epoller.h"

namespace totoro {
    template<class T>
    class ForwardEpoller : public Epoller<T> {
    public:
        explicit ForwardEpoller(bool& _isStop,IPFilter* _filter = nullptr,
                         bool _et = false,bool _oneShot = true,bool _noneBlock = true):
                Epoller<T>(_isStop,_filter,_et,_oneShot,_noneBlock){}

        void DelConnection(std::shared_ptr<T> &conn) override {
            if(Epoller<T>::EpollDel(conn->Sock())<0){
                LOG_ERROR(EpollerChan, strerror(errno));
            }
            int sock = conn->Sock();
            int ret = conn->Close();
            if(ret >= 0) {
                Epoller<T>::EpollDel(ret);
                Epoller<T>::connectionMap.erase(ret);
            }
            Epoller<T>::connectionPool.release(conn);
            Epoller<T>::connectionMap.erase(sock);
            Epoller<T>::currentConnectCount--;
        }

    protected:
        bool getMapIterator
        (SocketID cur, typename Epoller<T>::ConnectionMapIterator &mapIterator) override {
            if((mapIterator = Epoller<T>::connectionMap.find(cur)) == Epoller<T>::connectionMap.end()){

                typename Epoller<T>::ForwardCandidateMapIterator forwardIter;
                if((forwardIter = Epoller<T>::forwardCandidateMap.find(cur)) != Epoller<T>::forwardCandidateMap.end()){
                    if((mapIterator = Epoller<T>::connectionMap.find(forwardIter->second)) == Epoller<T>::connectionMap.end()){
                        LOG_ERROR(EpollerChan,"not found connection for forward socket");
                        return false;
                    }
                    if(!Epoller<T>::connectionMap.insert({cur,mapIterator->second}).second){
                        LOG_ERROR(EpollerChan,"can't insert forward socket to connection map");
                        return false;
                    }
                    Epoller<T>::forwardCandidateMap.erase(forwardIter);
                    return true;
                }

                std::shared_ptr<T> conn;
                sockaddr_in myAddr{},destAddr{};
                socklen_t addrLen = sizeof(myAddr);

                if(!Epoller<T>::connectionPool.acquire(conn)){
                    close(cur);
                    LOG_ERROR(EpollerChan,"connection pool acquire failed");
                    return false;
                }

                if(getsockname(cur,(sockaddr*)&myAddr,&addrLen) < 0){
                    close(cur);
                    LOG_ERROR(EpollerChan,"can't get sock my address");
                    return false;
                }
                if(getpeername(cur,(sockaddr*)&destAddr,&addrLen) < 0){
                    close(cur);
                    LOG_ERROR(EpollerChan,"can't get sock dest address");
                    return false;
                }

                Epoller<T>::connectionInitParameter.sock = cur;
                Epoller<T>::connectionInitParameter.myAddr = myAddr;
                Epoller<T>::connectionInitParameter.destAddr = destAddr;
                int ret = conn->Init(Epoller<T>::connectionInitParameter);
                if(ret < 0){
                    conn->Close();
                    Epoller<T>::connectionPool.release(conn);
                    return false;
                }

                typename Epoller<T>::InsertConnectionMapResult result;

                result = Epoller<T>::connectionMap.insert({cur,conn});
                if(!result.second){
                    LOG_ERROR(EpollerChan,"connectionMap already have key");
                    conn->Close();
                    Epoller<T>::connectionPool.release(conn);
                    return false;
                }
                Epoller<T>::currentConnectCount++;
                mapIterator = result.first;
                LOG_TRACE(EpollerChan,std::to_string(cur) + " new connection added");
            }
            return true;
        }
    };

} // totoro

#endif //TOTORO_FORWARDEPOLLER_H
