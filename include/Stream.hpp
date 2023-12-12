//
// Created by dferraro on 11/12/23.
//

#ifndef STREAMQUEUE_H
#define STREAMQUEUE_H

template <typename Value>
class Stream {
public:
    void add(Value value);
    void eos();
    bool hasNext();
    Value next();
};

#endif //STREAMQUEUE_H
