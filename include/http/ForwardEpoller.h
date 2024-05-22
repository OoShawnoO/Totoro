#ifndef TOTORO_FORWARD_EPOLLER_H
#define TOTORO_FORWARD_EPOLLER_H

#include "core/Epoller.h"

namespace totoro {
    template<class T = Connection>
    class ForwardEpoller : public Epoller<T> {

        ForwardCandidateMap forward_map{};

        bool getMapIterator(SOCKET cur, typename Epoller<T>::ConnectionMapIterator &mapIterator) override {
            if ((mapIterator = Epoller<T>::connectionMap.find(cur)) == Epoller<T>::connectionMap.end()) {

                typename Epoller<T>::ForwardCandidateMapIterator forwardIter;
                if ((Epoller<T>::forwardIter = Epoller<T>::forwardCandidateMap.find(cur)) !=
                    Epoller<T>::forwardCandidateMap.end()) {
                    if ((mapIterator = Epoller<T>::connectionMap.find(forwardIter->second)) ==
                        Epoller<T>::connectionMap.end()) {
                        MOLE_ERROR(EpollerChan, "not found connection for forward socket");
                        return false;
                    }
                    if (!Epoller<T>::connectionMap.insert({cur, mapIterator->second}).second) {
                        MOLE_ERROR(EpollerChan, "can't insert forward socket to connection map");
                        return false;
                    }
                    Epoller<T>::forwardCandidateMap.erase(forwardIter);
                    return true;
                }

                std::shared_ptr<T> conn;
                sockaddr_in myAddr{}, destAddr{};
                socklen_t addrLen = sizeof(myAddr);

                if (!Epoller<T>::connectionPool.acquire(conn)) {
                    close(cur);
                    MOLE_ERROR(EpollerChan, "connection pool acquire failed");
                    return false;
                }

                if (getsockname(cur, (sockaddr *) &myAddr, &addrLen) < 0) {
                    close(cur);
                    MOLE_ERROR(EpollerChan, "can't get sock my address");
                    return false;
                }
                if (getpeername(cur, (sockaddr *) &destAddr, &addrLen) < 0) {
                    close(cur);
                    MOLE_ERROR(EpollerChan, "can't get sock dest address");
                    return false;
                }

                Epoller<T>::connectionInitParameter.sock = cur;
                Epoller<T>::connectionInitParameter.myAddr = myAddr;
                Epoller<T>::connectionInitParameter.destAddr = destAddr;
                int ret = conn->Init(Epoller<T>::connectionInitParameter);
                if (ret < 0) {
                    conn->Close();
                    Epoller<T>::connectionPool.release(conn);
                    return false;
                }

                typename Epoller<T>::InsertConnectionMapResult result;

                result = Epoller<T>::connectionMap.insert({cur, conn});
                if (!result.second) {
                    MOLE_ERROR(EpollerChan, "connectionMap already have key");
                    conn->Close();
                    Epoller<T>::connectionPool.release(conn);
                    return false;
                }
                Epoller<T>::currentConnectCount++;
                mapIterator = result.first;
                MOLE_TRACE(EpollerChan, std::to_string(cur) + " new connection added");
            }
            return true;
        }

        void initConnectionParameter() override {
            Epoller<T>::connectionInitParameter.epollId = Epoller<T>::id;
            Epoller<T>::connectionInitParameter.oneshot = Epoller<T>::oneshot;
            Epoller<T>::connectionInitParameter.edge_trigger = Epoller<T>::edge_tigger;
            Epoller<T>::connectionInitParameter.forward_map = &forward_map;
        }

    public:
        void DelConnection(std::shared_ptr<T> &conn) override {
            if (EpollDel(conn->Sock()) < 0) {
                MOLE_ERROR(EpollerChan, strerror(errno));
            }
            int sock = conn->Sock();
            int ret = conn->Close();
            if (ret >= 0) {
                Epoller<T>::EpollDel(ret);
                Epoller<T>::connectionMap.erase(ret);
            }
            Epoller<T>::connectionPool.release(conn);
            Epoller<T>::connectionMap.erase(sock);
            Epoller<T>::currentConnectCount--;
        }
    };

} // totoro

#endif //TOTORO_FORWARD_EPOLLER_H
