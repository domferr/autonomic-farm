#ifndef AUTONOMICFARM_FFAUTONOMICFARM_HPP
#define AUTONOMICFARM_FFAUTONOMICFARM_HPP


#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include "MonitoredFarm.hpp"
#include "FFAutonomicWorker.hpp"
#include "FFAutonomicGatherer.hpp"
#include "FFAutonomicEmitter.hpp"
#include "../../benchmark/benchmark.hpp"


template<typename InputType, typename OutputType>
class FFAutonomicFarm {
public:
    using WorkerFunType = FFAutonomicWorker<InputType, OutputType>::WorkerFunType;
    typedef std::function<void(OutputType *)> SendOutFunType;

    FFAutonomicFarm(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers, double target_service_time,
                    const WorkerFunType &fun, const SendOutFunType &sendOutFun, farm_analytics *analytics);

    template<typename SourceNodeType>
    void run(SourceNodeType &source);
    void wait();

    virtual ~FFAutonomicFarm();

private:
    ff::ff_farm *farm;
    farm_analytics *analytics;
    FFAutonomicEmitter<InputType, FFAutonomicWorker<InputType, OutputType>> *emitter;
    FFAutonomicGatherer<OutputType> *collector;
    ff::ff_Pipe<InputType, OutputType>* running_pipe;
};

template<typename InputType, typename OutputType>
FFAutonomicFarm<InputType, OutputType>::FFAutonomicFarm(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers,
    double target_service_time, const WorkerFunType &fun, const SendOutFunType &sendOutFun, farm_analytics *analytics)
    : analytics(analytics) {
    std::vector<ff::ff_node*> workers;
    emitter = new FFAutonomicEmitter<InputType, FFAutonomicWorker<InputType, OutputType>>(num_workers, minNumWorkers, maxNumWorkers, target_service_time, analytics);
    for (auto i = 0; i < maxNumWorkers; i++) {
        auto worker = new FFAutonomicWorker<InputType, OutputType>(fun);
        workers.push_back(worker);
        emitter->addWorker(worker);
    }
    farm = new ff::ff_farm(workers);
    farm->cleanup_workers();
    farm->remove_collector();
    farm->add_emitter(emitter);
    farm->cleanup_emitter();
    farm->wrap_around();
    collector = new FFAutonomicGatherer<OutputType>(sendOutFun, analytics);
    farm->add_collector(collector);
    farm->cleanup_collector();
    farm->wrap_around();
}

template<typename InputType, typename OutputType>
template<typename SourceNodeType>
void FFAutonomicFarm<InputType, OutputType>::run(SourceNodeType &source) {
    // the farm_start_time is the time when the run() method was called and before running any thread
    analytics->farm_start_time = std::chrono::system_clock::now();
    running_pipe = new ff::ff_Pipe<InputType, OutputType>(source, *farm);
    running_pipe->run_then_freeze();
}

template<typename InputType, typename OutputType>
void FFAutonomicFarm<InputType, OutputType>::wait() {
    this->running_pipe->wait_freezing();
    this->running_pipe->wait();
}

template<typename InputType, typename OutputType>
FFAutonomicFarm<InputType, OutputType>::~FFAutonomicFarm() {
    delete farm;
    delete running_pipe;
}


#endif //AUTONOMICFARM_FFAUTONOMICFARM_HPP
