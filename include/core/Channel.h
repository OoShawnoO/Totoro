#ifndef TOTORO_CHANNEL_H
#define TOTORO_CHANNEL_H

#include <mutex>                /* mutex */
#include <condition_variable>   /* condition_variable */
#include <deque>                /* deque */

namespace totoro {
    template<size_t _least_max_value = SIZE_MAX>
    class counting_semaphore {
        static_assert(_least_max_value >= 0,"least max value must more than 0");
        static_assert(_least_max_value <= SIZE_MAX,"least max value must lower than size_t max");
    public:
        counting_semaphore() = default;
        explicit counting_semaphore(size_t desire) : count(desire){};
        void acquire(){
            std::unique_lock<std::mutex> lock(mt);
            cond.wait(lock,[&]{return count > 0;});
            --count;
        }
        void release(){
            std::unique_lock<std::mutex> lock(mt);
            ++count;
            cond.notify_one();
        }
    private:
        std::mutex mt;
        std::condition_variable cond;
        size_t count{0};
    };
    template<typename T>
    struct Channel {
        std::mutex mtx;
        std::deque<T> data;
        virtual void push(T&& d){
            std::lock_guard<std::mutex> lock(mtx);
            data.emplace_back(std::move(d));
        }
        virtual void push(T& ref){
            std::lock_guard<std::mutex> lock(mtx);
            data.emplace_back(ref);
        }
        virtual bool pop(T& d){
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

    template<typename T>
    struct BlockChannel : Channel<T> {
        counting_semaphore<0> sem;
        void push(T&& d) override{
            std::lock_guard<std::mutex> lock(Channel<T>::mtx);
            Channel<T>::data.emplace_back(std::move(d));
            sem.release();
        }
        void push(T& ref) override{
            std::lock_guard<std::mutex> lock(Channel<T>::mtx);
            Channel<T>::data.emplace_back(ref);
            sem.release();
        }
        bool pop(T& d) override{
            sem.acquire();
            std::lock_guard<std::mutex> lock(Channel<T>::mtx);
            if(Channel<T>::data.empty()) return false;
            d = Channel<T>::data.front();
            Channel<T>::data.pop_front();
            return true;
        }
        size_t size(){
            std::lock_guard<std::mutex> lock(Channel<T>::mtx);
            return Channel<T>::data.size();
        }
    };
} // totoro

#endif //TOTORO_CHANNEL_H
