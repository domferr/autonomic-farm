#ifndef AUTONOMICFARM_FFAUTONOMICGATHERER_HPP
#define AUTONOMICFARM_FFAUTONOMICGATHERER_HPP


#include <ff/node.hpp>
#include "MonitoringGatherer.hpp"

template<typename OutputType>
class FFAutonomicGatherer : public ff::ff_minode {
public:
    typedef std::function<void(OutputType *)> SendOutFunType;

    explicit FFAutonomicGatherer(const SendOutFunType &sendOutFun, farm_analytics *analytics)
        : monitoring([&sendOutFun](auto* val) { sendOutFun(val); }, analytics), analytics(analytics) {}

    void *svc(void *in) override;

    void eosnotify(ssize_t id) override;

    void svc_end() override;

private:
    farm_analytics *analytics;
    MonitoringGatherer<OutputType*> monitoring;
};

template<typename OutputType>
void *FFAutonomicGatherer<OutputType>::svc(void *in) {
    auto *in_casted = reinterpret_cast<OutputType*>(in);
    // get the last service time, to understand if it will change or not
    auto prev_size = this->analytics->service_time.size();
    monitoring.onValue(in_casted);
    // set to true if the service time changed
    auto changed = this->analytics->service_time.size() != prev_size;
    auto service_time = this->analytics->service_time.empty() ? 0:this->analytics->service_time.back().first;

    // notify the newest service time to the emitter.
    return new std::pair<double, bool>(service_time, changed); // send feedback to the emitter
}

template<typename OutputType>
void FFAutonomicGatherer<OutputType>::eosnotify(ssize_t id) {
    TRACE("Gatherer eosnotify");
}

template<typename OutputType>
void FFAutonomicGatherer<OutputType>::svc_end() {
    TRACE("Gatherer end");
}


#endif //AUTONOMICFARM_FFAUTONOMICGATHERER_HPP
