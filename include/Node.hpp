#ifndef AUTONOMIC_FARM_NODE_HPP
#define AUTONOMIC_FARM_NODE_HPP

#include <functional>

template <typename InputType>
class Node {
public:
    virtual void run() = 0;
    virtual void wait() = 0;
    virtual void notify_eos() = 0;
    virtual void send(InputType& value) = 0;
};

#endif //AUTONOMIC_FARM_NODE_HPP
