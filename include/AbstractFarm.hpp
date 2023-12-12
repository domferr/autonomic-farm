//
// Created by dferraro on 11/12/23.
//

#ifndef ABSTRACTFARM_H
#define ABSTRACTFARM_H

#include <vector>
#include "Emitter.hpp"
#include "AbstractNode.hpp"

template <typename InputType, typename OutputType>
class AbstractFarm : public AbstractNode<InputType> {
public:
    void run() override;
    void wait() override;
    void send(InputType value) override;
    void notify_eos() override;

protected:
    AbstractNode<InputType> *emitter;
    AbstractNode<OutputType> *gatherer;
    std::vector<AbstractNode<InputType>*> workers;
};

template<typename InputType, typename OutputType>
void AbstractFarm<InputType, OutputType>::run() {
    emitter->run();
    for (auto const& worker: workers) {
        worker->run();
    }
    gatherer->run();
}

template<typename InputType, typename OutputType>
void AbstractFarm<InputType, OutputType>::wait() {
    emitter->wait();
    for (auto const& worker: workers) {
        worker->wait();
    }
    gatherer->wait();
}

template<typename InputType, typename OutputType>
void AbstractFarm<InputType, OutputType>::send(InputType value) {
    emitter->send(value);
}

template<typename InputType, typename OutputType>
void AbstractFarm<InputType, OutputType>::notify_eos() {
    emitter->notify_eos();
    for (auto const& worker: workers) {
        worker->notify_eos();
    }
    gatherer->notify_eos();
}


#endif //ABSTRACTFARM_H
