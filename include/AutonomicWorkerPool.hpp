#ifndef AUTONOMICFARM_AUTONOMICWORKERPOOL_HPP
#define AUTONOMICFARM_AUTONOMICWORKERPOOL_HPP

#include "AutonomicWorker.hpp"
#include "NodePool.hpp"
#include "FarmAnalytics.hpp"

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
class AutonomicWorkerPool : public NodePool<InputType, AutonomicWorker<InputType>> {
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

    /**
     * Notify the newest service time and change the number of workers accordingly.
     * @param current_service_time the newest service time
     */
    void onNewServiceTime(double current_service_time);

    /**
     * Given the current service time and a point in time, compute the new number of workers. Returns the new number
     * of workers if it is possible to compute it. It returns -1 if it is not possible to compute a new number of
     * worker or if it is not a good idea to change the number of workers.
     * @param current_service_time the current service time
     * @param now the point in time in which this method is called
     * @return the new number of workers or -1 if the number of workers shouldn't be changed
     */
    int improveServiceTime(double current_service_time, std::chrono::system_clock::time_point now);

    /**
     * Changes the number of workers and unpauses or pauses accordingly. Given the point in time, this function takes
     * care of remembering the given point in time, to know later when it was the last time the number of workers
     * was changed.
     * @param new_num_workers the new number of workers
     * @param now the point in time when this method was called
     */
    void changeWorkersNumber(size_t new_num_workers, std::chrono::system_clock::time_point now);

private:
    farm_analytics* analytics;
    // input stream of this node pool
    Stream<InputType> main_stream;
    // current number of workers
    size_t num_workers;
    size_t min_num_workers;
    size_t max_num_workers;
    double target_service_time;
    bool target_best_service_time = false;

    // arrival time computation
    std::atomic<size_t> atomic_arrival_time;
    std::chrono::system_clock::time_point last_arrival_timepoint;

    std::atomic<size_t> atomic_worker_service_time;

    // the last time when the number of workers was correct based on the service time
    std::chrono::system_clock::time_point last_change;
    // window of service times, used to perform linear regression
    std::deque<std::pair<double, long>> service_time_window;
    // sum of all the service times in the window
    double window_service_time_sum = 0.0;
    // sum of all the times in the window
    double window_elapsed_time_sum = 0.0;

    // constants
    // minimum time needed to elapse before making a change in the number of workers
    const long reaction_time_ms = 190; // ms
    const size_t service_time_window_size = 6;
    // maximum service time error
    const double max_service_time_error = 1.0;
};

template<typename InputType>
template<typename... Args, typename WorkerFunType>
AutonomicWorkerPool<InputType>::AutonomicWorkerPool(size_t num_workers, const WorkerFunType &workerFun,
    size_t min_num_workers, size_t max_num_workers, double target_service_time, farm_analytics* analytics)
