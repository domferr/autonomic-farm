#ifndef AUTONOMICFARM_AUTONOMICFARM_HPP
#define AUTONOMICFARM_AUTONOMICFARM_HPP


#include "MonitoredFarm.hpp"
#include "AutonomicGatherer.hpp"
#include "AutonomicWorkerPool.hpp"
#include "FarmAnalytics.hpp"

template <typename InputType, typename OutputType>
class AutonomicFarm : public MonitoredFarm<InputType, OutputType> {
public:
    using WorkerFunType = MonitoredFarm<InputType, OutputType>::WorkerFunType;
    using SendOutFunType = MonitoredFarm<InputType, OutputType>::SendOutFunType;

    AutonomicFarm(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers, double target_service_time,
                  const WorkerFunType &fun, const SendOutFunType &sendOutFun);
};

template<typename InputType, typename OutputType>
AutonomicFarm<InputType, OutputType>::AutonomicFarm(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers,
    double target_service_time, const WorkerFunType &fun, const SendOutFunType &sendOutFun) {
    auto workerfun = [&fun, this](auto val) {
        auto res = fun(val);
        this->gatherer->send(res);
    };
    auto autonomic_pool = new AutonomicWorkerPool<InputType>(num_workers, workerfun,
        minNumWorkers, maxNumWorkers, target_service_time, &this->analytics
    );
    this->gatherer = new AutonomicGatherer<InputType, OutputType>(sendOutFun, &this->analytics, autonomic_pool);
    this->workers_pool = autonomic_pool;
}


#endif //AUTONOMICFARM_AUTONOMICFARM_HPP
