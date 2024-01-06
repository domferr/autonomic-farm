//#define ENABLE_TRACE

#include <iostream>
#include <fastflow/FFAutonomicFarm.hpp>
#include "ProgramArgs.hpp"
#include "benchmark.hpp"
#include "ffbenchmarkutils.hpp"

int main(int argc, char *argv[]) {
    program_args args = program_args::build(argc, argv);
    std::cout << args << std::endl;

    auto workerfun = [](auto *val) {
        active_wait(*val);
        return val;
    };

    std::cout << "Running fastflow autonomic farm..." << std::flush;
    START(farm_start_time);
    farm_analytics analytics;
    FFBenchmarkSource sourceOfStream(args, &analytics);
    FFAutonomicFarm<size_t, size_t> ff_autonomicFarm(args.num_workers, args.min_num_workers, args.max_num_workers,
        args.target_service_time, workerfun, [](auto* ignored) { }, &analytics);

    ff_autonomicFarm.run(sourceOfStream);
    ff_autonomicFarm.wait();

    STOP(farm_start_time, farm_elapsed, std::chrono::milliseconds);
    std::cout << "took " << farm_elapsed << "msec" << std::endl;

    analytics_to_csv(analytics, args);


    return 0;
}