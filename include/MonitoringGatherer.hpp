//
// Created by dferraro on 22/12/23.
//

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

private:
    farm_analytics* analytics;
    size_t tasks_gathered = 0;
    size_t window_size = 4;
    double window_moving_sum = 0;
};

template<typename OutputType>
void MonitoringGatherer<OutputType>::onValue(OutputType& value) {
    // override gatherer thread's function to add monitoring
    auto now = std::chrono::system_clock::now();
    this->onValueFun(value);
    tasks_gathered++;
    // get the time elapsed from the beginning of the farm computation
    auto global_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - (*analytics).farm_start_time).count();
    double curr_throughput = ((double) tasks_gathered / (double) global_elapsed);
    (*analytics).throughput_points.emplace_back(curr_throughput, global_elapsed);

    // compute moving average by subtracting the value exiting the window and adding the value entering the window
    window_moving_sum += curr_throughput;
    if ((*analytics).throughput_points.size() <= window_size) return;

    auto exiting = (*analytics).throughput_points[(*analytics).throughput_points.size() - window_size - 1];
    window_moving_sum -= exiting.first;
    (*analytics).throughput.emplace_back(
        window_moving_sum / (double) window_size,
        (exiting.second + global_elapsed) / 2
    );
}


#endif //AUTONOMICFARM_MONITORINGGATHERER_HPP
