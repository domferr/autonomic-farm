#ifndef AUTONOMICFARM_PROGRAMARGS_HPP
#define AUTONOMICFARM_PROGRAMARGS_HPP


#include <vector>
#include <iostream>
#include <ostream>
#include <cmath>


#define WORKERS_FLAG "-w"
#define STREAM_SIZE_FLAG "--stream"
#define SERVICE_TIME_FLAG "--service"
#define ARRIVAL_TIME_FLAG "--arrival"
#define DEFAULT_NUM_WORKERS 4
#define DEFAULT_STREAM_SIZE 300
#define DEFAULT_SERVICE_TIME_MS std::vector<size_t>{ 8L }
#define DEFAULT_ARRIVAL_TIME_MS std::vector<size_t>{ 5L }

struct program_args {
public:
    size_t num_workers;
    size_t stream_size;
    std::vector<size_t> serviceTimes;
    std::vector<size_t> arrivalTimes;

    static program_args build(int argc, char* argv[]);
    friend std::ostream &operator<<(std::ostream &os, const program_args &args);

private:
    program_args(size_t numWorkers, size_t streamSize, const std::vector<size_t> &serviceTimes, const std::vector<size_t> &arrivalTimes)
    : num_workers(numWorkers), stream_size(streamSize), serviceTimes(serviceTimes), arrivalTimes(arrivalTimes) {}

    static void proportions_to_stream(std::ostream &os, size_t stream_size, const std::vector<size_t>& data, std::string_view label);
};

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

    size_t num_workers = DEFAULT_NUM_WORKERS;
    if (flags_to_values.contains(WORKERS_FLAG) && !flags_to_values[WORKERS_FLAG].empty()) {
        num_workers = flags_to_values[WORKERS_FLAG][0];
    }

    size_t stream_size = DEFAULT_STREAM_SIZE;
    if (flags_to_values.contains(STREAM_SIZE_FLAG) && !flags_to_values[STREAM_SIZE_FLAG].empty()) {
        stream_size = flags_to_values[STREAM_SIZE_FLAG][0];
    }

    auto service_times = flags_to_values.contains(SERVICE_TIME_FLAG) ? flags_to_values[SERVICE_TIME_FLAG]:DEFAULT_SERVICE_TIME_MS;
    if (service_times.size() > stream_size) service_times.resize(stream_size);

    auto arrival_times = flags_to_values.contains(ARRIVAL_TIME_FLAG) ? flags_to_values[ARRIVAL_TIME_FLAG]:DEFAULT_ARRIVAL_TIME_MS;
    if (service_times.size() > stream_size) service_times.resize(stream_size);

    return { num_workers, stream_size, service_times, arrival_times };
}

#define NUMBER_OF_DIGITS(integer) (integer == 0 ? 1:(int) std::log10((double) (integer)) + 1)

std::ostream &operator<<(std::ostream &os, const program_args &args) {
    os << "Number of workers: " << args.num_workers << std::endl;
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
