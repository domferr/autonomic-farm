#ifndef AUTONOMICFARM_AUTONOMICWORKERPOOL_HPP
#define AUTONOMICFARM_AUTONOMICWORKERPOOL_HPP

#include "AutonomicWorker.hpp"
#include "NodePool.hpp"
#include "FarmAnalytics.hpp"
#include "Autonomic.hpp"

/**
 * A node pool able to dynamically change the number of nodes based on service time. The processing element who notifies
 * the newest service time is also responsible of executing the code needed to change the number of nodes.
 * When sending a new item, instead of round-robin, each node is responsible of pulling items from a main stream.
 * When a new service time is provided the pool will compute the new number of workers and they are paused or unpaused
 * accordingly. Then, when the service time changes instead of computing again the number of workers, the pool checks
 * if the service time is increasing or decreasing to validate the previous decision. To do so, linear regression is
 * performed and the slope of the resulting line is used.
 */
template <typename InputType>
class AutonomicWorkerPool : public NodePool<InputType, AutonomicWorker<InputType>>, public Autonomic {
public:
    template <typename... Args, typename WorkerFunType>
    explicit AutonomicWorkerPool(size_t num_workers, const WorkerFunType &workerFun,
       size_t min_num_workers, size_t max_num_workers, double target_service_time, farm_analytics* analytics);

    /**
     * Run the autonomic worker pool by running all the nodes.
     */
    void run() override;

    /**
     * Instead of round-robin, the new value is sent to the autonomic worker pool's input stream. The nodes in the pool
     * are responsible of taking the items from the input stream.
     * @param value
     */
    void send(InputType &value) override;

    /**
     * Send end-of-stream to the autonomic worker pool's input stream.
     */
    void notify_eos() override;

    void pauseWorkers(size_t fromIndex, size_t toIndex) override;

    void unpauseWorkers(size_t fromIndex, size_t toIndex) override;

    long getArrivalTime() override;

    long getWorkerServiceTime() override;

private:
    // input stream of this node pool
    Stream<InputType> main_stream;

    // arrival time computation
    std::atomic<long> atomic_arrival_time;
    std::chrono::system_clock::time_point last_arrival_timepoint;

    std::atomic<double> atomic_worker_service_time;
};

template<typename InputType>
long AutonomicWorkerPool<InputType>::getArrivalTime() {
    return atomic_arrival_time;
}

template<typename InputType>
long AutonomicWorkerPool<InputType>::getWorkerServiceTime() {
    return atomic_worker_service_time;
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::unpauseWorkers(size_t fromIndex, size_t toIndex) {
    for (size_t i = fromIndex; i <= toIndex; ++i) {
        this->nodes[i].unpause();
    }
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::pauseWorkers(size_t fromIndex, size_t toIndex) {
    for (size_t i = fromIndex; i <= toIndex; ++i) {
        this->nodes[i].pause();
    }
}

template<typename InputType>
template<typename... Args, typename WorkerFunType>
AutonomicWorkerPool<InputType>::AutonomicWorkerPool(size_t num_workers, const WorkerFunType &workerFun,
    size_t min_num_workers, size_t max_num_workers, double target_service_time, farm_analytics* analytics)
: Autonomic(analytics, num_workers, min_num_workers, max_num_workers, target_service_time) {
    auto onExit = [this]() {
        for (int i = 0; i < this->nodes.size(); ++i) {
            this->nodes[i].unpause();
        }
    };
    this->init(max_num_workers, workerFun, &main_stream, onExit);
    this->nodes[0].setAsReference(&atomic_worker_service_time);
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::run() {
    NodePool<InputType, AutonomicWorker<InputType>>::run();

    analytics->num_workers.emplace_back(this->num_workers, 0);
    // initialize the arrival time
    last_change = analytics->farm_start_time;
    last_arrival_timepoint = analytics->farm_start_time;
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::send(InputType &value) {
    START(now);

    main_stream.add(value);

    auto elapsed = ELAPSED(last_arrival_timepoint, now, std::chrono::milliseconds);
    atomic_arrival_time = elapsed;
    if (target_best_service_time) target_service_time = (double) elapsed;
    last_arrival_timepoint = now;
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::notify_eos() {
    main_stream.eos();
}


#endif //AUTONOMICFARM_AUTONOMICWORKERPOOL_HPP
