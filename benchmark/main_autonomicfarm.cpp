#include "ProgramArgs.hpp"
#include "utimer.hpp"
#include "benchmark.hpp"
#include "AutonomicFarm.hpp"

int main(int argc, char *argv[]) {
    program_args args = program_args::build(argc, argv);
    std::cout << args << std::endl;

    std::cout << "Running autonomic farm..." << std::flush;
    START(farm_start_time);
    AutonomicFarm<size_t, size_t> autonomicFarm(args.num_workers, args.min_num_workers, args.max_num_workers,
                                                args.target_service_time, &active_wait, [](auto& ignored) { });
    auto farm_analytics = benchmark_farm(autonomicFarm, args.stream_size, args.serviceTimes, args.arrivalTimes);
    STOP(farm_start_time, farm_elapsed, std::chrono::milliseconds);
    std::cout << "took " << farm_elapsed << "msec" << std::endl;

    analytics_to_csv(farm_analytics, args);

    return 0;
}