#ifndef AUTONOMICFARM_FFAUTONOMICWORKER_HPP
#define AUTONOMICFARM_FFAUTONOMICWORKER_HPP


#include <ff/ff.hpp>
#include <ff/farm.hpp>

template <typename InputType>
struct WorkerCommand {
    InputType* task;
    bool pause = false;

    explicit WorkerCommand(InputType *task) : task(task), pause(false) {}
    WorkerCommand() : task(nullptr), pause(true) {}
};

template <typename InputType, typename OutputType>
class FFAutonomicWorker : public ff::ff_monode_t<WorkerCommand<InputType>, OutputType> {
public:
    typedef std::function<OutputType*(InputType *)> WorkerFunType;

    explicit FFAutonomicWorker(const WorkerFunType &fun) : fun(fun) {}

    int svc_init() override;

    OutputType *svc(WorkerCommand<InputType> *cmd) override;

    void eosnotify(ssize_t id) override;

    void svc_end() override;

    void unpause();
private:
    WorkerFunType fun;

    std::mutex mutex;
    std::condition_variable cond_pause;
    bool is_paused = false;
};

template<typename InputType, typename OutputType>
int FFAutonomicWorker<InputType, OutputType>::svc_init() {
    TRACEF("Worker %ld init", this->get_my_id());
    return 0;
}

template<typename InputType, typename OutputType>
OutputType *FFAutonomicWorker<InputType, OutputType>::svc(WorkerCommand<InputType> *cmd) {
    TRACEF("Worker %ld svc", this->get_my_id());
    if (cmd->pause) {
        TRACEF("Worker %ld going to sleep", this->get_my_id());
        std::unique_lock<std::mutex> lock(mutex);
        is_paused = true;
        cond_pause.wait(lock, [this] { return !this->is_paused; });
        TRACEF("Worker %ld woke up", this->get_my_id());
    } else {
        auto task = cmd->task;
        START(now);
        auto result = fun(task);
        STOP(now, service_time, std::chrono::milliseconds);
        this->ff_send_out_to(result, 1); // send to the gatherer
        this->ff_send_out_to(new long(service_time), 0); // send feedback to emitter
        TRACEF("Worker %ld svc end", this->get_my_id());
    }

    return this->GO_ON;
}

template<typename InputType, typename OutputType>
void FFAutonomicWorker<InputType, OutputType>::eosnotify(ssize_t id) {
    if (id == -1) {
        TRACEF("Worker %ld eosnotify", this->get_my_id());
    }
}

template<typename InputType, typename OutputType>
void FFAutonomicWorker<InputType, OutputType>::svc_end() {
    TRACEF("Worker %ld end", this->get_my_id());
}

template<typename InputType, typename OutputType>
void FFAutonomicWorker<InputType, OutputType>::unpause() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        is_paused = false;
    }
    cond_pause.notify_one();
}


#endif //AUTONOMICFARM_FFAUTONOMICWORKER_HPP
