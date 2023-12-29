#ifndef AUTONOMICFARM_MONITOREDFARM_HPP
#define AUTONOMICFARM_MONITOREDFARM_HPP

#include "Farm.hpp"
#include "MonitoringGatherer.hpp"
#include "FarmAnalytics.hpp"

template <typename InputType, typename OutputType>
class MonitoredFarm : public Farm<InputType, OutputType> {
public:
    using WorkerFunType = Farm<InputType, OutputType>::WorkerFunType;
    using SendOutFunType = Farm<InputType, OutputType>::SendOutFunType;

    MonitoredFarm(size_t num_workers, const WorkerFunType &fun, const SendOutFunType &sendOutFun);

    void run() override;
    void send(InputType &value) override;

    /**
     * Wait for the farm to finish and then return the analytics. It is the only way to safely obtain the analytics,
     * ensuring mutual exclusion
     * @return the analytics obtained during the farm execution
     */
    farm_analytics wait_and_analytics();

protected:
    MonitoredFarm() = default;

    farm_analytics analytics;
};

template<typename InputType, typename OutputType>
MonitoredFarm<InputType, OutputType>::MonitoredFarm(size_t num_workers, const WorkerFunType &fun, const SendOutFunType &sendOutFun) {
    this->gatherer = new MonitoringGatherer<OutputType>(sendOutFun, &analytics);
    this->workers_pool = new NodePool<InputType, ThreadedNode<InputType>>(num_workers, [this, &fun](auto val) {
        auto res = fun(val);
        this->gatherer->send(res);
    });
    // we already know the current number of workers
    analytics.num_workers.emplace_back(num_workers, 0);
}

template<typename InputType, typename OutputType>
void MonitoredFarm<InputType, OutputType>::run() {
    // the farm_start_time is the time when the run() method was called and before running any thread
    analytics.farm_start_time = std::chrono::system_clock::now();
    Farm<InputType, OutputType>::run();
}

template<typename InputType, typename OutputType>
void MonitoredFarm<InputType, OutputType>::send(InputType &value) {
    Farm<InputType, OutputType>::send(value);
    // track at which time a new item arrived
    STOP(analytics.farm_start_time, time, std::chrono::milliseconds);
    analytics.arrival_time.emplace_back(time);
}

template<typename InputType, typename OutputType>
farm_analytics MonitoredFarm<InputType, OutputType>::wait_and_analytics() {
    // wait for the farm to finish and then return the analytics
    Farm<InputType, OutputType>::wait();
    return analytics;
}

#endif //AUTONOMICFARM_MONITOREDFARM_HPP
