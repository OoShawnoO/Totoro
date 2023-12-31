#ifndef TOTOROSERVER_ASYNCLOGGER_H
#define TOTOROSERVER_ASYNCLOGGER_H

#include <string>           /* string */
#include <thread>           /* thread */
#include <unordered_map>    /* unordered_map */
#include "Channel.h"        /* Semaphore Channel */

namespace totoro {

    enum LEVEL {
        L_TRACE,    /* 跟踪 */
        L_DEBUG,    /* 调试 */
        L_INFO,     /* 信息 */
        L_WARN,     /* 警告 */
        L_ERROR,    /* 错误 */
        L_FATAL     /* 致命错误 */
    };
#define CANCEL_COLOR_SCHEME "\033[0m"
#define LOG_BUF_SIZE 4096
    class AsyncLogger {
        struct Msg {
            std::string channel;
            LEVEL level;
            int line;
            std::string msg;
            const char* filename;
            const char* function;
            char time[20];
        };
        const std::unordered_map<LEVEL,const char*> levelMap{
                {L_TRACE,"TRACE"},{L_DEBUG,"DEBUG"},{L_INFO,"INFO"},{L_WARN,"WARN"},{L_ERROR,"ERROR"},{L_FATAL,"FATAL"}
        };
        const std::unordered_map<LEVEL,const char*> colorScheme{
                {L_TRACE,"\033[36m"},{L_DEBUG,"\033[34m"},{L_INFO,"\033[32m"},{L_WARN,"\033[33m"},{L_ERROR,"\033[31m"},{L_FATAL,"\033[31m"}
        };

        std::string filePath                    {};
        char buffer[LOG_BUF_SIZE]               = {0};
        char writeBuffer[LOG_BUF_SIZE * 10]     = {0};
        int writeCursor                         {0};
        FILE* fp                                {nullptr};
        Channel<Msg> logChan                    {};
        counting_semaphore<0> logSem               {0};
        std::thread workThread                  {};
        bool isStop                             {false};

        explicit AsyncLogger(std::string  filePath);
        void writeLog(Msg& msg);
    public:
        ~AsyncLogger();
        const static std::unordered_map<std::string,LEVEL> str2Level;
        static std::unordered_map<std::string,LEVEL> channelLevel;
        static void Work(AsyncLogger*);
        static void Log(LEVEL level,const std::string& channel, int line, std::string data, const char * filename, const char * function);
    };
#define PRIVATE_LOG(level,channel,msg) do {                                         \
    if(level >= totoro::AsyncLogger::channelLevel[channel]){                             \
        totoro::AsyncLogger::Log(level,channel,__LINE__,msg,__FILE__,__FUNCTION__); \
    }                                                                       \
}while(0)
#ifndef NO_ASYNC_LOG
#define LOG_TRACE(channel,msg) PRIVATE_LOG(totoro::LEVEL::L_TRACE,channel,msg)
#define LOG_DEBUG(channel,msg) PRIVATE_LOG(totoro::LEVEL::L_DEBUG,channel,msg)
#define LOG_INFO(channel,msg) PRIVATE_LOG(totoro::LEVEL::L_INFO,channel,msg)
#define LOG_WARN(channel,msg) PRIVATE_LOG(totoro::LEVEL::L_WARN,channel,msg)
#define LOG_ERROR(channel,msg) PRIVATE_LOG(totoro::LEVEL::L_ERROR,channel,msg)
#define LOG_FATAL(channel,msg) PRIVATE_LOG(totoro::LEVEL::L_FATAL,channel,msg)
#else
    #define LOG_TRACE(msg)
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARN(msg)
#define LOG_ERROR(msg)
#define LOG_FATAL(msg)
#endif

} // sstc

#endif //TOTOROSERVER_ASYNCLOGGER_H
