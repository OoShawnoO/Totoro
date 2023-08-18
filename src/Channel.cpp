#include "Channel.h"

namespace totoro {

    Semaphore::Semaphore(long _count) : count(_count) {}

    void Semaphore::signal(){
        std::unique_lock<std::mutex> unique(mt);
        ++count;
        if (count <= 0)
            cond.notify_one();
    }


    void Semaphore::wait() {
        std::unique_lock<std::mutex> unique(mt);
        --count;
        if (count < 0)
            cond.wait(unique);
    }
} // totoro