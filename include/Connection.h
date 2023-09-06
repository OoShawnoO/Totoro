#ifndef TOTOROSERVER_CONNECTION_H
#define TOTOROSERVER_CONNECTION_H

#include "Socket.h"     /* Socket */
#include "Configure.h"  /* Configure */

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
        void Init(TCPSocket& tcpSocket,EpollID epollId,bool edgeTriggle = false,bool oneShot = true);
        virtual int Init(SocketID sock,sockaddr_in myAddr,sockaddr_in destAddr,EpollID epollId,bool edgeTriggle = false,bool oneShot = true);
        void Run();
        void RegisterNextEvent(SocketID sock,Status nextStatus,bool isMod);
        void SetWorkSock(SocketID sock);
        int Close() override;
    protected:
        SocketID workSock               {BAD_FILE_DESCRIPTOR};
        EpollID epollId                 {BAD_FILE_DESCRIPTOR};
        std::string data                {};
        Status status                   {None};
        bool edgeTriggle                {false};
        bool oneShot                    {true};
        virtual int ReadCallback();
        virtual int AfterReadCallback();
        virtual int WriteCallback();
        virtual int AfterWriteCallback();
        virtual int EpollMod(SocketID sock,uint32_t ev);
        virtual int EpollAdd(SocketID sock);
        virtual int EpollDel(SocketID sock);
    };

} // totoro

#endif //TOTOROSERVER_CONNECTION_H
