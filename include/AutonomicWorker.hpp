#ifndef AUTONOMICFARM_AUTONOMICWORKER_HPP
#define AUTONOMICFARM_AUTONOMICWORKER_HPP


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

protected:
    void node_fun() override;

    Stream<InputType>* main_stream;
    std::condition_variable cond_pause;
    std::mutex pause_mutex;
    bool is_paused = false;
    OnExitFunType onExitFun;
};

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
            this->onValue(next_opt.value());
        } else {
            break;
        }
    }
    this->onExitFun();
}


#endif //AUTONOMICFARM_AUTONOMICWORKER_HPP
