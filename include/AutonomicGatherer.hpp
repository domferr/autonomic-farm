#ifndef AUTONOMICFARM_AUTONOMICGATHERER_HPP
#define AUTONOMICFARM_AUTONOMICGATHERER_HPP


#include "MonitoringGatherer.hpp"
#include "AutonomicWorkerPool.hpp"

/**
 * A special monitoring gatherer that notifies to autonomic worker pool the most updated service time.
 */
template <typename InputType, typename OutputType>
class AutonomicGatherer : public MonitoringGatherer<OutputType> {
public:
    using GathererFunType = MonitoringGatherer<OutputType>::GathererFunType;

    AutonomicGatherer(const GathererFunType &onValueFun, farm_analytics *analytics, AutonomicWorkerPool<InputType> *workersPool);

    void onValue(OutputType &value) override;

private:
    AutonomicWorkerPool<InputType>* workers_pool;
};

template<typename InputType, typename OutputType>
AutonomicGatherer<InputType, OutputType>::AutonomicGatherer(
const GathererFunType &onValueFun, farm_analytics *analytics, AutonomicWorkerPool<InputType> *workersPool)
: MonitoringGatherer<OutputType>(onValueFun, analytics), workers_pool(workersPool) {}

template<typename InputType, typename OutputType>
void AutonomicGatherer<InputType, OutputType>::onValue(OutputType &value) {
    // get the last service time, to understand if it will change or not
    auto prev_size = this->analytics->service_time.size();
    MonitoringGatherer<OutputType>::onValue(value);
    // if the service time didn't change then avoid notifying
    if (this->analytics->service_time.size() == prev_size) return;

    // notify the newest service time to the workers. The gatherer thread will execute all the code needed by the
    // workers pool to change the number of workers.
    auto current_service_time = this->analytics->service_time.back();
    workers_pool->onNewServiceTime(current_service_time.first);
}


#endif //AUTONOMICFARM_AUTONOMICGATHERER_HPP
