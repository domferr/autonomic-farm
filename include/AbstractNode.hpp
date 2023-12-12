//
// Created by dferraro on 11/12/23.
//

#ifndef ABSTRACTNODE_H
#define ABSTRACTNODE_H

template <typename InputType>
class AbstractNode {
public:
    virtual void run() = 0;
    virtual void wait() = 0;
    virtual void send(InputType value) = 0;
    virtual void notify_eos() = 0;
};

#endif //ABSTRACTNODE_H
