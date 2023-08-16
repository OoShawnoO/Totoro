#ifndef TOTOROSERVER_ASYNCLOGGER_H
#define TOTOROSERVER_ASYNCLOGGER_H

namespace totoro {

    enum LEVEL {
        L_TRACE,    /* 跟踪 */
        L_DEBUG,    /* 调试 */
        L_INFO,     /* 信息 */
        L_WARN,     /* 警告 */
        L_ERROR,    /* 错误 */
        L_FATAL     /* 致命错误 */
    };

    class AsyncLogger {
        struct Msg {
            LEVEL level;
            int line;
            const char* msg;
            const char* filename;
            const char* function;
            char time[20];
        };
    };

} // totoro

#endif //TOTOROSERVER_ASYNCLOGGER_H
