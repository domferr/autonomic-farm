#ifndef AUTONOMIC_FARM_FARM_HPP
#define AUTONOMIC_FARM_FARM_HPP

#include "Node.hpp"
#include "ThreadedNode.hpp"
#include "trace.hpp"
#include "NodePool.hpp"

template <typename InputType, typename OutputType>
class Farm : public Node<InputType> {
public:
    // function executed by a worker. It takes a reference to an input item, computes and produces a new item
    using WorkerFunType = std::function<OutputType(InputType&)>;
    // function executed by the gatherer to output all the results from the workers
    using SendOutFunType = std::function<void(OutputType&)>;

    explicit Farm(size_t num_workers, const WorkerFunType &fun, const SendOutFunType &sendOutFun);

    void run() override;
    void wait() override;
    void notify_eos() override;
    void send(InputType& value) override;

    virtual ~Farm();

protected:
    Farm() = default;

    Node<InputType>* workers_pool;
    Node<OutputType>* gatherer;
};

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::Farm(size_t num_workers, const WorkerFunType &fun, const SendOutFunType &sendOutFun) {
    gatherer = new ThreadedNode<OutputType>(sendOutFun);
    workers_pool = new NodePool(num_workers, [this, &fun](auto val) { 
        auto res = fun(val);
        gatherer->send(res); 
    });
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::run() {
    workers_pool->run();
    gatherer->run();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::wait() {
    workers_pool->wait();
    gatherer->notify_eos();
    gatherer->wait();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::notify_eos() {
    workers_pool->notify_eos();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::send(InputType& value) {
    workers_pool->send(value);
}

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::~Farm() {
    delete workers_pool;
    delete gatherer;
}


#endif //AUTONOMIC_FARM_FARM_HPP
