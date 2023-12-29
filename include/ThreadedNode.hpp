#ifndef THREADEDNODE_H
#define THREADEDNODE_H

#include <thread>
#include "Node.hpp"
#include "Stream.hpp"

/**
 * An implementation of the Node class that processes input items from an independent thread.
 * @tparam InputType the type of the input items
 */
template <typename InputType>
class ThreadedNode : public Node<InputType> {
public:
    // type of the function executed by the given thread to process an input item
    typedef std::function<void(InputType&)> OnValueFun;

    explicit ThreadedNode(const OnValueFun& onValueFun) : onValueFun(onValueFun) {}
    ThreadedNode(const ThreadedNode& other_node) : onValueFun(other_node.onValueFun) {}
    ThreadedNode(ThreadedNode&& other) noexcept : onValueFun(other.onValueFun), thread(std::move(other.thread)) {}

    /**
     * Wait for this thread to finish
     */
    void wait() override;

    /**
     * Run this thread
     */
    void run() override;

    /**
     * Send to the node an item to be processed. If there are many items, they are processed in FIFO order.
     * @param value the reference to the item to send to the node
     */
    void send(InputType& value) override;

    /**
     * Send many items to the node.
     * @tparam Iterator the iterator to iterate through the items to send
     * @param begin iterator pointing to the first item to be sent
     * @param end iterator pointing to the item after the last item to be sent
     */
    template<typename Iterator>
    void send(Iterator begin, Iterator end);

    /**
     * Notify the end-of-stream to this node. It allows the thread to finish.
     */
    void notify_eos() override;

protected:
    // thread function
    virtual void node_fun();
    // function executed by the given thread to process an input item
    virtual void onValue(InputType& value);

    std::thread thread;
    // store input items into an input stream
    Stream<InputType> inputStream;
    // function executed by the given thread to process an input item
    OnValueFun onValueFun;
};

template<typename InputType>
template<typename Iterator>
void ThreadedNode<InputType>::send(Iterator begin, Iterator end) {
    inputStream.add_all(begin, end);
}

template<typename InputType>
void ThreadedNode<InputType>::wait() {
    thread.join();
}

template<typename InputType>
void ThreadedNode<InputType>::run() {
    thread = std::thread(&ThreadedNode::node_fun, this);
}

template<typename InputType>
void ThreadedNode<InputType>::send(InputType& value) {
    inputStream.add(value);
}

template<typename InputType>
void ThreadedNode<InputType>::notify_eos() {
    inputStream.eos();
}

template<typename InputType>
void ThreadedNode<InputType>::node_fun() {
    do {
        auto next_opt = inputStream.next();
        if (next_opt.has_value()) {
            onValue(next_opt.value());
        } else {
            break;
        }
    } while (true);
}

template<typename InputType>
void ThreadedNode<InputType>::onValue(InputType &value) {
    this->onValueFun(value);
}

#endif //THREADEDNODE_H
