#ifndef AUTONOMICFARM_AUTONOMICFEEDBACKGATHERER_HPP
#define AUTONOMICFARM_AUTONOMICFEEDBACKGATHERER_HPP


#include <set>
#include "MonitoringGatherer.hpp"

template <typename InputType, typename OutputType>
struct GathererInput {
    OutputType out;
    Node<InputType>* worker;

    GathererInput(OutputType &out, Node<InputType>* worker) : out(out), worker(worker) {}
};

/**
 * A special monitoring gatherer that notifies to autonomic worker pool the most updated service time.
 */
template <typename InputType, typename OutputType>
class AutonomicFeedbackGatherer : public MonitoringGatherer<GathererInput<InputType, OutputType>> {
public:
    using GathererFunType = MonitoringGatherer<OutputType>::GathererFunType;
    typedef std::function<void(void)> OnEosFun;

    AutonomicFeedbackGatherer(const GathererFunType &onValueFun, size_t num_workers, size_t min_num_workers,
                              size_t max_num_workers, double target_service_time, farm_analytics* analytics, OnEosFun onEosFun);

    void onValue(GathererInput<InputType, OutputType> &value) override;

    void send(GathererInput<InputType, OutputType> &value) override {
        MonitoringGatherer<GathererInput<InputType, OutputType>>::send(value);
    }

    void send_input(InputType &value);

    void notify_eos() override;

private:
    Stream<InputType> main_stream;
    std::deque<InputType> data_buffer;
    OnEosFun onEosFun;
    size_t eos_got_count = 0;

    size_t max_num_workers;
    std::set<Node<InputType>*> active_workers;
    std::set<Node<InputType>*> ready_workers;

    // arrival time computation
    std::atomic<long> atomic_arrival_time;
    std::chrono::system_clock::time_point last_arrival_timepoint;

    const long num_workers_sensitivity_ms = 100; //ms
};

template<typename InputType, typename OutputType>
AutonomicFeedbackGatherer<InputType, OutputType>::AutonomicFeedbackGatherer(const GathererFunType &onValueFun, size_t num_workers,
    size_t min_num_workers, size_t max_num_workers, double target_service_time, farm_analytics* analytics, OnEosFun onEosFun)
: MonitoringGatherer<GathererInput<InputType, OutputType>>([&onValueFun](auto in) { onValueFun(in.out); }, analytics), onEosFun(onEosFun), max_num_workers(max_num_workers) {}

template<typename InputType, typename OutputType>
void AutonomicFeedbackGatherer<InputType, OutputType>::onValue(GathererInput<InputType, OutputType> &value) {
    // get the last service time, to understand if it will change or not
    auto prev_size = this->analytics->service_time.size();
    MonitoringGatherer<GathererInput<InputType, OutputType>>::onValue(value);
    START(now);
    long first_global_elapsed = ELAPSED(this->analytics->farm_start_time, now, std::chrono::milliseconds);
    ready_workers.insert(value.worker);
    active_workers.erase(value.worker);
    if (this->analytics->num_workers.back().first != active_workers.size()) {
        this->analytics->num_workers.emplace_back(active_workers.size(), first_global_elapsed);
    }
    auto is_eos = false;
    auto next_opt = main_stream.next(&is_eos);
    if (!next_opt.has_value() && !is_eos && ready_workers.size() == max_num_workers) {
        next_opt = main_stream.next();
        is_eos = !next_opt.has_value();
    }

    if (is_eos && !next_opt.has_value()) {
        if (is_eos && active_workers.empty()) {
            this->inputStream.eos();
            for (const auto &worker: ready_workers) worker->notify_eos();
        }
    } else if (next_opt.has_value()) {
        InputType next = next_opt.value();
        auto *next_ready_worker = *ready_workers.begin();
        next_ready_worker->send(next);
        active_workers.insert(next_ready_worker);
        ready_workers.erase(next_ready_worker);
    }

    long global_elapsed = ELAPSED(this->analytics->farm_start_time, now, std::chrono::milliseconds);
    if (global_elapsed - first_global_elapsed < num_workers_sensitivity_ms) this->analytics->num_workers.pop_back();
    this->analytics->num_workers.emplace_back(active_workers.size(), global_elapsed);
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackGatherer<InputType, OutputType>::send_input(InputType &value) {
    START(now);

    main_stream.add(value);

    auto elapsed = ELAPSED(last_arrival_timepoint, now, std::chrono::milliseconds);
    atomic_arrival_time = elapsed;
    //if (target_best_service_time) target_service_time = (double) elapsed;
    last_arrival_timepoint = now;
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackGatherer<InputType, OutputType>::notify_eos() {
    main_stream.eos();
}


#endif //AUTONOMICFARM_AUTONOMICFEEDBACKGATHERER_HPP
