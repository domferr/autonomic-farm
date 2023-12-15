#ifndef STREAMQUEUE_H
#define STREAMQUEUE_H


#include <condition_variable>
#include <queue>
#include <optional>

template<typename InputType>
class Stream {
public:
    Stream() = default;
    bool add(InputType& value);
    void eos();
    std::optional<InputType> next();

private:
    std::mutex mutex;
    std::condition_variable cond_empty;
    std::queue<InputType> queue;
    bool eosFlag = false;
};

template<typename InputType>
bool Stream<InputType>::add(InputType& value) {
    {
        std::unique_lock lock(mutex);
        if (eosFlag) return false; // avoid adding new values after end of stream

        queue.push(std::move(value));
    }
    cond_empty.notify_one();

    return true;
}

template<typename InputType>
void Stream<InputType>::eos() {
    {
        std::unique_lock lock(mutex);
        eosFlag = true;
    }
    cond_empty.notify_all();
}

template<typename InputType>
std::optional<InputType> Stream<InputType>::next() {
    std::unique_lock<std::mutex> lock(mutex);
    cond_empty.wait(lock, [&]{ return eosFlag || !queue.empty(); });
    if (eosFlag && queue.empty()) return {};

    auto next_elem = std::optional<InputType>{std::move(queue.front())};
    queue.pop();

    return next_elem;
}

#endif //STREAMQUEUE_H
