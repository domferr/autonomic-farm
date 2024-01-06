#ifndef AUTONOMICFARM_FFBENCHMARKUTILS_HPP
#define AUTONOMICFARM_FFBENCHMARKUTILS_HPP

#include <thread>
#include "FarmAnalytics.hpp"
#include "utimer.hpp"
#include "ProgramArgs.hpp"
#include "ff/node.hpp"

class FFBenchmarkSource : public ff::ff_node {
public:
    explicit FFBenchmarkSource(program_args &args, farm_analytics *analytics) : args(args), analytics(analytics) {}

    void * svc(void *) override;

    void svc_end() override {
        TRACE("Source end");
    }

private:
    program_args args;
    farm_analytics *analytics;
};

void *FFBenchmarkSource::svc(void *) {
    for (int stream_index = 0; stream_index < args.stream_size; ++stream_index) {
        // given the index of the current stream item, compute its service time by applying the proportion
        auto service_time_index = (args.serviceTimes.size() * stream_index) / args.stream_size;
        // send this work to the farm
        this->ff_send_out(&args.serviceTimes[service_time_index]);
        // track at which time a new item arrived
        STOP(analytics->farm_start_time, time, std::chrono::milliseconds);
        analytics->arrival_time.emplace_back(time);

        // given the index of the current stream item, compute its arrival time by applying the proportion
        auto arrival_time_index = (args.arrivalTimes.size() * stream_index) / args.stream_size;
        // wait some milliseconds before sending the next work
        std::this_thread::sleep_for(std::chrono::milliseconds(args.arrivalTimes[arrival_time_index]));
    }
    return this->EOS;
}


#endif //AUTONOMICFARM_FFBENCHMARKUTILS_HPP