: target_service_time(target_service_time), min_num_workers(min_num_workers), max_num_workers(max_num_workers),
analytics(analytics), num_workers(num_workers), target_best_service_time(target_service_time == 0) {
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
    last_change = analytics->farm_start_time;
    last_arrival_timepoint = analytics->farm_start_time;
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::send(InputType &value) {
    START(now);

    main_stream.add(value);

    size_t elapsed = ELAPSED(last_arrival_timepoint, now, std::chrono::milliseconds);
    atomic_arrival_time = elapsed;
    if (target_best_service_time) target_service_time = (double) elapsed;
    last_arrival_timepoint = now;
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::notify_eos() {
    main_stream.eos();
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::onNewServiceTime(double current_service_time) {
    // update the window
    START(now);
    auto current_time = ELAPSED(analytics->farm_start_time, now, std::chrono::milliseconds);
    service_time_window.emplace_back(current_service_time, current_time);
    window_service_time_sum += current_service_time;
    window_elapsed_time_sum += (double) current_time;
    // ensure the window has the minimum number of elements
    if (service_time_window.size() <= service_time_window_size) return;

    // remove the oldest element from the window and lower the service time sum and the time sum
    window_service_time_sum -= service_time_window.front().first;
    window_elapsed_time_sum -= service_time_window.front().second;
    service_time_window.pop_front();

    // check if at least <reaction_time_ms> elapsed from the last time we had a correct number of workers
    if (ELAPSED(last_change, now, std::chrono::milliseconds) <= reaction_time_ms) return;

    int new_num_workers;
    if (target_best_service_time && std::abs(current_service_time - atomic_arrival_time) < max_service_time_error) {
        //current farm's service time is equal to arrival time, then try to improve efficiency
        new_num_workers = improveServiceTime(atomic_worker_service_time / num_workers, now);
    } else {
        new_num_workers = improveServiceTime(current_service_time, now);
    }
    if (new_num_workers == -1) return;

    // if the new optimal number of workers is equal to the current number, we don't make any change, but we have to
    // remember the point in time when we had the last correct number of workers
    if (new_num_workers == num_workers) {
        last_change = now;
        return;
    }

    // let's change the number of workers by pausing or unpausing accordingly
    changeWorkersNumber(new_num_workers, now);
}

template<typename InputType>
void AutonomicWorkerPool<InputType>::changeWorkersNumber(size_t new_num_workers, std::chrono::system_clock::time_point now) {
    // pause of unpause accordingly
    if (new_num_workers > num_workers) {
        // to increase number of nodes, unpause the paused ones
        for (size_t i = num_workers; i < new_num_workers; ++i) {
            this->nodes[i].unpause();
        }
    } else {
        // to decrease the number of nodes, pause the not needed ones
        for (size_t i = new_num_workers; i < num_workers; ++i) {
            this->nodes[i].pause();
        }
    }
    // change the number of workers
    num_workers = new_num_workers;

    // update the analytics with the newest number of workers
    auto global_elapsed = ELAPSED(analytics->farm_start_time, now, std::chrono::milliseconds);
    analytics->num_workers.emplace_back(num_workers, global_elapsed);

    // remember the point in time when we had the last correct number of workers
    last_change = now;
}

template<typename InputType>
int AutonomicWorkerPool<InputType>::improveServiceTime(
        double current_service_time, std::chrono::system_clock::time_point now) {
    // if the service time is near to the target with a maximum of max_service_time_error error, then the current number
    // of workers can be considered correct. Abort any change
    if (current_service_time > target_service_time - max_service_time_error && current_service_time < target_service_time + max_service_time_error) {
        last_change = now;
        return -1;
    }

    // if we are here it means that the service time is not near the target. We may need to change the number of workers,
    // but if we already did it previously, we need to check if the previous change is having a good impact. We do it
    // by looking at the slope of the service time function. To compute the slope, we perform linear regression.

    double sxy = 0.0, sxx = 0.0;
    double service_time_avg = window_service_time_sum / service_time_window.size();
    double time_avg = window_elapsed_time_sum / service_time_window.size();
    for (auto &svt : service_time_window) {
        auto service_time_i = svt.first;
        auto time_i = (double) svt.second;
        sxy += (time_i - time_avg) * (service_time_i - service_time_avg);
        sxx += (time_i - time_avg) * (time_i - time_avg);
    }

    double curr_slope = sxy / sxx;

    // if it is below target but rising, then the previous change is having a good impact
    if (current_service_time < target_service_time - max_service_time_error && curr_slope > 1) return -1;
    // if it is above target but falling, then the previous change is having a good impact
    if (current_service_time > target_service_time + max_service_time_error && curr_slope < -1) return -1;

    // if we are here it means that we need to change the number of workers according to the current service time.

    // the optimal number of nodes is worker's service time over the target service time
    double curr_workers_service_time = current_service_time * (double) num_workers;
    return std::clamp(
            (size_t) std::round(curr_workers_service_time / target_service_time),
            min_num_workers,
            max_num_workers
    );
}


#endif //AUTONOMICFARM_AUTONOMICWORKERPOOL_HPP
