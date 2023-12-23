//
// Created by dferraro on 23/12/23.
//

#ifndef AUTONOMICFARM_WORKERPOOL_HPP
#define AUTONOMICFARM_WORKERPOOL_HPP


#include <vector>
#include "ThreadedNode.hpp"

template <typename InputType>
class WorkerPool {

private:
    void increaseWorkers();
    void decreaseWorkers();

    std::vector<ThreadedNode<InputType>> workers;
};


#endif //AUTONOMICFARM_WORKERPOOL_HPP
