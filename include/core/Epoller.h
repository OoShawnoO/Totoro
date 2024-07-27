#ifndef TOTORO_EPOLLER_H
#define TOTORO_EPOLLER_H

#include <sys/epoll.h>      /* epoll */
#include <fcntl.h>          /* fcntl */
#include "Connection.h"     /* Connection */
#include "utils/Pool.h"           /* Pool */
#include "utils/ThreadPool.h"     /* ThreadPool */

static const std::string EpollerChan = "Totoro";
namespace totoro {
    /**
     * @brief 负责连接资源获取、事件监听与分发 \n Responsible for connecting resource acquisition, event monitoring, and distribution
     * @tparam T Connection类或继承自Connection类 \n Connection or derived from Connection.
     */
    template<class T = Connection>
    class Epoller {
    protected:
        using ConnectionMap = std::unordered_map<SOCKET, std::shared_ptr<T>>;
        using ConnectionMapIterator = typename ConnectionMap::iterator;
        using ForwardCandidateMapIterator = typename ForwardCandidateMap::iterator;
        using InsertConnectionMapResult = std::pair<ConnectionMapIterator, bool>;

        // 停止 / stop
        volatile bool &stop;
        // epoll 边缘触发 / epoll edge trigger
        bool edge_tigger{false};
        // epoll one-shot
        bool oneshot{true};
        // 非阻塞 / none block
        bool none_block{true};
        // epoll实例文件描述符 / epoll file descriptor
        EpollID id{BAD_SOCKET};
        // epoll事件池 / epoll events pool
        epoll_event events[4096]{};
        // 文件描述符 -> 连接 哈希表 / file descriptor -> connection hash map
        ConnectionMap connectionMap{};
        // 连接池 / connection pool
        Pool<T> connection_pool{1024};
        // 线程池 / thread pool
        ThreadPool<T> thread_pool{4, 8196};
        // 当前连接数 / current connect count
        std::atomic<int> current_connect_count{0};
        // 连接初始化参数 / connection initialize parameter
        ConnectionInitParameter connectionInitParameter{};


        int NoneBlock(SOCKET socketId) {
            int option = fcntl(socketId, F_GETFL);
            int newOption = option | O_NONBLOCK;
            return fcntl(socketId, F_SETFL, newOption);
        }

        int EpollAdd(SOCKET socketId, bool isListen = false) {
            epoll_event ev{};
            ev.data.fd = socketId;
            ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
            ev.events = (edge_tigger ? ev.events | EPOLLET : ev.events);
            ev.events = (oneshot && !isListen ? ev.events | EPOLLONESHOT : ev.events);
            if (none_block) {
                if (NoneBlock(socketId) < 0) {
                    MOLE_ERROR(EpollerChan, "set socket none block failed:" + std::string(strerror(errno)));
                    return -1;
                }
            }
            return epoll_ctl(id, EPOLL_CTL_ADD, socketId, &ev);
        }

        int EpollMod(SOCKET socketId, uint32_t ev) {
            epoll_event event{};
            event.data.fd = socketId;
            event.events = ev | EPOLLRDHUP | EPOLLHUP;
            event.events = edge_tigger ? event.events | EPOLLET : event.events;
            event.events = oneshot ? event.events | EPOLLONESHOT : event.events;
            return epoll_ctl(id, EPOLL_CTL_MOD, socketId, &event);
        }

        int EpollDel(SOCKET socketId) {
            return epoll_ctl(id, EPOLL_CTL_DEL, socketId, nullptr);
        }

        virtual bool getMapIterator(SOCKET cur, ConnectionMapIterator &mapIterator) {
            if ((mapIterator = connectionMap.find(cur)) == connectionMap.end()) {

                std::shared_ptr<T> conn;
                sockaddr_in myAddr{}, destAddr{};
                socklen_t addrLen = sizeof(myAddr);

                if (!connection_pool.acquire(conn)) {
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

                connectionInitParameter.sock = cur;
                connectionInitParameter.local_address = &myAddr;
                connectionInitParameter.destination_address = &destAddr;
                int ret = conn->Init(connectionInitParameter);
                if (ret < 0) {
                    conn->Close();
                    connection_pool.release(conn);
                    return false;
                }

                InsertConnectionMapResult result;

                result = connectionMap.insert({cur, conn});
                if (!result.second) {
                    MOLE_ERROR(EpollerChan, "connectionMap already have key");
                    conn->Close();
                    connection_pool.release(conn);
                    return false;
                }
                current_connect_count++;
                mapIterator = result.first;
                MOLE_TRACE(EpollerChan, std::to_string(cur) + " new connection added");
            }
            return true;
        }

        virtual void initConnectionParameter() {
            connectionInitParameter.epoll_id = id;
            connectionInitParameter.oneshot = oneshot;
            connectionInitParameter.edge_trigger = edge_tigger;
        }

        void poll(int timeOut) {
            int ret, index, cur;
            ConnectionMapIterator mapIterator;

            initConnectionParameter();

            while (!stop) {
                if ((ret = epoll_wait(id, events, 4096, timeOut)) < 0) {
                    if (errno == EINTR) continue;
                    MOLE_ERROR(EpollerChan, "epoll wait failed");
                    exit(-1);
                }

                for (index = 0; index < ret; index++) {
                    cur = events[index].data.fd;
                    if (!getMapIterator(cur, mapIterator)) continue;
                    auto &connection = mapIterator->second;
                    auto event = events[index].events;
                    if (event & EPOLLRDHUP) {
                        MOLE_TRACE(EpollerChan, std::to_string(cur) + " connection close");
                        DelConnection(connection);
                    } else if (event & EPOLLERR) {
                        MOLE_ERROR(EpollerChan, "epoll error");
                        DelConnection(connection);
                    } else if (event & EPOLLIN) {
                        connection->RegisterNextEvent(cur, Connection::Read, false);
                        thread_pool.Add(connection);
                    } else if (event & EPOLLOUT) {
                        connection->RegisterNextEvent(cur, Connection::Write, false);
                        thread_pool.Add(connection);
                    } else {
                        MOLE_ERROR(EpollerChan, "unknown error");
                        DelConnection(connection);
                    }
                }
            }
        }

    public:
        explicit Epoller(
                volatile bool &_stop,
                bool _et = false,
                bool _oneshot = true,
                bool _none_block = true
        ) : stop(_stop), edge_tigger(_et), oneshot(_oneshot), none_block(_none_block) {
            if ((id = epoll_create(1234)) < 0) {
                MOLE_ERROR(EpollerChan, "create epoll failed");
                exit(-1);
            }
            connectionMap.reserve(4096);
        }

        ~Epoller() {
            close(id);
        }

        bool AddConnection(TcpSocket &tcpSocket) {
            if (EpollAdd(tcpSocket.Sock()) < 0) {
                MOLE_ERROR(EpollerChan, "add connection failed");
                return false;
            }
            return true;
        };

        virtual void DelConnection(std::shared_ptr<T> &conn) {
            if (EpollDel(conn->Sock()) < 0) {
                MOLE_ERROR(EpollerChan, strerror(errno));
            }
            int sock = conn->Sock();
            conn->Close();
            connection_pool.release(conn);
            connectionMap.erase(sock);
            current_connect_count--;
        };

        static void Poll(Epoller<T> *_epoller, int timeOut) {
            auto &epoller = *_epoller;
            epoller.poll(timeOut);
        }

        int CurrentConnectionCount() const {
            return current_connect_count;
        }
    };

} // totoro

#endif //TOTORO_EPOLLER_H
