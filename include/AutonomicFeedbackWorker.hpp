#ifndef AUTONOMICFARM_AUTONOMICFEEDBACKWORKER_HPP
#define AUTONOMICFARM_AUTONOMICFEEDBACKWORKER_HPP


#include <chrono>
#include "AutonomicFeedbackGatherer.hpp"
#include "ThreadedNode.hpp"
#include "trace.hpp"

template <typename InputType, typename OutputType>
class AutonomicFeedbackWorker : public ThreadedNode<InputType> {
public:
    using WorkerFunType = std::function<OutputType(InputType&)>;

    explicit AutonomicFeedbackWorker(const WorkerFunType &workerfun, AutonomicFeedbackGatherer<InputType, OutputType> *feedback_gatherer)
        : ThreadedNode<InputType>([](auto v) {}), workerfun(workerfun), feedback_gatherer(feedback_gatherer) {}
    AutonomicFeedbackWorker(AutonomicFeedbackWorker&& other) noexcept : ThreadedNode<InputType>(std::move(other)) {}

protected:
    WorkerFunType workerfun;
    AutonomicFeedbackGatherer<InputType, OutputType> *feedback_gatherer;

    void onValue(InputType &value) override;

    void node_fun() override;

};

template<typename InputType, typename OutputType>
void AutonomicFeedbackWorker<InputType, OutputType>::onValue(InputType &value) {
    auto res = workerfun(value);
    auto feedback = GathererInput<InputType, OutputType>(res, this);
    this->feedback_gatherer->send(feedback);
}

template<typename InputType, typename OutputType>
void AutonomicFeedbackWorker<InputType, OutputType>::node_fun() {
    // work as normal. It will exit on EOS
    ThreadedNode<InputType>::node_fun();
    // now we reached EOS, propagate to gatherer
    this->feedback_gatherer->notify_eos();
}

#endif //AUTONOMICFARM_AUTONOMICFEEDBACKWORKER_HPP
