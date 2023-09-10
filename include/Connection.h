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
        bool BanAddr(in_addr_t addr);
        bool AllowAddr(in_addr_t addr);
        void SetWorkSock(SocketID sock);
        void RegisterNextEvent(SocketID sock,Status nextStatus,bool isMod);
        void Init(TCPSocket& tcpSocket,EpollID epollId,IPFilter* filter = nullptr,bool edgeTriggle = false,bool oneShot = true);
        virtual int Init(SocketID sock,sockaddr_in myAddr,sockaddr_in destAddr,EpollID epollId,
                         IPFilter* filter = nullptr,bool edgeTriggle = false,bool oneShot = true);
    protected:
        SocketID workSock               {BAD_FILE_DESCRIPTOR};
        EpollID epollId                 {BAD_FILE_DESCRIPTOR};
        std::string data                {};
        Status status                   {None};
        Status lastStatus               {None};
        bool edgeTriggle                {false};
        bool oneShot                    {true};
        IPFilter* filter                {nullptr};

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
