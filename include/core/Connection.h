#ifndef TOTORO_CONNECTION_H
#define TOTORO_CONNECTION_H

#include "utils/Socket.h"     /* Socket */
#include "utils/Configure.h"  /* Configure */

#include <utility>           /* pair */
#include <functional>        /* function */
#include <list>


namespace totoro {

    using EpollID = int;
    using ForwardCandidateMap = std::unordered_map<SOCKET, SOCKET>;
    /**
     * @brief 负责初始化连接 \n Response connection init
     */
    struct ConnectionInitParameter {
        // 边缘触发            /  edge trigger
        bool edge_trigger{false};
        // one shot            /  one shot
        bool oneshot{true};
        // 从属epoll实例id     /  dependent epoll instance ID
        EpollID epoll_id{BAD_SOCKET};
        // 客户端TCP连接Socket /  client tcp connection socket
        SOCKET sock{BAD_SOCKET};
        // 服务器端地址        /  server address
        sockaddr_in *local_address{nullptr};
        // 客户端地址          /  client address
        sockaddr_in *destination_address{nullptr};
    };

    /**
     * @brief 负责连接相关事务的处理 \n Response connection transaction
     */
    class Connection : virtual public TcpSocket {
    public:
        enum Status {
            None, Read, Write, Shutdown
        };
    protected:
        // Oneshot 模式 / Oneshot model
        bool                                    oneshot{true};
        // 边界触发 / Edge trigger
        bool                                    edge_trigger{false};
        // EPOLL实例id / Epoll id
        EpollID                                 epoll_id{BAD_SOCKET};
        // 事件循环状态 / Event loop status
        Status                                  status{None};
        // Handler结束后需要发送的数据 / data need to send after Handler
        std::list<std::pair<std::string,bool>>  send_queue{};

        virtual void Handler();

        int EpollMod(SOCKET sock, uint32_t ev) const;

        int EpollAdd(SOCKET sock) const;

        int EpollDel(SOCKET sock) const;

    public:
        ~Connection();

        /* 事件循环 / Event loop */
        void Run();

        /* 主动关闭连接 / Manual shut down connection */
        int ShutDown();

        /* 关闭连接并清除connection数据 / Close connection and clear data */
        int Close() override;

        /* 为socket注册下一个事件 / Register next event for socket */
        void RegisterNextEvent(SOCKET sock, Status nextStatus, bool isMod);

        /* 提交需要发送的字符串 / submit string need send */
        void Submit(std::string&& data);

        /* 提交需要发送的文件 / submit file need send */
        void SubmitFile(const std::string& file_name);

        /* 初始化 / Initialize */
        virtual int Init(const ConnectionInitParameter &connectionInitParameter);
    };

} // totoro

#endif //TOTORO_CONNECTION_H
