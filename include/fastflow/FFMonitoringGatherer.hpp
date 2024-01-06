#ifndef AUTONOMICFARM_FFMONITORINGGATHERER_HPP
#define AUTONOMICFARM_FFMONITORINGGATHERER_HPP

#include <ff/node.hpp>
#include "MonitoringGatherer.hpp"
#include "ff/multinode.hpp"

template<typename OutputType>
class FFMonitoringGatherer : public ff::ff_minode_t<OutputType, OutputType> {
public:
    typedef std::function<void(OutputType *)> SendOutFunType;

    explicit FFMonitoringGatherer(const SendOutFunType &sendOutFun, farm_analytics *analytics)
        : monitoring([](auto ign) {}, analytics), lb(lb), sendOutFun(sendOutFun) {}

    OutputType *svc(OutputType *in) override;

private:
    MonitoringGatherer<OutputType> monitoring;
    const SendOutFunType &sendOutFun;
    ff::ff_loadbalancer *lb;
};

template<typename OutputType>
OutputType *FFMonitoringGatherer<OutputType>::svc(OutputType *in) {
    monitoring.onValue(*in);
    sendOutFun(in);
    return this->GO_ON;
}


#endif //AUTONOMICFARM_FFMONITORINGGATHERER_HPP
