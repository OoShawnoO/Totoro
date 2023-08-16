#ifndef TOTOROSERVER_CHANNEL_H
#define TOTOROSERVER_CHANNEL_H

#include <mutex>
#include <condition_variable>

namespace totoro {
    class Semaphore {
    public:
        explicit Semaphore(long count = 0) : count(count) {}
        //V操作，唤醒
        void signal()
        {
            std::unique_lock<std::mutex> unique(mt);
            ++count;
            if (count <= 0)
                cond.notify_one();
        }
        //P操作，阻塞
        void wait()
        {
            std::unique_lock<std::mutex> unique(mt);
            --count;
            if (count < 0)
                cond.wait(unique);
        }

    private:
        std::mutex mt;
        std::condition_variable cond;
        long count;
    };
    class Channel {

    };

} // totoro

#endif //TOTOROSERVER_CHANNEL_H
