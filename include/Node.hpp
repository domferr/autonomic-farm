#ifndef AUTONOMIC_FARM_NODE_HPP
#define AUTONOMIC_FARM_NODE_HPP

#include <functional>

template <typename InputType>
class Node {
public:
    /**
     * Run this node, which will later process the items sent.
     */
    virtual void run() = 0;

    /**
     * Wait for this node to process all the items sent and to reach the end-of-stream.
     */
    virtual void wait() = 0;

    /**
     * Notify to the node the end-of-stream, disallowing to send more items and allowing the node to end its lifecycle.
     */
    virtual void notify_eos() = 0;

    /**
     * Send to the node an item to be processed. If there are many items, they are processed in FIFO order.
     * @param value the reference to the item to send to the node
     */
    virtual void send(InputType& value) = 0;
};

#endif //AUTONOMIC_FARM_NODE_HPP
