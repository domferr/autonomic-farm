#ifndef AUTONOMICFARM_FFWORKER_HPP
#define AUTONOMICFARM_FFWORKER_HPP

#include <ff/ff.hpp>
#include <ff/farm.hpp>

template <typename InputType, typename OutputType>
class FFWorker : public ff::ff_node_t<InputType, OutputType> {
public:
    typedef std::function<OutputType*(InputType *)> WorkerFunType;

    explicit FFWorker(const WorkerFunType &fun) : fun(fun) {}

    OutputType *svc(InputType *task) override;

private:
    WorkerFunType fun;

};

template<typename InputType, typename OutputType>
OutputType *FFWorker<InputType, OutputType>::svc(InputType *task) {
    auto result = fun(task);
    this->ff_send_out(result);
    return this->GO_ON;
}


#endif //AUTONOMICFARM_FFWORKER_HPP
