#ifndef AUTONOMICFARM_PROGRAMARGS_HPP
#define AUTONOMICFARM_PROGRAMARGS_HPP


#include <vector>
#include <iostream>
#include <ostream>
#include <cmath>
#include <unordered_map>
#include <iomanip>

#define WORKERS_FLAG "-w"
#define MIN_NUM_WORKERS_FLAG "-minw"
#define MAX_NUM_WORKERS_FLAG "-maxw"
#define STREAM_SIZE_FLAG "--stream"
#define TARGET_SERVICE_TIME_FLAG "--target"
#define SERVICE_TIME_FLAG "--service"
#define ARRIVAL_TIME_FLAG "--arrival"
#define DEFAULT_NUM_WORKERS 4
#define DEFAULT_MIN_NUM_WORKERS 2
#define DEFAULT_MAX_NUM_WORKERS 32
#define DEFAULT_STREAM_SIZE 300
#define DEFAULT_TARGET_SERVICE_TIME 8.0
#define DEFAULT_SERVICE_TIME_MS std::vector<size_t>{ 8L }
#define DEFAULT_ARRIVAL_TIME_MS std::vector<size_t>{ 5L }

struct program_args {
public:
    // initial number of workers
    size_t num_workers;
    // minimum number of workers
    size_t min_num_workers;
    // maximum number of workers
    size_t max_num_workers;
    // service time to target
    double target_service_time;
    // number of items in the stream
    size_t stream_size;
    // service times of stream's items
    std::vector<size_t> serviceTimes;
    // arrival times of stream's items
    std::vector<size_t> arrivalTimes;

    /**
     * Build program_args from program arguments.
     */
    static program_args build(int argc, char* argv[]);

    /**
     * Output the given program_args to the given ostream.
     */
    friend std::ostream &operator<<(std::ostream &os, const program_args &args);

private:
    program_args(size_t numWorkers, size_t minNumWorkers, size_t maxNumWorkers, double reqServiceTime, size_t streamSize,
                 const std::vector<size_t> &serviceTimes, const std::vector<size_t> &arrivalTimes)
    : num_workers(numWorkers), min_num_workers(minNumWorkers), max_num_workers(maxNumWorkers), target_service_time(reqServiceTime),
      stream_size(streamSize), serviceTimes(serviceTimes), arrivalTimes(arrivalTimes) {}

    static void proportions_to_stream(std::ostream &os, size_t stream_size, const std::vector<size_t>& data, std::string_view label);
};

#define GET_ARG(type, name, map,  flag, default_value) \
type name = default_value; \
if (map.contains(flag) && !map[flag].empty()) { \
    name = map[flag][0]; \
} \

program_args program_args::build(int argc, char* argv[]) {
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    std::string_view last_flag = "beginning_with_no_flag"; // first arguments, not preceded by a flag
    std::unordered_map<std::string_view, std::vector<size_t>> flags_to_values;

    for (const auto& arg: args) {
        if (arg.starts_with('-')) {
            last_flag = arg;
            flags_to_values[last_flag] = {};
        } else {
            flags_to_values[last_flag].push_back(atoi(arg.data()));
        }
    }

    GET_ARG(size_t, num_workers, flags_to_values, WORKERS_FLAG, DEFAULT_NUM_WORKERS)
    GET_ARG(size_t, min_num_workers, flags_to_values, MIN_NUM_WORKERS_FLAG, DEFAULT_MIN_NUM_WORKERS)
    GET_ARG(size_t, max_num_workers, flags_to_values, MAX_NUM_WORKERS_FLAG, DEFAULT_MAX_NUM_WORKERS)
    GET_ARG(size_t, stream_size, flags_to_values, STREAM_SIZE_FLAG, DEFAULT_STREAM_SIZE)
    GET_ARG(double, required_service_time, flags_to_values, TARGET_SERVICE_TIME_FLAG, DEFAULT_TARGET_SERVICE_TIME)

    auto service_times = flags_to_values.contains(SERVICE_TIME_FLAG) ? flags_to_values[SERVICE_TIME_FLAG]:DEFAULT_SERVICE_TIME_MS;
    if (service_times.size() > stream_size) service_times.resize(stream_size);

    auto arrival_times = flags_to_values.contains(ARRIVAL_TIME_FLAG) ? flags_to_values[ARRIVAL_TIME_FLAG]:DEFAULT_ARRIVAL_TIME_MS;
    if (service_times.size() > stream_size) service_times.resize(stream_size);

    return { num_workers, min_num_workers, max_num_workers, required_service_time, stream_size, service_times, arrival_times };
}

#define NUMBER_OF_DIGITS(integer) (integer == 0 ? 1:(int) std::log10((double) (integer)) + 1)

std::ostream &operator<<(std::ostream &os, const program_args &args) {
    os << "Initial number of nodes: " << args.num_workers;
    os << ", min: " << args.min_num_workers << ", max: " << args.max_num_workers << std::endl;
    os << "Target service time: " << args.target_service_time << std::endl;
    os << "Stream size: " << args.stream_size << std::endl;
    program_args::proportions_to_stream(os, args.stream_size, args.arrivalTimes, "arrival times");
    os << std::endl;
    program_args::proportions_to_stream(os, args.stream_size, args.serviceTimes, "service times");
    return os;
}

void program_args::proportions_to_stream(std::ostream &os, size_t stream_size, const std::vector<size_t>& data, const std::string_view label) {
    std::vector<size_t> stream_end_index;
    stream_end_index.reserve(data.size());
    for (size_t data_index = 1; data_index < data.size(); ++data_index) {
        // service_times_size : args.stream_size = data_index : stream_item_index
        auto stream_item_index = (stream_size * data_index) / data.size();
        stream_end_index.push_back(stream_item_index);
    }
    stream_end_index.push_back(stream_size - 1);

    int max_width = NUMBER_OF_DIGITS(stream_size - 1) * (data.size() == 1 ? 1:2);
    for (int index = 0; index < stream_end_index.size(); ++index) {
        auto last_stream_index = index == 0 ? 0: stream_end_index[index - 1]+1;
        size_t mid_stream_index = (last_stream_index + stream_end_index[index]) / 2;
        auto data_index = (data.size() * mid_stream_index) / stream_size;
        //auto data_index = (data.size() * stream_end_index[index]) / args.stream_size;
        auto left_width = NUMBER_OF_DIGITS(last_stream_index);
        os << "Data interval [" << std::setw(left_width) << last_stream_index << ", " << std::setw(max_width - left_width) << stream_end_index[index] << "] " << label << ": " << data[data_index] << "ms";
        if (index < stream_end_index.size() - 1) os << std::endl;
    }
}

#endif //AUTONOMICFARM_PROGRAMARGS_HPP
