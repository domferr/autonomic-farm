//
// Created by dferraro on 11/12/23.
//

#include <thread>

#ifndef GATHERER_H
#define GATHERER_H


template <typename OutputType, typename OutputFun>
class Gatherer: public BufferedNode<OutputType> {
public:
    explicit Gatherer(const OutputFun &outputFun);

private:
    void body(OutputType) override;

    OutputFun outputFun;
};

template<typename OutputValue, typename OutputFun>
Gatherer<OutputValue, OutputFun>::Gatherer(const OutputFun &outputFun) : outputFun(outputFun) {}

template<typename OutputValue, typename OutputFun>
void Gatherer<OutputValue, OutputFun>::body(OutputValue value) {
    outputFun(value);
}


#endif //GATHERER_H
