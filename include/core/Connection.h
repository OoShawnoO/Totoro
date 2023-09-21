#ifndef TOTOROSERVER_CONNECTION_H
#define TOTOROSERVER_CONNECTION_H

#include "core/Socket.h"     /* Socket */
#include "core/Configure.h"  /* Configure */
#include "core/IPFilter.h"

#include <utility>           /* pair */
#include <functional>        /* function */


namespace totoro {
    using EpollID = int;
    using SocketID = int;
    using ForwardCandidateMap = std::unordered_map<SocketID,SocketID>;
    /**
     * @brief 负责初始化连接 \n Response connection init
     */
    struct ConnectionInitParameter{
        // 客户端TCP连接Socket /  client tcp connection socket
        SocketID sock                               {BAD_FILE_DESCRIPTOR};
        // 服务器端地址        /  server address
        sockaddr_in myAddr                          {};
        // 客户端地址          /  client address
        sockaddr_in destAddr                        {};
        // 从属epoll实例id     /  dependent epoll instance ID
        EpollID epollId                             {BAD_FILE_DESCRIPTOR};
        // 边缘触发            /  edge triggle
        bool edgeTriggle                            {false};
        // one shot            /  one shot
        bool oneShot                                {true};
        // ip 过滤器           /  ip filter
        IPFilter* filter                            {nullptr};
        // 转发候选表          / forward candidate map
        ForwardCandidateMap* forwardCandidateMap    {nullptr};
        // 关闭队列            / close queue
        Channel<SocketID>* closeChan                {nullptr};
    };

    /**
     * @brief 负责连接相关事务的处理 \n Response connection transaction
     */
    class Connection : virtual public TCPSocket{
    public:
        enum Status {
            None,Read,Write,AfterRead,AfterWrite,Error
        };
        enum CallbackReturnType {
            FAILED = -1,AGAIN,SUCCESS,INTERRUPT,SHUTDOWN
        };
        ~Connection() override;
        /* 事件循环 / Event loop */
        void Run();
        /* 主动关闭连接 / Manual shut down connection */
        int ShutDown();
        /* 关闭连接并清除connection数据 / Close connection and clear data */
        int Close() override;
        /* 清除缓冲区数据 / Clear buffer data */
        void ClearData();
        /* 封禁IP地址 / Ban ip address */
        bool BanAddr(const std::string& banIp);
        /* 添加允许的IP地址 / Add allowed ip address */
        bool AllowAddr(const std::string& allowIp);
        /* 设置工作socket / Set work socket */
        void SetWorkSock(SocketID sock);
        /* 为socket注册下一个事件 / Register next event for socket */
        void RegisterNextEvent(SocketID sock,Status nextStatus,bool isMod);
        /* 初始化 / Initialize */
        virtual int Init(const ConnectionInitParameter& connectionInitParameter);
    protected:
        // 工作socket / Working socket
        SocketID workSock                           {BAD_FILE_DESCRIPTOR};
        // EPOLL实例id / Epoll id
        EpollID epollId                             {BAD_FILE_DESCRIPTOR};
        // 缓冲区数据 / Buffer data
        std::string data                            {};
        // 事件循环状态 / Event loop status
        Status status                               {None};
        // 上次事件状态 / Last event status
        Status lastStatus                           {None};
        // 边界触发 / Edge triggle
        bool edgeTriggle                            {false};
        // Oneshot 模式 / Oneshot model
        bool oneShot                                {true};
        // IP过滤器 / IP Filter
        IPFilter* filter                            {nullptr};
        // 转发候选表 / Forward candidate map
        ForwardCandidateMap* forwardCandidateMap    {nullptr};
        // 读事件回调 / Read event callback
        virtual CallbackReturnType ReadCallback();
        // 读事件后回调 / After read event callback
        virtual CallbackReturnType AfterReadCallback();
        // 写事件回调 / Write event callback
        virtual CallbackReturnType WriteCallback();
        // 写事件后回调 / After write event callback
        virtual CallbackReturnType AfterWriteCallback();
        int EpollMod(SocketID sock,uint32_t ev) const;
        int EpollAdd(SocketID sock) const;
        int EpollDel(SocketID sock) const;
    };

} // totoro

#endif //TOTOROSERVER_CONNECTION_H
