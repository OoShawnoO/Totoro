#ifndef TOTOROSERVER_THREADPOOL_H
#define TOTOROSERVER_THREADPOOL_H

#include <atomic>
#include <vector>
#include "AsyncLogger.h"
#include "Channel.h"


namespace totoro {
    template<class T>
    class ThreadPool {
        using Ptr = std::shared_ptr<T>                          ;
        int threadCount                                         {8};
        int maxWorkCount                                        {8196};
        BlockChannel<Ptr> workChan                              ;
        std::atomic<bool> isStop                                {false};
        std::vector<std::shared_ptr<std::thread>> threads       {};
        static void work(ThreadPool<T>* arg){
            arg->run();
        }
        void run(){
            Ptr ptr;
            while(!isStop){
                if(!workChan.pop(ptr))  continue;
                ptr->Run();
            }
        }
    public:
        explicit ThreadPool(int _threadCount = 8,int _maxWorkCount = 8196)
        : threadCount(_threadCount),maxWorkCount(_maxWorkCount){
            if(threadCount <= 0 || maxWorkCount <= 0){
                LOG_ERROR("ThreadPool","thread count or max process count should >= 0");
                exit(-1);
            }
            for(int i=0;i<threadCount;i++){
                threads.emplace_back(std::make_shared<std::thread>(work,this));
            }
        }
        bool Add(Ptr& ptr){
            if(workChan.size() >= maxWorkCount){
                LOG_WARN("ThreadPool","thread pool overload");
                return false;
            }
            workChan.push(ptr);
            return true;
        }
    };

} // totoro

#endif //TOTOROSERVER_THREADPOOL_H
