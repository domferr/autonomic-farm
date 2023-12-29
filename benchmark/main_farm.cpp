//#define ENABLE_TRACE

#include <iostream>
#include "utimer.hpp"
#include "MonitoredFarm.hpp"
#include "ProgramArgs.hpp"
#include "benchmark.hpp"

int main(int argc, char *argv[]) {
    program_args args = program_args::build(argc, argv);
    std::cout << args << std::endl;

    std::cout << "Running farm..." << std::flush;
    START(farm_start_time);

    MonitoredFarm<size_t, size_t> farm(args.num_workers, &active_wait, [](auto& ignored) { });
    auto farm_analytics = benchmark_farm(farm, args.stream_size, args.serviceTimes, args.arrivalTimes);

    STOP(farm_start_time, farm_elapsed, std::chrono::milliseconds);
    std::cout << "took " << farm_elapsed << "msec" << std::endl;

    analytics_to_csv(farm_analytics, args);

    return 0;
}