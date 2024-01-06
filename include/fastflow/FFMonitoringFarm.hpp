#ifndef AUTONOMICFARM_FFMonitoringFarm_HPP
#define AUTONOMICFARM_FFMonitoringFarm_HPP


#include <iostream>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include "MonitoredFarm.hpp"
#include "FFWorker.hpp"
#include "FFMonitoringGatherer.hpp"

template<typename InputType, typename OutputType>
class FFMonitoringFarm {
public:
    using WorkerFunType = FFWorker<InputType, OutputType>::WorkerFunType;
    typedef std::function<void(OutputType *)> SendOutFunType;

    FFMonitoringFarm(size_t num_workers, const WorkerFunType &fun, const SendOutFunType &sendOutFun, farm_analytics *analytics);

    template<typename SourceNodeType>
    void run(SourceNodeType &source);
    void wait();

private:
    ff::ff_farm *farm;
    farm_analytics *analytics;
    FFMonitoringGatherer<OutputType> *collector;
    ff::ff_Pipe<InputType, OutputType>* running_pipe;
};

template<typename InputType, typename OutputType>
FFMonitoringFarm<InputType, OutputType>::FFMonitoringFarm(size_t num_workers, const WorkerFunType &fun, const SendOutFunType &sendOutFun,
                                                          farm_analytics *analytics) : analytics(analytics) {
    std::vector<ff::ff_node *> workers;
    for (auto i = 0; i < num_workers; i++) {
        workers.push_back(new FFWorker<InputType, OutputType>(fun));
    }
    farm = new ff::ff_farm();
    farm->add_workers(workers);
    collector = new FFMonitoringGatherer<OutputType>(sendOutFun, analytics);
    farm->add_collector(collector);
    // we already know the current number of workers
    analytics->num_workers.emplace_back(num_workers, 0);
}

template<typename InputType, typename OutputType>
template<typename SourceNodeType>
void FFMonitoringFarm<InputType, OutputType>::run(SourceNodeType &source) {
    // the farm_start_time is the time when the run() method was called and before running any thread
    analytics->farm_start_time = std::chrono::system_clock::now();
    running_pipe = new ff::ff_Pipe<InputType, OutputType>(source, *farm);
    running_pipe->run_then_freeze();
}

template<typename InputType, typename OutputType>
void FFMonitoringFarm<InputType, OutputType>::wait() {
    this->running_pipe->wait_freezing();
}


#endif //AUTONOMICFARM_FFMonitoringFarm_HPP
