//
// Created by dferraro on 11/12/23.
//

#ifndef FARM_H
#define FARM_H

#include "Emitter.hpp"
#include "Gatherer.hpp"
#include "Worker.hpp"
#include "BufferedNode.hpp"
#include "AbstractFarm.hpp"

template <typename InputType, typename OutputType>
class Farm: public AbstractFarm<InputType, OutputType> {
public:
    typedef std::function<OutputType(InputType)> Fun;
    typedef std::function<void(OutputType)> OutputFun;

    explicit Farm(int workers, const Fun& fun, const OutputFun& outputFun);

    using AbstractFarm<InputType, OutputType>::run;
    using AbstractFarm<InputType, OutputType>::wait;
    using AbstractFarm<InputType, OutputType>::send;
    using AbstractFarm<InputType, OutputType>::notify_eos;
};

template<typename InputType, typename OutputType>
Farm<InputType, OutputType>::Farm(int num_workers, const Fun &fun, const OutputFun &outputFun) {
    this->gatherer = new Gatherer<OutputType, OutputFun>(outputFun);
    this->workers = std::vector<AbstractNode<InputType>*>(num_workers);
    for (int i = 0; i < num_workers; ++i) {
        this->workers[i] = new Worker<InputType, OutputType, Fun>(fun, this->gatherer);
    }
    this->emitter = new Emitter<InputType>(this->workers);
}

#endif //FARM_H
