
#ifndef _AR_QUEUE_H_
#define _AR_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

using namespace std::chrono_literals;

namespace ar::Types {

    template <typename T>
    class QueueBase {
       public:
        virtual ~QueueBase() {};

        virtual void put(const T& x) = 0;
        virtual T take()             = 0;

        // with time out
        // @param timeout: max wait time, ms
        // @param out: reference result if take successfully
        // @return take successfully or not
        virtual bool take(int timeout, T& out) = 0;

       protected:
        std::queue<T> queue_;
        //用于限制消费者线程
        std::condition_variable m_notEmptyCv_;
    };

    template <typename T>
    class DynamicQueue : public QueueBase<T> {

       public:
        DynamicQueue(const DynamicQueue<T>&)            = delete;
        DynamicQueue& operator=(const DynamicQueue<T>&) = delete;
        ~DynamicQueue()                                 = default;

        explicit DynamicQueue<T>() : mtx_() {}

        virtual void put(const T& x) override {
            std::lock_guard<std::mutex> locker(mtx_);
            this->queue_.push(x);
            this->m_notEmptyCv_.notify_one();
        }

        virtual bool take(int timeout, T& out) override {
            std::unique_lock<std::mutex> locker(mtx_);
            this->m_notEmptyCv_.wait_for(locker, timeout * 1ms, [this]() { return !this->queue_.empty(); });

            if (this->queue_.empty()) {
                std::puts("Warninvg: trying to pop an empty queue.");
                return false;
            }

            out = this->queue_.front();
            this->queue_.pop();
            return true;
        }

        virtual T take() override {
            std::unique_lock<std::mutex> locker(mtx_);
            this->m_notEmptyCv_.wait(locker, [this] { return !this->queue_.empty(); });

            T front(this->queue_.front());
            this->queue_.pop();

            return front;
        }

       private:
        mutable std::mutex mtx_;
    };

    template <typename T>
    class BlockingBoundedQueue : public QueueBase<T> {

       public:
        BlockingBoundedQueue(const BlockingBoundedQueue<T>&)            = delete;
        BlockingBoundedQueue& operator=(const BlockingBoundedQueue<T>&) = delete;
        BlockingBoundedQueue()                                          = default;
        ~BlockingBoundedQueue()                                         = default;

        explicit BlockingBoundedQueue<T>(size_t maxSize) : mtx_(), maxSize_(maxSize) {}

        void put(const T& x) {
            std::unique_lock<std::mutex> locker(mtx_);
            this->m_notFullCv_.wait(locker, [this]() { return this->queue_.size() < maxSize_; });

            this->queue_.push(x);
            this->m_notEmptyCv_.notify_one();
        }

        T take() {
            std::unique_lock<std::mutex> locker(mtx_);
            this->m_notEmptyCv_.wait(locker, [this]() { return !this->queue_.empty(); });

            T front(this->queue_.front());
            this->queue_.pop();
            this->m_notFullCv_.notify_one();

            return front;
        }

        bool take(int timeout, T& out) {
            std::unique_lock<std::mutex> locker(mtx_);
            this->m_notEmptyCv_.wait_for(locker, timeout * 1ms, [this]() { return !this->queue_.empty(); });
            if (this->queue_.empty()) {
                return false;
            }

            out = this->queue_.front();
            this->queue_.pop();
            this->m_notFullCv_.notify_one();

            return true;
        }

        // Checking BlockingQueue status from outside
        // DO NOT use it as internal call, which will cause DEADLOCK
        bool empty() const {
            std::unique_lock<std::mutex> locker(mtx_);
            return this->queue_.empty();
        }

        size_t size() const {
            std::unique_lock<std::mutex> locker(mtx_);
            return this->queue_.size();
        }

        size_t maxSize() const { return maxSize_; }

        void setSize(size_t maxSize) { maxSize_ = maxSize; }

        bool full() const {
            std::unique_lock<std::mutex> locker(mtx_);
            return this->queue_.size() == maxSize_;
        }

       private:
        std::condition_variable m_notFullCv_;
        size_t maxSize_;
        mutable std::mutex mtx_;
    };

};  // namespace ar::Types

#endif