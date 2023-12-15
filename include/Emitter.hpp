//
// Created by dferraro on 21/12/23.
//

#ifndef AUTONOMICFARM_EMITTER_HPP
#define AUTONOMICFARM_EMITTER_HPP


#include "ThreadedNode.hpp"
#include "trace.hpp"

template <typename InputType>
class Emitter : public ThreadedNode<InputType> {
public:
    void setWorkers(std::vector<ThreadedNode<InputType>>* newWorkers) { this->workers = newWorkers; }
    void onValue(InputType& value) override;
    void onEOS() override;

private:
    std::vector<ThreadedNode<InputType>>* workers;
    size_t worker_index{};
};

template<typename InputType>
void Emitter<InputType>::onValue(InputType& value) {
    TRACEF("Send to worker %lu", worker_index);
    workers->at(worker_index).send(value);
    worker_index = (worker_index+1) % workers->size();
}

template<typename InputType>
void Emitter<InputType>::onEOS() {
    // when the emitter reaches the end of its stream, it notifies EOS to every worker
    for (int i = 0; i < workers->size(); ++i) {
        TRACEF("Notify EOS to worker %d/%lu", (i+1), workers.size());
        workers->at(i).notify_eos();
    }
}


#endif //AUTONOMICFARM_EMITTER_HPP
