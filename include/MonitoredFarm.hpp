//
// Created by dferraro on 21/12/23.
//

#ifndef AUTONOMICFARM_MONITOREDFARM_HPP
#define AUTONOMICFARM_MONITOREDFARM_HPP

#include "Farm.hpp"
#include "MonitoringGatherer.hpp"
#include "FarmAnalytics.hpp"


template <typename InputType, typename OutputType>
class MonitoredFarm : public Farm<InputType, OutputType> {
public:
    MonitoredFarm(size_t numWorkers, const Farm<InputType, OutputType>::WorkerFunType &fun, const ThreadedNode<OutputType>::OnValueFun &sendOutFun);

    void run() override;
    void send(InputType &value) override;

    farm_analytics wait_and_analytics();

private:
    farm_analytics analytics;
};

template<typename InputType, typename OutputType>
MonitoredFarm<InputType, OutputType>::MonitoredFarm(
size_t numWorkers, const Farm<InputType, OutputType>::WorkerFunType &fun, const ThreadedNode<OutputType>::OnValueFun &sendOutFun)
: Farm<InputType, OutputType>(numWorkers, fun, new MonitoringGatherer<OutputType>(sendOutFun, &analytics)) {}

template<typename InputType, typename OutputType>
void MonitoredFarm<InputType, OutputType>::run() {
    Farm<InputType, OutputType>::run();
    analytics.farm_start_time = std::chrono::system_clock::now();
}

template<typename InputType, typename OutputType>
void MonitoredFarm<InputType, OutputType>::send(InputType &value) {
    Farm<InputType, OutputType>::send(value);
    auto now = std::chrono::system_clock::now();
    STOP(analytics.farm_start_time, time);
    analytics.arrival_time.emplace_back(time);
}

template<typename InputType, typename OutputType>
farm_analytics MonitoredFarm<InputType, OutputType>::wait_and_analytics() {
    Farm<InputType, OutputType>::wait();
    return this->analytics;
}

#endif //AUTONOMICFARM_MONITOREDFARM_HPP
