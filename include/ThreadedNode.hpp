#ifndef ABSTRACTNODE_H
#define ABSTRACTNODE_H

#include <thread>
#include "Node.hpp"
#include "Stream.hpp"

template <typename InputType>
class ThreadedNode : public Node<InputType> {
public:
    typedef std::function<void(InputType&)> OnValueFun;

    ThreadedNode() : onValueFun([](auto ignored){}) {}
    explicit ThreadedNode(const OnValueFun& onValueFun) : onValueFun(onValueFun) {}
    ThreadedNode(const ThreadedNode& other_node) : onValueFun(other_node.onValueFun) {}

    void wait() override;
    void run() override;
    void send(InputType& value) override;
    void notify_eos() override;

protected:
    void node_fun();
    virtual void onValue(InputType& value) { this->onValueFun(value); }
    virtual void onEOS() { }
    std::thread thread;
    Stream<InputType> inputStream;
    OnValueFun onValueFun;
};

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
    onEOS();
}

#endif //ABSTRACTNODE_H
