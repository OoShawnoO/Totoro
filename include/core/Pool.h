#ifndef TOTORO_POOL_H
#define TOTORO_POOL_H

#include "Channel.h"        /* Channel */

namespace totoro {
    template<class T>
    class Pool {
        Channel<std::shared_ptr<T>> pool    {};
        size_t size                         {1024};
        size_t maxSize                      {8196};
    public:
        explicit Pool(size_t _size,size_t _maxSize = 8196){
            size = _size;
            maxSize = _maxSize;
            for(size_t i=0; i<_size; i++) pool.push(std::make_shared<T>());
        }
        Pool(const Pool<T>&) = delete;
        Pool<T>& operator=(const Pool<T>&) = delete;
        bool acquire(std::shared_ptr<T>& ptr){
            if(!pool.pop(ptr)){
                if(size++ > maxSize) { return false; }
                ptr = std::make_shared<T>();
            }
            return true;
        }
        void release(std::shared_ptr<T>& ptr){
            pool.push(ptr);
            ptr.reset();
        }
    };

} // totoro

#endif //TOTORO_POOL_H
