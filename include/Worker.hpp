//
// Created by dferraro on 11/12/23.
//

#include <thread>
#include "BufferedNode.hpp"
#include "Gatherer.hpp"
#include "Farm.hpp"

#ifndef WORKER_H
#define WORKER_H


template <typename InputType, typename OutputType, typename Fun>
class Worker: public BufferedNode<InputType> {
public:
    Worker(const Fun &fun, AbstractNode<OutputType>* gatherer);

private:
    void body(InputType) override;

    Fun fun;
    AbstractNode<OutputType>* gatherer;
};

template <typename InputType, typename OutputType, typename Fun>
Worker<InputType, OutputType, Fun>::Worker(const Fun &fun, AbstractNode<OutputType>* gatherer) : fun(fun), gatherer(gatherer) {

}

template <typename InputType, typename OutputType, typename Fun>
void Worker<InputType, OutputType, Fun>::body(InputType value) {
    auto res = fun(value);
    gatherer->send(res);
}


#endif //WORKER_H
