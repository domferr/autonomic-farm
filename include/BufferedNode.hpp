#include <thread>
#include "Stream.hpp"
#include "AbstractNode.hpp"

#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

template <typename InputType>
class BufferedNode : public AbstractNode<InputType> {
public:
    BufferedNode();
    void run() override;
    void wait() override;
    void send(InputType value) override;
    void notify_eos() override;

private:
    std::thread thread;
    Stream<InputType> inputStream;

    virtual void body(InputType value) = 0;
};

template<typename InputType>
BufferedNode<InputType>::BufferedNode() : inputStream(Stream<InputType>()) {
    thread = std::thread(&BufferedNode<InputType>::run, this);
}

template<typename InputType>
void BufferedNode<InputType>::notify_eos() {
    inputStream.eos();
}

template<typename InputType>
void BufferedNode<InputType>::send(InputType value) {
    inputStream.add(value);
}

template<typename InputType>
void BufferedNode<InputType>::run() {
    while (!inputStream.hasNext()) {
        auto value = inputStream.next();
        body(value);
    }
}

template<typename InputType>
void BufferedNode<InputType>::wait() {
    thread.join();
}

#endif //ABSTRACT_NODE_H
