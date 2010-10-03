// Written by Anthony Williams, 2008
// Public domain based on his comment:
// "Yes, you can just copy the code presented here and use it for whatever you 
// like. There won't be any licensing issues. I'm glad you find it helpful."
// Reference:
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html

#ifndef _CONCURRENT_QUEUE_H_
#define _CONCURRENT_QUEUE_H_

#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

template<typename Data>
class ConcurrentQueue
{
    private:
        std::queue<Data> queue_;
        mutable boost::mutex mutex_;
        boost::condition_variable condition_;
    public:
        ConcurrentQueue() : queue_(), mutex_(), condition_()
    {}

        void push(Data const& data)
        {
            boost::mutex::scoped_lock lock(mutex_);
            queue_.push(data);
            lock.unlock();
            condition_.notify_one();
        }

        bool empty() const
        {
            boost::mutex::scoped_lock lock(mutex_);
            return queue_.empty();
        }

        bool try_pop(Data& popped_value)
        {
            boost::mutex::scoped_lock lock(mutex_);
            if(queue_.empty())
            {
                return false;
            }

            popped_value = queue_.front();
            queue_.pop();
            return true;
        }

        void wait_and_pop(Data& popped_value)
        {
            boost::mutex::scoped_lock lock(mutex_);
            while(queue_.empty())
            {
                condition_.wait(lock);
            }

            popped_value = queue_.front();
            queue_.pop();
        }
};

#endif // _CONCURRENT_QUEUE_H_
