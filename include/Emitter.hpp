//
// Created by dferraro on 11/12/23.
//

#include <thread>
#include "BufferedNode.hpp"
#include "Worker.hpp"
#include "Stream.hpp"

#ifndef EMITTER_H
#define EMITTER_H


template <typename InputType>
class Emitter : public BufferedNode<InputType> {
public:
    explicit Emitter(const std::vector<AbstractNode<InputType>*>& workers);

private:
    void body(InputType) override;

    std::vector<AbstractNode<InputType>*> workers;
    int worker_index = 0;
};

template<typename InputType>
Emitter<InputType>::Emitter(const std::vector<AbstractNode<InputType>*>& workers) : workers(workers) {}

template<typename InputType>
void Emitter<InputType>::body(InputType value) {
    // round-robin
    workers[worker_index]->send(value);
    worker_index = (worker_index+1) % workers.size();
}


#endif //EMITTER_H
