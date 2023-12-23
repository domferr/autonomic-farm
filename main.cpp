//#define ENABLE_TRACE

#include <vector>
#include <map>
#include <iostream>
#include "utimer.hpp"
#include "MonitoredFarm.hpp"
#include "ProgramArgs.hpp"


size_t active_wait(size_t& msec) {
    START(start_time);
    long elapsed_ms = 0;
    while(elapsed_ms < msec) {
        auto now = std::chrono::system_clock::now();
        elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    }
    return msec;
}

void sequential(size_t stream_size, std::vector<size_t>& serviceTimes) {
    for (int stream_index = 0; stream_index < stream_size; ++stream_index) {
        auto service_time_index = (serviceTimes.size() * stream_index) / stream_size;
        active_wait(serviceTimes[service_time_index]);
    }
}

farm_analytics farm(size_t pardegree, size_t stream_size, std::vector<size_t>& serviceTimes, const std::vector<size_t>& arrivalTimes) {
    MonitoredFarm<size_t, size_t> farm(pardegree, &active_wait, [](auto& ignored) { });
    farm.run();
    for (int stream_index = 0; stream_index < stream_size; ++stream_index) {
        auto service_time_index = (serviceTimes.size() * stream_index) / stream_size;
        farm.send(serviceTimes[service_time_index]);
        auto arrival_time_index = (arrivalTimes.size() * stream_index) / stream_size;
        std::this_thread::sleep_for(std::chrono::milliseconds(arrivalTimes[arrival_time_index]));
    }
    farm.notify_eos();
    return farm.wait_and_analytics();
}

int main(int argc, char *argv[]) {
    program_args args = program_args::build(argc, argv);
    std::cout << args << std::endl;

    // Run sequential program
    /*std::cout << "Running sequential solution..." << std::flush;
    START(seq_start_time);
    sequential(stream_size, serviceTimes);
    STOP(seq_start_time, seq_elapsed);
    std::cout << "took " << seq_elapsed << "usec" << std::endl;*/

    // Run the same work with a farm of <num_workers> workers
    std::cout << "Running farm..." << std::flush;
    START(farm_start_time);
    auto farm_analytics = farm(args.num_workers, args.stream_size, args.serviceTimes, args.arrivalTimes);
    STOP(farm_start_time, farm_elapsed, std::chrono::milliseconds);
    std::cout << "took " << farm_elapsed << "usec" << std::endl;

    farm_analytics.throughput_to_file("csv", "throughput", args.num_workers, args.stream_size);
    farm_analytics.throughput_points_to_file("csv", "throughput_points", args.num_workers, args.stream_size);
    farm_analytics.arrivaltime_to_file("csv", "arrival_time", args.num_workers, args.stream_size);
    farm_analytics.servicetime_to_file("csv", "service_time", args.num_workers, args.stream_size);
    farm_analytics.servicetime_points_to_file("csv", "service_time_points", args.num_workers, args.stream_size);

    return 0;
}