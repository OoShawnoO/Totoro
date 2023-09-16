#ifndef TOTOROSERVER_CONNECTION_H
#define TOTOROSERVER_CONNECTION_H

#include "core/Socket.h"     /* Socket */
#include "core/Configure.h"  /* Configure */
#include "core/IPFilter.h"

#include <utility>
#include <functional>


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
            FAILED = -1,AGAIN,SUCCESS,SHUTDOWN
        };
        ~Connection() override;
        void Run();
        int ShutDown();
        int Close() override;
        bool BanAddr(const std::string& banIp);
        bool AllowAddr(const std::string& allowIp);
        void SetWorkSock(SocketID sock);
        void RegisterNextEvent(SocketID sock,Status nextStatus,bool isMod);
        virtual int Init(const ConnectionInitParameter& connectionInitParameter);
    protected:
        SocketID workSock                           {BAD_FILE_DESCRIPTOR};
        EpollID epollId                             {BAD_FILE_DESCRIPTOR};
        std::string data                            {};
        Status status                               {None};
        Status lastStatus                           {None};
        bool edgeTriggle                            {false};
        bool oneShot                                {true};
        IPFilter* filter                            {nullptr};
        ForwardCandidateMap* forwardCandidateMap    {nullptr};
        Channel<SocketID>* closeChan                {nullptr};
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
