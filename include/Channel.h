#ifndef TOTOROSERVER_CHANNEL_H
#define TOTOROSERVER_CHANNEL_H

#include <mutex>
#include <condition_variable>
#include <deque>

namespace totoro {
    class Semaphore {
    public:
        explicit Semaphore(long count = 0);
        //V操作，唤醒
        void signal();
        //P操作，阻塞
        void wait();
    private:
        std::mutex mt;
        std::condition_variable cond;
        long count;
    };
    template<typename T>
    struct Channel {
        std::mutex mtx;
        std::deque<T> data;
        void push(T&& d){
            std::lock_guard<std::mutex> lock(mtx);
            data.emplace_back(std::move(d));
        }
        void push(T& ref){
            std::lock_guard<std::mutex> lock(mtx);
            data.emplace_back(ref);
        }
        bool pop(T& d){
            std::lock_guard<std::mutex> lock(mtx);
            if(data.empty()) return false;
            d = data.front();
            data.pop_front();
            return true;
        }
        bool empty() {
            std::lock_guard<std::mutex> lock(mtx);
            return data.empty();
        }
    };

} // totoro

#endif //TOTOROSERVER_CHANNEL_H
