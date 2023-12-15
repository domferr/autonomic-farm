//#define ENABLE_TRACE

#include <vector>
#include <map>
#include <iostream>
#include <cmath>
#include "utimer.hpp"
#include "MonitoredFarm.hpp"


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

farm_analytics farm(size_t pardegree, size_t stream_size, std::vector<size_t>& serviceTimes) {
    MonitoredFarm<size_t, size_t> farm(pardegree, &active_wait, [](auto& ignored) { });
    farm.run();
    for (int stream_index = 0; stream_index < stream_size; ++stream_index) {
        auto service_time_index = (serviceTimes.size() * stream_index) / stream_size;
        farm.send(serviceTimes[service_time_index]);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    farm.notify_eos();
    return farm.wait_and_analytics();
}

#define DEFAULT_SERVICE_TIME_MS 4L

std::vector<size_t> parseServiceTimes(int argc, size_t fromIndex, char *argv[], size_t max_size) {
    if (fromIndex >= argc || max_size == 0) {
        return { DEFAULT_SERVICE_TIME_MS };
    }
    std::vector<size_t> times_ms;
    times_ms.reserve(std::min(argc - fromIndex, max_size));

    while(fromIndex < argc && times_ms.size() <= max_size) {
        times_ms.push_back(atoi(argv[fromIndex]));
        fromIndex++;
    }

    return times_ms;
}

#define NUMBER_OF_DIGITS(integer) (integer == 0 ? 1:(int) std::log10((double) (integer)) + 1)

void print_summary(int pardegree, size_t stream_size, std::vector<size_t>& serviceTimes) {
    std::cout << "Number of workers: " << pardegree << std::endl;
    std::cout << "Stream size: " << stream_size << std::endl;

    std::vector<size_t> stream_end_index;
    stream_end_index.reserve(serviceTimes.size());
    for (size_t service_time_index = 1; service_time_index < serviceTimes.size(); ++service_time_index) {
        // service_times_size : stream_size = service_time_index : stream_item_index
        auto stream_item_index = (stream_size * service_time_index) / serviceTimes.size();
        //if (stream_size % serviceTimes.size() == 0) stream_item_index--;

        stream_end_index.push_back(stream_item_index);
    }
    stream_end_index.push_back(stream_size - 1);

    int max_width = NUMBER_OF_DIGITS(stream_size - 1) * (serviceTimes.size() == 1 ? 1:2);
    for (int index = 0; index < stream_end_index.size(); ++index) {
        auto last_stream_index = index == 0 ? 0: stream_end_index[index - 1]+1;
        size_t mid_stream_index = (last_stream_index + stream_end_index[index]) / 2;
        auto service_time_index = (serviceTimes.size() * mid_stream_index) / stream_size;
        //auto service_time_index = (serviceTimes.size() * stream_end_index[index]) / stream_size;
        auto left_width = NUMBER_OF_DIGITS(last_stream_index);
        std::cout << "Data interval [" << std::setw(left_width) << last_stream_index << ", " << std::setw(max_width - left_width) <<  stream_end_index[index] << "] service time: " << serviceTimes[service_time_index] << "ms" << std::endl;
    }
}

#define INT_ARG(default_val, index) (argc <= index ? default_val : atoi(argv[index]))

int main(int argc, char *argv[]) {
    int pardegree = INT_ARG(2, 1);
    size_t stream_size = INT_ARG(300, 2);
    std::vector<size_t> serviceTimes = parseServiceTimes(argc, 3, argv, stream_size);
    print_summary(pardegree, stream_size, serviceTimes);

    // Run sequential program
    /*std::cout << "Running sequential solution..." << std::flush;
    START(seq_start_time);
    sequential(stream_size, serviceTimes);
    STOP(seq_start_time, seq_elapsed);
    std::cout << "took " << seq_elapsed << "usec" << std::endl;*/

    // Run the same work with a farm of <pardegree> workers
    std::cout << "Running farm..." << std::flush;
    START(farm_start_time);
    auto farm_analytics = farm(pardegree, stream_size, serviceTimes);
    STOP(farm_start_time, farm_elapsed);
    std::cout << "took " << farm_elapsed << "usec" << std::endl;

    farm_analytics.throughput_to_file("csv", "throughput", pardegree, stream_size);
    farm_analytics.throughput_points_to_file("csv", "throughput_points", pardegree, stream_size);
    farm_analytics.arrivaltime_to_file("csv", "arrival_time", pardegree, stream_size);

    return 0;
}