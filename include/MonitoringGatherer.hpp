#ifndef AUTONOMICFARM_MONITORINGGATHERER_HPP
#define AUTONOMICFARM_MONITORINGGATHERER_HPP


#include "ThreadedNode.hpp"
#include "trace.hpp"
#include "utimer.hpp"
#include "MonitoredFarm.hpp"
#include "FarmAnalytics.hpp"

/**
 * A special gatherer that computes throughput and service times. The metrics are local, meaning that they are not computed
 * looking at the needed information from the beginning of the farm execution, but instead by looking at most to some
 * milliseconds in the past. The metrics are also smoothed via moving average application.
 */
template <typename OutputType>
class MonitoringGatherer : public ThreadedNode<OutputType> {
public:
    using GathererFunType = ThreadedNode<OutputType>::OnValueFun;

    MonitoringGatherer(const GathererFunType &onValueFun, farm_analytics *analytics) : ThreadedNode<OutputType>(onValueFun), analytics(analytics) {}

    void onValue(OutputType& value) override;

protected:
    farm_analytics* analytics;
    // window of the last throughput values withing <throughput_evaluation_time> milliseconds
    std::deque<std::pair<size_t, double>> throughput_window; // pair <tasks gathered, when it was acquired>
    std::deque<std::pair<double, double>> linear_regression_throughput_window; // pair <throughput, when it was acquired>
    // the size of a second window, used to compute throughput and service time's moving averages
    size_t moving_avg_window_size = 5;
    double moving_avg_window_throughput_sum = 0;
    double moving_avg_window_servicetime_sum = 0;

    // constants
    // throughput is computed by counting the number of tasks per millisecond gathered within the last <throughput_evaluation_time> milliseconds
    const size_t throughput_evaluation_time = 300; //ms

    void compute_moving_average(double throughput, double global_elapsed);
};

template <typename OutputType>
void MonitoringGatherer<OutputType>::onValue(OutputType& value) {
    // override gatherer thread's function to add monitoring
    START(now);
    this->onValueFun(value);
    // get the time elapsed from the beginning of the farm computation
    double global_elapsed = ELAPSED(analytics->farm_start_time, now, std::chrono::milliseconds);

    // compute the number of tasks gathered now by increasing the number of tasks gathered before
    auto global_tasks_gathered = throughput_window.front().first + 1;
    // add current measurement into the window
    throughput_window.emplace_front( global_tasks_gathered, global_elapsed );

    // remove old measurements from the window
    while(!throughput_window.empty() && global_elapsed - throughput_window.back().second > throughput_evaluation_time) {
        throughput_window.pop_back();
    }

    // how many tasks were gathered from the beginning of the window is equal to total number of tasks gathered now
    // minus the total number of tasks gathered at the beginning of the window
    double window_tasks_gathered = global_tasks_gathered - throughput_window.back().first;
    double window_elapsed = global_elapsed - throughput_window.back().second;
    if (window_elapsed == 0) return;

    // compute window's throughput
    double window_throughput = window_tasks_gathered / window_elapsed;

    // save throughput and service time measurements
    analytics->throughput_points.emplace_back(window_throughput, global_elapsed);
    analytics->service_time_points.emplace_back(1 / window_throughput, global_elapsed);

    compute_moving_average(window_throughput, global_elapsed);
}

template<typename OutputType>
void MonitoringGatherer<OutputType>::compute_moving_average(double throughput, double global_elapsed) {
    // update moving average by summing current value and subtracting the oldest value (if available)
    moving_avg_window_throughput_sum += throughput;
    moving_avg_window_servicetime_sum += 1 / throughput;
    int exitingIndex = analytics->throughput_points.size() - moving_avg_window_size - 1;
    if (exitingIndex < 0) return; // stop if we are at the beginning, and we don't have enough points to compute moving average

    // subtract oldest value
    moving_avg_window_throughput_sum -= analytics->throughput_points[exitingIndex].first;
    moving_avg_window_servicetime_sum -= analytics->service_time_points[exitingIndex].first;
    // save throughput and service time after applying moving window average
    analytics->throughput.emplace_back(
            moving_avg_window_throughput_sum / (double) moving_avg_window_size,
            global_elapsed
    );
    auto current_service_time = moving_avg_window_servicetime_sum / (double) moving_avg_window_size;
    analytics->service_time.emplace_back(
            current_service_time,
            global_elapsed
    );
}


#endif //AUTONOMICFARM_MONITORINGGATHERER_HPP
