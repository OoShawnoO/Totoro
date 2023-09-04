#include <iostream>
#include <cstring>
#include <sstream>
#include <utility>
#include "AsyncLogger.h"


namespace totoro {
    const std::unordered_map<std::string,LEVEL> AsyncLogger::str2Level{
            {"TRACE",L_TRACE},{"DEBUG",L_DEBUG},{"INFO",L_INFO},{"WARN",L_WARN},{"ERROR",L_ERROR},{"FATAL",L_FATAL}
    };
    std::unordered_map<std::string,LEVEL> AsyncLogger::channelLevel;


    AsyncLogger::AsyncLogger(std::string _filePath):filePath(std::move(_filePath)) {
        workThread = std::thread(Work,this);
    }

    AsyncLogger::~AsyncLogger(){
        isStop = true;
        logSem.signal();
        workThread.join();
        if(fp){
            if(strlen(writeBuffer) > 0){
                fwrite(writeBuffer,strlen(writeBuffer),1,fp);
                bzero(writeBuffer,LOG_BUF_SIZE * 10);
                writeCursor = 0;
            }
            fclose(fp);
            fp = nullptr;
        }
    }

    void AsyncLogger::Log(LEVEL level,const std::string& channel, int line,
                          std::string data, const char * filename, const char * function) {
        static AsyncLogger asyncLogger("");
        std::time_t current = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        static time_t last = 0;
        static char timeStr[20];
        Msg msg{channel,level,line,std::move(data),filename,function};
        if(current - last > 1){
            std::strftime(timeStr,sizeof(timeStr),"%Y-%m-%d %H:%M:%S",std::localtime(&current));
            last = current;
            strcpy(msg.time,timeStr);
        }else{
            strcpy(msg.time,timeStr);
        }
        asyncLogger.logChan.push(std::move(msg));
        asyncLogger.logSem.signal();
    }

    void AsyncLogger::Work(AsyncLogger* _asyncLogger) {
        auto& asyncLogger = *_asyncLogger;
        Msg msg{};
        if(!asyncLogger.fp && !asyncLogger.filePath.empty())
            asyncLogger.fp = fopen(asyncLogger.filePath.c_str(),"a+");
        while(!asyncLogger.isStop){
            asyncLogger.logSem.wait();
            if(!asyncLogger.logChan.pop(msg)){
                std::cerr << "[FATAL] 空队列，信号量wait成功!" << std::endl;
                exit(-1);
            }
            asyncLogger.writeLog(msg);
        }
        while(!asyncLogger.logChan.empty()){
            if(asyncLogger.logChan.pop(msg)){
                asyncLogger.writeLog(msg);
            }
        }
    }

    void AsyncLogger::writeLog(Msg& msg) {
        bzero(buffer,LOG_BUF_SIZE);
        if(msg.msg.size() + strlen(msg.filename) + strlen(msg.function) > LOG_BUF_SIZE - 30){
            std::cerr << "[ERROR] 单条日志容量大于4k" << std::endl;
            std::stringstream ss;
            ss << "[" << msg.time << "]" <<"[ " << msg.channel << " ]" <<  "[ " << levelMap.at(msg.level) << " ] " << msg.msg
               << " [" << msg.filename << ":" << msg.line << ":" << msg.function << "]\n";
            if(fp){
                fwrite(writeBuffer,strlen(writeBuffer),1,fp);
                bzero(writeBuffer,LOG_BUF_SIZE*10);
                writeCursor = 0;
                fwrite(ss.str().c_str(),ss.str().size(),1,fp);
            }
            std::cout << colorScheme.at(msg.level) << ss.str() << CANCEL_COLOR_SCHEME;
            return;
        }
        sprintf(buffer,"[%s][ %s ][ %s ] %s [%s:%d:%s]\n",
                msg.time,msg.channel.c_str(),levelMap.at(msg.level),msg.msg.c_str(),
                msg.filename,msg.line,msg.function);
        std::cout << colorScheme.at(msg.level) << buffer << CANCEL_COLOR_SCHEME;
        fflush(stdout);
        if(fp){
            strcat(writeBuffer,buffer);
            writeCursor += (int)strlen(buffer);
            if(writeCursor >= LOG_BUF_SIZE * 9){
                fwrite(writeBuffer,strlen(writeBuffer),1,fp);
                bzero(writeBuffer,LOG_BUF_SIZE * 10);
                writeCursor = 0;
            }
        }
    }


} // totoro