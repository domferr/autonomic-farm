
#include <iostream>
#include "fastflow/FFMonitoringFarm.hpp"
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

    std::cout << "Running fastflow monitored farm..." << std::flush;
    START(farm_start_time);
    farm_analytics analytics;
    FFBenchmarkSource sourceOfStream(args, &analytics);
    FFMonitoringFarm<size_t, size_t> ff_farm(args.num_workers, workerfun, [](auto* ignored) {}, &analytics);

    ff_farm.run(sourceOfStream);
    ff_farm.wait();

    STOP(farm_start_time, farm_elapsed, std::chrono::milliseconds);
    std::cout << "took " << farm_elapsed << "msec" << std::endl;

    analytics_to_csv(analytics, args);

    return 0;
}