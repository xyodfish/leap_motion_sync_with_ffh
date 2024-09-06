#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include "arQueue.h"

class Runnable {
   public:
    Runnable()  = default;
    ~Runnable() = default;

    virtual void stop() { running_ = false; }
    virtual void start() { running_ = true; }

    bool isRunning() { return running_; }

   private:
    std::atomic_bool running_;
};

template <typename T>
class TaskRunnable : public Runnable {
   public:
    explicit TaskRunnable() = default;
    virtual ~TaskRunnable() = default;

    ar::Types::BlockingBoundedQueue<T> blque_;

    void initialize(size_t queueSize) { blque_.setSize(queueSize); }
};

template <typename T>
class ProducerBase {
   public:
    explicit ProducerBase() = default;
    virtual ~ProducerBase() = default;
    virtual void run()      = 0;
};

template <typename T>
class ConsumerBase {
   public:
    explicit ConsumerBase()                                            = default;
    virtual ~ConsumerBase()                                            = default;
    virtual void setResourceUsage(std::function<void(T&)> useResource) = 0;
    virtual void setQueueWaitTime(int waitTime)                        = 0;
    virtual void run()                                                 = 0;
};

template <typename T, typename... Args>
class CreateResourceBase {
   public:
    explicit CreateResourceBase(std::function<T(Args... args)> func, Args&&... arg)
        : createResource_(func), args_(std::forward<Args&>(arg)...) {}

    virtual ~CreateResourceBase() = default;

    std::function<T(Args...)>& getFunc() { return createResource_; }

    T createResource() { return std::apply(createResource_, args_); }

   private:
    std::function<T(Args...)> createResource_;
    std::tuple<Args...> args_;
};

template <typename T, typename... Args>
class AbstractProducer : public ProducerBase<T> {
   public:
    explicit AbstractProducer(TaskRunnable<T>& taskRunnable, const CreateResourceBase<T, Args...>& crf,
                              const std::function<bool()>& stopPred)
        : taskRunnable_(taskRunnable), crf_(crf), stopPred_(stopPred) {}

    virtual ~AbstractProducer() = default;

    void setResourceCreation(const CreateResourceBase<T, Args...>& crf, std::function<bool()> stopPred) {
        this->crf_      = crf;
        this->stopPred_ = stopPred;
    }

   protected:
    CreateResourceBase<T, Args...> crf_;
    std::function<bool()> stopPred_;
    TaskRunnable<T>& taskRunnable_;

    virtual void run() = 0;  // 纯虚函数，由子类实现
};

/**
 * @brief thread type producer
 * 
 * @tparam T 
 */

template <typename T, typename... Args>
class Producer : public AbstractProducer<T, Args...> {
   public:
    using AbstractProducer<T, Args...>::AbstractProducer;  // 继承构造函数

    virtual void run() override {
        while (this->taskRunnable_.isRunning() && !this->taskRunnable_.blque_.full()) {
            if (this->stopPred_()) {
                this->taskRunnable_.stop();
                break;
            }

            T obj = this->crf_.createResource();
            this->taskRunnable_.blque_.put(obj);
        }
    }
};

/**
 * @brief trig type producer 
 * 
 * @tparam T 
 */
template <typename T, typename... Args>
class TrigProducer : public AbstractProducer<T, Args...> {
   public:
    using AbstractProducer<T, Args...>::AbstractProducer;  // 继承构造函数

    virtual void run() override {
        if (this->taskRunnable_.isRunning() && !this->taskRunnable_.blque_.full()) {
            T obj = this->crf_.createResource();
            this->taskRunnable_.blque_.put(obj);

            if (this->stopPred_()) {
                printf("the producer stopped!\n");  // todo should be replaced by a input func
                this->taskRunnable_.stop();
            }
        }
    }
};

template <typename T, typename... Args>
class Consumer : public ConsumerBase<T> {
   public:
    explicit Consumer() = default;
    explicit Consumer(TaskRunnable<T>& taskRunnable) : taskRunnable_(taskRunnable) {}
    explicit Consumer(std::function<void(T&)> useResource) : useResource_(useResource) {}

    virtual void setResourceUsage(std::function<void(T&)> useResource) override { useResource_ = useResource; }

    virtual void setQueueWaitTime(int waitTime) override { waitTime_ = waitTime; }

    virtual void run() override {
        T obj;
        while (taskRunnable_.isRunning() || !taskRunnable_.blque_.empty()) {
            if (taskRunnable_.blque_.take(waitTime_, obj)) {  // try to take
                useResource_(obj);
            } else {
                printf("Consumer: take timeout!\n");
            }
        }
    }

   private:
    std::function<void(T&)> useResource_;
    TaskRunnable<T>& taskRunnable_;
    int waitTime_ = 1000;
};

#endif