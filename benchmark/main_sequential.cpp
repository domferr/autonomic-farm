#include "ProgramArgs.hpp"
#include "utimer.hpp"
#include "benchmark.hpp"

int main(int argc, char *argv[]) {
    program_args args = program_args::build(argc, argv);
    std::cout << args << std::endl;

    // Run sequential program
    std::cout << "Running sequential solution..." << std::flush;
    START(seq_start_time);
    for (int stream_index = 0; stream_index < args.stream_size; ++stream_index) {
        auto service_time_index = (args.serviceTimes.size() * stream_index) / args.stream_size;
        active_wait(args.serviceTimes[service_time_index]);
    }
    STOP(seq_start_time, seq_elapsed, std::chrono::milliseconds);
    std::cout << "took " << seq_elapsed << "msec" << std::endl;

    return 0;
}