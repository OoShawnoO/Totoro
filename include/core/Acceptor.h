#ifndef TOTORO_ACCEPTOR_H
#define TOTORO_ACCEPTOR_H

#include <cstring>
#include <climits>
#include <csignal>

#include "Epoller.h"

static const std::string AcceptorChan = "Totoro";
namespace totoro {
    /**
     * @brief 负责监听新连接并分发给epoller \n response for accept new connection and dispatch new connection to epoller
     * @tparam T Connection类或继承自Connection类 \n Connection or derived from Connection.
     */
    template<typename E>
    class Acceptor {
    protected:
        volatile bool stop{false};
        std::string ip{};
        unsigned short port{};
        TcpListener listen_socket;
        std::deque<E> reactors{};

        /* 轮询式reactor处理新连接 / Use round-robin algorithm to process new connection */
        E &RoundRobin() {
            static int index = 0;
            return reactors[++index % reactors.size()];
        }

        /* 使用当前最小连接数reactor处理新连接 / Use current least connection count reactor to process new connection */
        E &Minimum() {
            int minCount = INT_MAX;
            E *cur = nullptr;
            for (auto &reactor: reactors) {
                if (reactor.CurrentConnectionCount() < minCount)
                    cur = &reactor;
            }
            return *cur;
        }

    public:
        Acceptor(
                std::string ip_,
                unsigned short port_,
                int epoll_timeout = -1,
                int reactor_count = 1,
                bool edge_trigger = false,
                bool oneshot = true,
                bool none_block = true
        ) : listen_socket(ip_, port_) {
            ip = std::move(ip_);
            port = port_;

            if (reactor_count <= 0) {
                MOLE_FATAL("Epoller", "reactor count must > 0");
                exit(-1);
            }
            for (int i = 0; i < reactor_count; i++) {
                reactors.emplace_back(stop, edge_trigger, oneshot, none_block);
            }

            if (!listen_socket.Bind()) {
                MOLE_ERROR("Epoller", "bind listen socket failed");
                exit(-1);
            }
            if (!listen_socket.Listen()) {
                MOLE_ERROR("Epoller", "listen socket failed");
                exit(-1);
            }

            for (auto &reactor: reactors) {
                std::thread(E::Poll, &reactor, epoll_timeout).detach();
            }
        }

        explicit Acceptor(const Json &config)
                : listen_socket(config["ip"], config["port"]) {
            ip = config["ip"];
            port = config["port"];
            int epoll_timeout = config["epoll-timeout"];
            int reactor_count = config["reactor-count"];
            bool edge_trigger = config["edge-trigger"];
            bool oneshot = config["one-shot"];
            bool none_block = config["none-block"];

            if (reactor_count <= 0) {
                MOLE_FATAL("Epoller", "reactor count must > 0");
                exit(-1);
            }
            for (int i = 0; i < reactor_count; i++) {
                reactors.emplace_back(stop, edge_trigger, oneshot, none_block);
            }

            if (!listen_socket.Bind()) { exit(-1); }
            if (!listen_socket.Listen()) { exit(-1); }

            for (auto &reactor: reactors) {
                std::thread(E::Poll, &reactor, epoll_timeout).detach();
            }
        }

        ~Acceptor() {
            stop = true;
        }

        void Close() {
            stop = true;
        }

        void Run() {
            TcpSocket tcpSocket;

            int option = fcntl(listen_socket.Sock(), F_GETFL);
            int newOption = option | O_NONBLOCK;
            fcntl(listen_socket.Sock(), F_SETFL, newOption);

            while (!stop) {
                if (!listen_socket.Accept(tcpSocket)) {
                    if(errno == EWOULDBLOCK || errno == EAGAIN) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }
                    exit(-1);
                }

                Minimum().AddConnection(tcpSocket);
            }
        }
    };

} // totoro

#endif //TOTORO_ACCEPTOR_H
