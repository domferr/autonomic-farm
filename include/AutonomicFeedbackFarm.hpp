#ifndef AUTONOMICFARM_AUTONOMICFEEDBACKFARM_HPP
#define AUTONOMICFARM_AUTONOMICFEEDBACKFARM_HPP


#include "MonitoredFarm.hpp"
#include "AutonomicFeedbackGatherer.hpp"
#include "FarmAnalytics.hpp"
#include "AutonomicFeedbackWorker.hpp"

template <typename InputType, typename OutputType>
class AutonomicFeedbackFarm : public MonitoredFarm<InputType, OutputType> {
public:
    using WorkerFunType = MonitoredFarm<InputType, OutputType>::WorkerFunType;
    using SendOutFunType = MonitoredFarm<InputType, OutputType>::SendOutFunType;

    AutonomicFeedbackFarm(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers, double target_service_time,
                  const WorkerFunType &fun, const SendOutFunType &sendOutFun);

    void send(InputType &value) override;

    void notify_eos() override;

    void run() override;

    void wait() override;

    ~AutonomicFeedbackFarm() override;

    farm_analytics wait_and_analytics() override;

private:
    size_t maxNumWorkers;
    size_t data_sent = 0;

    AutonomicFeedbackGatherer<InputType, OutputType>* feedback_gatherer;
};

template<typename InputType, typename OutputType>
AutonomicFeedbackFarm<InputType, OutputType>::AutonomicFeedbackFarm(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers,
    double target_service_time, const WorkerFunType &fun, const SendOutFunType &sendOutFun) : maxNumWorkers(maxNumWorkers) {
    this->feedback_gatherer = new AutonomicFeedbackGatherer<InputType, OutputType>(sendOutFun, num_workers, minNumWorkers,
           maxNumWorkers, target_service_time, &this->analytics, [this](){ this->workers_pool->notify_eos(); });
    this->workers_pool = new NodePool<InputType, AutonomicFeedbackWorker<InputType, OutputType>>(num_workers, fun, feedback_gatherer);
    // we already know the current number of workers
    this->analytics.num_workers.emplace_back(num_workers, 0);
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackFarm<InputType, OutputType>::run() {
    // the farm_start_time is the time when the run() method was called and before running any thread
    this->analytics.farm_start_time = std::chrono::system_clock::now();
    this->workers_pool->run();
    this->feedback_gatherer->run();
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackFarm<InputType, OutputType>::wait() {
    this->workers_pool->wait();
    this->feedback_gatherer->wait();
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackFarm<InputType, OutputType>::send(InputType &value) {
    if (data_sent < maxNumWorkers) { // initial-scheduling, perform round-robin
        MonitoredFarm<InputType, OutputType>::send(value);
    } else {
        // initial-scheduling ended, send to gatherer
        this->feedback_gatherer->send_input(value);
    }

    data_sent++;
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackFarm<InputType, OutputType>::notify_eos() {
    this->feedback_gatherer->notify_eos();
}

template<typename InputType, typename OutputType>
farm_analytics AutonomicFeedbackFarm<InputType, OutputType>::wait_and_analytics() {
    this->wait();
    return this->analytics;
}

template<typename InputType, typename OutputType>
AutonomicFeedbackFarm<InputType, OutputType>::~AutonomicFeedbackFarm() {
    delete this->feedback_gatherer;
}


#endif //AUTONOMICFARM_AUTONOMICFEEDBACKFARM_HPP
