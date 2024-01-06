#ifndef AUTONOMICFARM_AUTONOMICWORKER_HPP
#define AUTONOMICFARM_AUTONOMICWORKER_HPP


#include <chrono>
#include "ThreadedNode.hpp"
#include "trace.hpp"

template <typename InputType>
class AutonomicWorker : public ThreadedNode<InputType> {
public:
    using WorkerFunType = ThreadedNode<InputType>::OnValueFun;
    using OnExitFunType = std::function<void(void)>;

    AutonomicWorker(const WorkerFunType &onValueFun, Stream<InputType>* main_stream, const OnExitFunType& onExitFun)
    : ThreadedNode<InputType>(onValueFun), main_stream(main_stream), onExitFun(onExitFun) {}
    AutonomicWorker(AutonomicWorker&& other) noexcept : ThreadedNode<InputType>(std::move(other)), main_stream(other.main_stream), is_paused(other.is_paused), onExitFun(other.onExitFun) {}

    void send(InputType &ignored) override;
    void notify_eos() override;
    void pause();
    void unpause();
    void setAsReference(std::atomic<size_t> *atomic_worker_service_time);

protected:
    void node_fun() override;

    Stream<InputType>* main_stream;
    std::condition_variable cond_pause;
    std::mutex pause_mutex;
    bool is_paused = false;
    OnExitFunType onExitFun;
    std::atomic<size_t> *atomic_worker_service_time = nullptr;
};

template<typename InputType>
void AutonomicWorker<InputType>::setAsReference(std::atomic<size_t> *new_atomic_worker_service_time) {
    this->atomic_worker_service_time = new_atomic_worker_service_time;
}

/**
 * Function to send a value to an autonomic worker. However, this function won't send anything to the worker since the
 * worker pulls values from a given stream.
 *
 * @tparam InputType the type of the input values
 * @param ignored the value to send to the worker, which will be ignored
 */
template<typename InputType>
void AutonomicWorker<InputType>::send(InputType &ignored) {}

template<typename InputType>
void AutonomicWorker<InputType>::notify_eos() {}

template<typename InputType>
void AutonomicWorker<InputType>::pause() {
    std::unique_lock<std::mutex> lock(pause_mutex);
    is_paused = true;
}

template<typename InputType>
void AutonomicWorker<InputType>::unpause() {
    {
        std::unique_lock<std::mutex> lock(pause_mutex);
        is_paused = false;
    }
    cond_pause.notify_one();
}

template<typename InputType>
void AutonomicWorker<InputType>::node_fun() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(pause_mutex);
            cond_pause.wait(lock, [this](){ return !is_paused; });
        }

        auto next_opt = main_stream->next();
        if (next_opt.has_value()) {
            START(start_time);
            this->onValue(next_opt.value());
            START(end_time);
            if (atomic_worker_service_time != nullptr) {
                *atomic_worker_service_time = ELAPSED(start_time, end_time, std::chrono::milliseconds);
            }
        } else {
            break;
        }
    }
    this->onExitFun();
}


#endif //AUTONOMICFARM_AUTONOMICWORKER_HPP
