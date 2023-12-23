#ifndef AUTONOMICFARM_MONITORINGGATHERER_HPP
#define AUTONOMICFARM_MONITORINGGATHERER_HPP


#include "ThreadedNode.hpp"
#include "trace.hpp"
#include "MonitoredFarm.hpp"
#include "FarmAnalytics.hpp"

template <typename OutputType>
class MonitoringGatherer : public ThreadedNode<OutputType> {
public:
    MonitoringGatherer(const ThreadedNode<OutputType>::OnValueFun &onValueFun, farm_analytics *analytics)
    : ThreadedNode<OutputType>(onValueFun), analytics(analytics) {}

    void onValue(OutputType& value) override;
    void run() override;

private:
    farm_analytics* analytics;
    // throughput is computed by counting the number of tasks per millisecond gathered within the last <throughput_evaluation_time> milliseconds
    const size_t throughput_evaluation_time = 100; //ms
    // window of the last throughput values withing <throughput_evaluation_time> milliseconds
    std::deque<std::pair<size_t, std::chrono::system_clock::time_point>> throughput_window; // pair <throughput value, when it was acquired>
    // the size of a second window, used to compute throughput and service time's moving averages
    size_t moving_avg_window_size = 4;
    double moving_avg_window_throughput_sum = 0;
    double moving_avg_window_servicetime_sum = 0;
};

template<typename OutputType>
void MonitoringGatherer<OutputType>::run() {
    ThreadedNode<OutputType>::run();
    throughput_window.emplace_front( 0, (*analytics).farm_start_time );
}

template<typename OutputType>
void MonitoringGatherer<OutputType>::onValue(OutputType& value) {
    // override gatherer thread's function to add monitoring
    auto now = std::chrono::system_clock::now();
    this->onValueFun(value);
    // compute the number of tasks gathered now by increasing the number of tasks gathered before
    auto global_tasks_gathered = throughput_window.front().first + 1;
    // add current measurement into the window
    throughput_window.emplace_front( global_tasks_gathered, now );
    // remove old measurements from the window
    while(ELAPSED(throughput_window.back().second, now, std::chrono::milliseconds) > throughput_evaluation_time) {
        throughput_window.pop_back();
    }

    // how many tasks were gathered from the beginning of the window is equal to total number of tasks gathered now
    // minus the total number of tasks gathered at the beginning of the window
    double window_tasks_gathered = global_tasks_gathered - throughput_window.back().first;
    // compute window's throughput
    double window_throughput = window_tasks_gathered / throughput_evaluation_time;

    // get the time elapsed from the beginning of the farm computation
    auto global_elapsed = ELAPSED((*analytics).farm_start_time, now, std::chrono::milliseconds);
    // save throughput and service time measurements
    (*analytics).throughput_points.emplace_back(window_throughput, global_elapsed);
    (*analytics).service_time_points.emplace_back(1 / window_throughput, global_elapsed);

    // update moving average by summing current value and subtracting the oldest value (if available)
    moving_avg_window_throughput_sum += window_throughput;
    moving_avg_window_servicetime_sum += 1 / window_throughput;
    int exitingIndex = (*analytics).throughput_points.size() - moving_avg_window_size - 1;
    if (exitingIndex < 0) return; // stop if we are at the beginning, and we don't have enough points to compute moving average

    // subtract oldest value
    moving_avg_window_throughput_sum -= (*analytics).throughput_points[exitingIndex].first;
    moving_avg_window_servicetime_sum -= (*analytics).service_time_points[exitingIndex].first;
    // save throughput and service time after applying moving window average
    (*analytics).throughput.emplace_back(
            moving_avg_window_throughput_sum / (double) moving_avg_window_size,
            ((*analytics).throughput_points[exitingIndex].second + global_elapsed) / 2
    );
    (*analytics).service_time.emplace_back(
            moving_avg_window_servicetime_sum / (double) moving_avg_window_size,
            ((*analytics).service_time_points[exitingIndex].second + global_elapsed) / 2
    );
}


#endif //AUTONOMICFARM_MONITORINGGATHERER_HPP
