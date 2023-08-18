#ifndef TOTOROSERVER_ASYNCLOGGER_H
#define TOTOROSERVER_ASYNCLOGGER_H

#include <string>
#include <thread>
#include <unordered_map>
#include "Channel.h"

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
        Semaphore logSem                        {};
        std::thread workThread                  {};
        bool isStop                             {false};

        explicit AsyncLogger(std::string  filePath);
        void writeLog(Msg& msg);
    public:
        ~AsyncLogger();
        static std::unordered_map<const char*,LEVEL> str2Level;
        static LEVEL logLevel;
        static void Work(AsyncLogger*);
        static void Log(LEVEL level, int line, std::string data, const char * filename, const char * function);
    };
#define PRIVATE_LOG(level,msg) do {                                         \
    if(level >= totoro::AsyncLogger::logLevel){                             \
        totoro::AsyncLogger::Log(level,__LINE__,msg,__FILE__,__FUNCTION__); \
    }                                                                       \
}while(0)
#ifndef NO_ASYNC_LOG
#define LOG_TRACE(msg) PRIVATE_LOG(totoro::LEVEL::L_TRACE,msg)
#define LOG_DEBUG(msg) PRIVATE_LOG(totoro::LEVEL::L_DEBUG,msg)
#define LOG_INFO(msg) PRIVATE_LOG(totoro::LEVEL::L_INFO,msg)
#define LOG_WARN(msg) PRIVATE_LOG(totoro::LEVEL::L_WARN,msg)
#define LOG_ERROR(msg) PRIVATE_LOG(totoro::LEVEL::L_ERROR,msg)
#define LOG_FATAL(msg) PRIVATE_LOG(totoro::LEVEL::L_FATAL,msg)
#else
#define LOG_TRACE(msg)
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARN(msg)
#define LOG_ERROR(msg)
#define LOG_FATAL(msg)
#endif

} // totoro

#endif //TOTOROSERVER_ASYNCLOGGER_H
