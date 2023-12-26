//
// Created by dferraro on 23/12/23.
//

#ifndef AUTONOMICFARM_WORKERPOOL_HPP
#define AUTONOMICFARM_WORKERPOOL_HPP


#include <vector>
#include "ThreadedNode.hpp"
#include "trace.hpp"

template <typename InputType, typename OutputType>
class WorkerPool : public Node<InputType> {
public:
    typedef std::function<OutputType(InputType&)> WorkerFunType;

    WorkerPool(const WorkerFunType &workerfun, size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers, const ThreadedNode<OutputType>::OnValueFun &sendOutFun);

    void setWorkersNumber(size_t required_num_workers);
    void run() override;
    void wait() override;
    void notify_eos() override;
    void send(InputType &value) override;

private:
    std::vector<ThreadedNode<InputType>> workers;
    std::atomic<size_t> num_workers;
    size_t min_num_workers;
    size_t max_num_workers;
    size_t worker_index = 0;
};

template<typename InputType, typename OutputType>
WorkerPool<InputType, OutputType>::WorkerPool(const WorkerFunType &workerfun, size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers, const ThreadedNode<OutputType>::OnValueFun &sendOutFun)
: min_num_workers(minNumWorkers), max_num_workers(maxNumWorkers), num_workers(num_workers) {
    workers.reserve(max_num_workers);
    for (int i = 0; i < max_num_workers; ++i) {
        TRACEF("Init worker %d/%lu", (i+1), max_num_workers);
        workers.emplace_back([workerfun, sendOutFun](InputType& value) {
            OutputType res = workerfun(value);
            sendOutFun(res);
        });
    }
}

template<typename InputType, typename OutputType>
void WorkerPool<InputType, OutputType>::setWorkersNumber(size_t required_num_workers) {
    if (required_num_workers < min_num_workers || required_num_workers > max_num_workers) return;
    
    num_workers = required_num_workers;
    for (size_t i = required_num_workers + 1; i < workers.size(); ++i) {
        auto worker = workers[i];
        worker.inputStream.eos();
    }
}

template<typename InputType, typename OutputType>
void WorkerPool<InputType, OutputType>::run() {
    for (auto& worker: workers) {
        worker.run();
    }
}

template<typename InputType, typename OutputType>
void WorkerPool<InputType, OutputType>::wait() {
    for (auto& worker: workers) {
        worker.wait();
    }
}

template<typename InputType, typename OutputType>
void WorkerPool<InputType, OutputType>::notify_eos() {
    for (auto& worker: workers) {
        worker.notify_eos();
    }
}

template<typename InputType, typename OutputType>
void WorkerPool<InputType, OutputType>::send(InputType &value) {
    TRACEF("Send to worker %lu", worker_index);
    workers[worker_index].send(value);
    worker_index = (worker_index+1) % max_num_workers;
}

#endif //AUTONOMICFARM_WORKERPOOL_HPP
