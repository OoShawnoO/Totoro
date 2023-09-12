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
    };

    /**
     * @brief 负责连接相关事务的处理 \n Response connection transaction
     */
    class Connection : public TCPSocket{
    public:
        enum Status {
            None,Read,Write,AfterRead,AfterWrite,Error
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

        virtual int ReadCallback();
        virtual int AfterReadCallback();
        virtual int WriteCallback();
        virtual int AfterWriteCallback();
        int EpollMod(SocketID sock,uint32_t ev) const;
        int EpollAdd(SocketID sock) const;
        int EpollDel(SocketID sock) const;
    };

} // totoro

#endif //TOTOROSERVER_CONNECTION_H
