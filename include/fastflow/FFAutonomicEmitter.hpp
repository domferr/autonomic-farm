#ifndef AUTONOMICFARM_FFAUTONOMICEMITTER_HPP
#define AUTONOMICFARM_FFAUTONOMICEMITTER_HPP


#include "MonitoringGatherer.hpp"
#include "ff/multinode.hpp"
#include "Autonomic.hpp"

template<typename InputType, typename WorkerType>
class FFAutonomicEmitter : public ff::ff_monode_t<InputType>, Autonomic {
public:
    FFAutonomicEmitter(size_t num_workers, size_t minNumWorkers, size_t maxNumWorkers, double target_service_time, farm_analytics *analytics)
    : Autonomic(analytics, num_workers, minNumWorkers, maxNumWorkers, target_service_time) {
        // at the beginning every worker can already receive a new task
        for (size_t i = 0; i < num_workers; ++i) {
            ready_workers.insert(i);
        }

        arrival_time = 0;
        worker_service_time = 0;
    }

    void addWorker(WorkerType* worker) {
        workers.push_back(worker);
    }

    int svc_init() override;

    InputType *svc(InputType *in) override;

    void eosnotify(ssize_t id) override;

    void svc_end() override;

private:
    std::vector<WorkerType*> workers;

    std::deque<InputType*> buffer;
    std::set<size_t> ready_workers;
    std::set<size_t> paused_workers;

    bool eos_flag = false;
    size_t emitted = 0;
    size_t gathered = 0;
    long onthefly = 0;

    // arrival time computation
    long arrival_time;
    std::chrono::system_clock::time_point last_arrival_timepoint;

    long worker_service_time;

    void pauseWorkers(size_t fromIndex, size_t toIndex) override;

    void unpauseWorkers(size_t fromIndex, size_t toIndex) override;

    long getArrivalTime() override {
        return arrival_time;
    }

    long getWorkerServiceTime() override {
        return worker_service_time;
    }
};

template<typename InputType, typename WorkerType>
int FFAutonomicEmitter<InputType, WorkerType>::svc_init() {
    analytics->num_workers.emplace_back(this->num_workers, 0);
    // initialize the arrival time
    last_change = analytics->farm_start_time;
    last_arrival_timepoint = analytics->farm_start_time;

    return 0;
}

template<typename InputType, typename WorkerType>
InputType *FFAutonomicEmitter<InputType, WorkerType>::svc(InputType *in) {
    int channel = this->lb->get_channel_id();
    if (channel == -1) {
        // received new input data
        // update the arrival time
        START(now);
        arrival_time = ELAPSED(last_arrival_timepoint, now, std::chrono::milliseconds);
        if (target_best_service_time) target_service_time = (double) arrival_time;
        last_arrival_timepoint = now;

        emitted++;

        if (ready_workers.empty()) {
            buffer.push_back(in);
        } else {
            size_t worker_index = *ready_workers.begin();
            ready_workers.erase(worker_index);
            this->lb->ff_send_out_to(new WorkerCommand<InputType>(in), worker_index);

            onthefly++;
        }
        //return this->GO_ON;
    } else if (channel < this->lb->get_num_outchannels()) {
        // received feedback from worker
        if (channel < num_workers) {
            if (buffer.empty()) {
                ready_workers.insert(channel);
            } else {
                this->lb->ff_send_out_to(new WorkerCommand<InputType>(buffer.front()), channel);
                buffer.pop_front();

                onthefly++;
            }
        }

        // update worker's service time
        auto *this_worker_service_time = reinterpret_cast<long*>(in);
        worker_service_time = *this_worker_service_time;
        delete this_worker_service_time;
        //return this->GO_ON;
    } else if (channel == this->lb->get_num_outchannels()) {
        gathered++;
        --onthefly;
        // received updated service time from collector
        auto *feedback = reinterpret_cast<std::pair<double, bool>*>(in);
        // if the service time changed, then consider if we need to change the number of workers
        if (feedback->second) {
            double new_service_time = feedback->first;
            onNewServiceTime(new_service_time);
        }
        delete feedback;
        //return this->GO_ON;
    }

    TRACEF("channel %d, emitted %ld, gathered %ld, eos %d, onthefly %ld, buffer.empty %d, ready %ld, paused %ld",
           channel, emitted, gathered, eos_flag, onthefly, buffer.empty(), ready_workers.size(), paused_workers.size());
    if (eos_flag && buffer.empty() && onthefly <= 0) {
        this->broadcast_task(this->EOS);
        unpauseWorkers(0, max_num_workers-1);
        return this->EOS;
    }

    return this->GO_ON;
}

template<typename InputType, typename WorkerType>
void FFAutonomicEmitter<InputType, WorkerType>::svc_end() {
    TRACE("Emitter end");
}

template<typename InputType, typename WorkerType>
void FFAutonomicEmitter<InputType, WorkerType>::eosnotify(ssize_t id) {
    if (id == -1) { // received EOS from input channel
        eos_flag = true;
        TRACEF("Emitter eosnotify: emitted %ld, gathered %ld, eos %d, onthefly %ld, buffer.empty %d, ready %ld, paused %ld",
               emitted, gathered, eos_flag, onthefly, buffer.empty(), ready_workers.size(), paused_workers.size());
        if (buffer.empty() && onthefly <= 0) {
            this->broadcast_task(this->EOS);
            unpauseWorkers(0, max_num_workers-1);
        }
    }
}

template<typename InputType, typename WorkerType>
void FFAutonomicEmitter<InputType, WorkerType>::pauseWorkers(size_t fromIndex, size_t toIndex) {
    for (size_t i = fromIndex; i <= toIndex; ++i) {
        TRACEF("%ld", i);
        //this->ff_send_out_to(this->GO_OUT, i);
        this->ff_send_out_to(new WorkerCommand<InputType>(), i);
        ready_workers.erase(i);
        paused_workers.insert(i);
    }
}

template<typename InputType, typename WorkerType>
void FFAutonomicEmitter<InputType, WorkerType>::unpauseWorkers(size_t fromIndex, size_t toIndex) {
    for (size_t i = fromIndex; i <= toIndex; ++i) {
        TRACEF("%ld", i);
        //this->lb->thaw(i, true);
        workers[i]->unpause();
        ready_workers.insert(i);
        paused_workers.erase(i);
    }
}


#endif //AUTONOMICFARM_FFAUTONOMICEMITTER_HPP
