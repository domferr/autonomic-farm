#ifndef AUTONOMIC_FARM_FARM_HPP
#define AUTONOMIC_FARM_FARM_HPP

#include "Node.hpp"
#include "ThreadedNode.hpp"
#include "trace.hpp"
#include "Emitter.hpp"
#include "WorkerPool.hpp"

template <typename InputType, typename OutputType>
class Farm : public Node<InputType> {
public:
    typedef std::function<OutputType(InputType&)> WorkerFunType;

    explicit Farm(size_t num_workers, const WorkerFunType &fun, const ThreadedNode<OutputType>::OnValueFun &sendOutFun);
    explicit Farm(size_t num_workers, const WorkerFunType &fun, ThreadedNode<OutputType>* newGatherer);

    void run() override;
    void wait() override;
    void notify_eos() override;
    void send(InputType& value) override;

    virtual ~Farm();

protected:
    WorkerPool<InputType, OutputType> workers_pool;
    ThreadedNode<OutputType>* gatherer;
};

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::Farm(size_t num_workers, const WorkerFunType &fun, ThreadedNode<OutputType>* newGatherer)
: gatherer(newGatherer), workers_pool(fun, num_workers, 0, num_workers, [newGatherer](auto val) { newGatherer->send(val); }) {}

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::Farm(size_t num_workers, const WorkerFunType &fun, const ThreadedNode<OutputType>::OnValueFun &sendOutFun)
: Farm<InputType, OutputType>::Farm(num_workers, fun, new ThreadedNode<OutputType>(sendOutFun)) {}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::run() {
    workers_pool.run();
    gatherer->run();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::wait() {
    workers_pool.wait();
    gatherer->notify_eos();
    gatherer->wait();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::notify_eos() {
    workers_pool.notify_eos();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::send(InputType& value) {
    workers_pool.send(value);
}

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::~Farm() {
    delete gatherer;
}


#endif //AUTONOMIC_FARM_FARM_HPP
