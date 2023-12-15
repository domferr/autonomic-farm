#ifndef AUTONOMIC_FARM_FARM_HPP
#define AUTONOMIC_FARM_FARM_HPP

#include "Node.hpp"
#include "ThreadedNode.hpp"
#include "trace.hpp"
#include "Emitter.hpp"

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
    std::vector<ThreadedNode<InputType>> workers;
    Emitter<InputType> emitter;
    ThreadedNode<OutputType>* gatherer;
};

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::Farm(size_t num_workers, const Farm::WorkerFunType &fun, ThreadedNode<OutputType>* newGatherer)
: gatherer(newGatherer) {
    workers.reserve(num_workers);
    for (int i = 0; i < num_workers; ++i) {
        TRACEF("Init worker %d/%lu", (i+1), num_workers);
        auto worker = ThreadedNode<InputType>([this, &fun](InputType& value) {
            OutputType res = fun(value);
            return this->gatherer->send(res);
        });
        workers.emplace_back(std::move(worker));
    }
    emitter.setWorkers(&workers);
}

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::Farm(size_t num_workers, const WorkerFunType &fun, const ThreadedNode<OutputType>::OnValueFun &sendOutFun)
: Farm<InputType, OutputType>::Farm(num_workers, fun, new ThreadedNode<OutputType>(sendOutFun)) {}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::run() {
    TRACE("Run emitter");
    emitter.run();
    for (int i = 0; i < workers.size(); ++i) {
        TRACEF("Run worker %d/%lu", (i+1), workers.size());
        workers[i].run();
    }
    TRACE("Run gatherer");
    gatherer->run();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::wait() {
    TRACE("Wait emitter");
    emitter.wait();
    TRACE("Emitter ended");
    for (int i = 0; i < workers.size(); ++i) {
        TRACEF("Wait worker %d/%lu", (i+1), workers.size());
        workers[i].wait();
        TRACEF("Worker %d ended", (i+1));
    }
    TRACE("Notify EOS to gatherer");
    gatherer->notify_eos();
    TRACE("Wait gatherer");
    gatherer->wait();
    TRACE("Gatherer ended");
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::notify_eos() {
    emitter.notify_eos();
}

template<typename InputType, typename OutputType>
void Farm<InputType, OutputType>::send(InputType& value) {
    emitter.send(value);
}

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::~Farm() {
    delete gatherer;
}


#endif //AUTONOMIC_FARM_FARM_HPP
