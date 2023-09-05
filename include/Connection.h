#ifndef TOTOROSERVER_CONNECTION_H
#define TOTOROSERVER_CONNECTION_H

#include "Socket.h"

#include <functional>
#include <utility>

namespace totoro {
    using EpollID = int;
    using SocketID = int;

    class Connection : public TCPSocket{
    public:
        enum Status {
            None,Read,Write,AfterRead,AfterWrite,Error
        };
        ~Connection() override;
        void Init(TCPSocket& tcpSocket,EpollID epollId,bool edgeTriggle = false,bool oneShot = true);
        void Run();
        void RegisterNextEvent(Status nextStatus,bool isMod);
        void Close() override;
    protected:
        int epollId                     {BAD_FILE_DESCRIPTOR};
        std::string data                {};
        Status status                   {None};
        bool edgeTriggle                {false};
        bool oneShot                    {true};
        virtual int ReadCallback();
        virtual int AfterReadCallback();
        virtual int WriteCallback();
        virtual int AfterWriteCallback();
        int EpollMod(uint32_t ev);
        int EpollAdd();
    };

} // totoro

#endif //TOTOROSERVER_CONNECTION_H
