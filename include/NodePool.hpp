#ifndef AUTONOMICFARM_NODEPOOL_HPP
#define AUTONOMICFARM_NODEPOOL_HPP

#include <vector>
#include "trace.hpp"
#include "Node.hpp"

/**
 * A NodePool is a group of nodes of a specified type.
 * 
 * @tparam InputType the input type of each node 
 * @tparam NodeType the type of each node
 */
template <typename InputType, typename NodeType>
class NodePool : public Node<InputType> {
public:
    /**
     * Constructs a node pool of <num_nodes> nodes. It will call nodes' constructor by passing the given arguments.
     * @param num_nodes the number of nodes of this node pool
     * @param args the arguments to be passed to each node's constructor
     */
    template <typename... Args>
    explicit NodePool(size_t num_nodes, Args... args);

    /**
     * Run all the nodes, which will later process the items sent.
     */
    void run() override;

    /**
     * Wait for all the nodes to process all the items sent and to reach the end-of-stream. 
     */
    void wait() override;

    /**
     * Notify to all the nodes the end-of-stream, disallowing to send more items and allowing the nodes to end their lifecycle.
     */
    void notify_eos() override;

    /**
     * Send to one of the nodes the node an item to be processed, following round-robin policy.
     * @param value the reference to the item to send to the selected node
     */
    void send(InputType& value) override;

protected:
    NodePool() = default;

    /**
     * Construct the nodes by calling their constructor and passing the given arguments.
     * @tparam Args the type of arguments to pass to each node's constructor
     * @param args the arguments to pass to each node's constructor
     */
    template <typename... Args>
    void init(size_t num_nodes, Args... args);

    std::vector<NodeType> nodes;
    size_t worker_index = 0;
};

template<typename InputType, typename NodeType>
template <typename... Args>
NodePool<InputType, NodeType>::NodePool(size_t num_nodes, Args... args) {
    this->init(num_nodes, args...);
}

template<typename InputType, typename NodeType>
template <typename... Args>
void NodePool<InputType, NodeType>::init(size_t num_nodes, Args... args) {
    nodes.clear();
    nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        TRACEF("Init worker %d/%lu", (i+1), num_nodes);
        nodes.emplace_back(args...);
    }
}

template<typename InputType, typename NodeType>
void NodePool<InputType, NodeType>::run() {
    for (int i = 0; i < nodes.size(); ++i) {
        nodes[i].run();
    }
}

template<typename InputType, typename NodeType>
void NodePool<InputType, NodeType>::wait() {
    for (int i = 0; i < nodes.size(); ++i) {
        nodes[i].wait();
    }
}

template<typename InputType, typename NodeType>
void NodePool<InputType, NodeType>::notify_eos() {
    for (int i = 0; i < nodes.size(); ++i) {
        nodes[i].notify_eos();
    }
}

template<typename InputType, typename NodeType>
void NodePool<InputType, NodeType>::send(InputType &value) {
    TRACEF("Send to worker %lu (round-robin)", worker_index);
    nodes[worker_index].send(value);
    worker_index = (worker_index+1) % nodes.size();
}


#endif //AUTONOMICFARM_NODEPOOL_HPP
