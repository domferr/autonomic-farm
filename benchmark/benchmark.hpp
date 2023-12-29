#ifndef AUTONOMICFARM_BENCHMARK_HPP
#define AUTONOMICFARM_BENCHMARK_HPP

#include <thread>
#include "FarmAnalytics.hpp"
#include "utimer.hpp"
#include "ProgramArgs.hpp"

/**
 * Simulate busy work by looping for msec time. The return value is needed by the farm itself since the worker has to
 * produce some new value
 * @param msec how many milliseconds this work will take doing nothing
 * @return the number of milliseconds of this work
 */
size_t active_wait(size_t& msec) {
    START(start_time);
    long elapsed_ms = 0;
    while(elapsed_ms < msec) {
        auto now = std::chrono::system_clock::now();
        elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    }
    return msec;
}

/**
 * Run a benchmark of a given farm. Given the stream size, the service and arrival times, the given farm is run and
 * the stream is sent to the farm (according to the given arrival times). To simulate busy work, the work sent to the
 * farm will not compute anything useful, but instead will busy waiting for a given amount of milliseconds (according
 * to the given service times).
 * @tparam FarmType the type of farm to benchmark. It must support the wait_and_analytics() method
 * @param farm reference to the farm to benchmark
 * @param stream_size how many items the stream has
 * @param serviceTimes the service times of the stream (milliseconds)
 * @param arrivalTimes the arrival times of the stream (milliseconds)
 * @return the result of the benchmark
 */
template <typename FarmType>
farm_analytics benchmark_farm(FarmType& farm, size_t stream_size,
                              std::vector<size_t>& serviceTimes, const std::vector<size_t>& arrivalTimes) {
    farm.run();
    for (int stream_index = 0; stream_index < stream_size; ++stream_index) {
        // given the index of the current stream item, compute its service time by applying the proportion
        auto service_time_index = (serviceTimes.size() * stream_index) / stream_size;
        // send this work to the farm
        farm.send(serviceTimes[service_time_index]);

        // given the index of the current stream item, compute its arrival time by applying the proportion
        auto arrival_time_index = (arrivalTimes.size() * stream_index) / stream_size;
        // wait some milliseconds before sending the next work
        std::this_thread::sleep_for(std::chrono::milliseconds(arrivalTimes[arrival_time_index]));
    }
    farm.notify_eos();
    // wait for the farm to end and return the benchmark results
    return farm.wait_and_analytics();
}

/**
 * Write benchmark result to new files into the csv folder
 * @param analytics the benchmark
 * @param args the program arguments
 */
void analytics_to_csv(farm_analytics &analytics, program_args& args) {
    analytics.throughput_to_file("csv", "throughput", args.num_workers, args.stream_size);
    analytics.throughput_points_to_file("csv", "throughput_points", args.num_workers, args.stream_size);
    analytics.arrivaltime_to_file("csv", "arrival_time", args.num_workers, args.stream_size);
    analytics.servicetime_to_file("csv", "service_time", args.num_workers, args.stream_size);
    analytics.servicetime_points_to_file("csv", "service_time_points", args.num_workers, args.stream_size);
    analytics.num_workers_to_file("csv", "num_workers", args.num_workers, args.stream_size);
}

#endif //AUTONOMICFARM_BENCHMARK_HPP
