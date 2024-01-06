#ifndef AUTONOMICFARM_AUTONOMIC_HPP
#define AUTONOMICFARM_AUTONOMIC_HPP

#include "utimer.hpp"

class Autonomic {
public:

    Autonomic(farm_analytics *analytics, size_t numWorkers, size_t minNumWorkers, size_t maxNumWorkers, double targetServiceTime);

    /**
     * Notify the newest service time and change the number of workers accordingly.
     * @param current_service_time the newest service time
     * @return the new number of workers or -1 if the number of workers shouldn't be changed
     */
    virtual void onNewServiceTime(double current_service_time);

    /**
     * Given the current service time and a point in time, compute the new number of workers. Returns the new number
     * of workers if it is possible to compute it. It returns -1 if it is not possible to compute a new number of
     * worker or if it is not a good idea to change the number of workers.
     * @param current_service_time the current service time
     * @param now the point in time in which this method is called
     * @return the new number of workers or -1 if the number of workers shouldn't be changed
     */
    virtual int improveServiceTime(double current_service_time, std::chrono::system_clock::time_point now);

protected:
    farm_analytics* analytics;

    // current number of workers
    size_t num_workers;
    size_t min_num_workers;
    size_t max_num_workers;
    double target_service_time;
    bool target_best_service_time = false;

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

    /**
     * Changes the number of workers and unpauses or pauses accordingly. Given the point in time, this function takes
     * care of remembering the given point in time, to know later when it was the last time the number of workers
     * was changed.
     * @param new_num_workers the new number of workers
     * @param now the point in time when this method was called
     */
    void changeWorkersNumber(size_t new_num_workers, std::chrono::system_clock::time_point now);

    virtual void pauseWorkers(size_t fromIndex, size_t toIndex) = 0;

    virtual void unpauseWorkers(size_t fromIndex, size_t toIndex) = 0;

    virtual long getArrivalTime() = 0;

    virtual long getWorkerServiceTime() = 0;
};

void Autonomic::onNewServiceTime(double current_service_time) {
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
    long arrival_time = getArrivalTime();
    if (target_best_service_time && std::abs(current_service_time - arrival_time) < max_service_time_error) {
        //current farm's service time is equal to arrival time, then try to improve efficiency
        new_num_workers = improveServiceTime(getWorkerServiceTime() / num_workers, now);
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

int Autonomic::improveServiceTime(double current_service_time, std::chrono::system_clock::time_point now) {
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

void Autonomic::changeWorkersNumber(size_t new_num_workers, std::chrono::system_clock::time_point now) {
    // pause of unpause accordingly
    if (new_num_workers > num_workers) {
        // to increase number of nodes, unpause the paused ones
        unpauseWorkers(num_workers, new_num_workers - 1);
    } else {
        // to decrease the number of nodes, pause the not needed ones
        pauseWorkers(new_num_workers, num_workers - 1);
    }
    // change the number of workers
    num_workers = new_num_workers;

    // update the analytics with the newest number of workers
    auto global_elapsed = ELAPSED(analytics->farm_start_time, now, std::chrono::milliseconds);
    analytics->num_workers.emplace_back(num_workers, global_elapsed);

    // remember the point in time when we had the last correct number of workers
    last_change = now;
}

Autonomic::Autonomic(farm_analytics *analytics, size_t numWorkers, size_t minNumWorkers, size_t maxNumWorkers,
                     double targetServiceTime) : analytics(analytics), num_workers(numWorkers),
                     min_num_workers(minNumWorkers), max_num_workers(maxNumWorkers), target_service_time(targetServiceTime),
                     target_best_service_time(target_service_time == 0) {}


#endif //AUTONOMICFARM_AUTONOMIC_HPP
