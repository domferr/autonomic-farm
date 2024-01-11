#ifndef STREAMQUEUE_H
#define STREAMQUEUE_H


#include <condition_variable>
#include <queue>
#include <optional>

template<typename InputType>
class Stream {
public:
    /**
     * Construct an empty stream
     */
    Stream() = default;

    /**
     * Add the given value to the stream. Applies zero-copy communication. If the end-of-stream was sent before, this
     * method won't add and will return false.
     *
     * @param value the value to add to the stream. It is moved and not copied. The value must be movable.
     * @return true if the add was allowed, false otherwise
     */
    bool add(InputType& value);

    /**
     * Adds many values to the stream. The values are accessed from begin to end, by following the given iterators. It
     * is equivalent to call add(value) method many times but this is more efficient since the lock is acquired once.
     * @tparam Iterator the iterator to iterate through the values to add
     * @param begin begin iterator representing the first element to add
     * @param end end iterator representing the last element to not be added
     * @return true if it was allowed to add all the elements, false otherwise
     */
    template<typename Iterator>
    bool add_all(Iterator begin, Iterator end);

    /**
     * Send end-of-stream. After this method returns, all the additions will be disallowed.
     */
    void eos();

    /**
     * Pop the next element from the stream, following the FIFO order. An empty optional is returned when this method is
     * called on an empty stream that reached end-of-stream.
     * @return and optional containing the next element, if available, and empty optional if the stream reached the
     * end-of-stream
     */
    std::optional<InputType> next();

    std::optional<InputType> next(bool* is_eos);

private:
    std::mutex mutex;
    std::condition_variable cond_empty;
    std::deque<InputType> queue;
    bool eosFlag = false;
};

template<typename InputType>
bool Stream<InputType>::add(InputType& value) {
    {
        std::unique_lock lock(mutex);
        if (eosFlag) return false; // avoid adding new values after end of stream
        // zero-copy communication
        queue.push_back(std::move(value));
    }
    cond_empty.notify_one();

    return true;
}

template<typename InputType>
template<typename Iterator>
bool Stream<InputType>::add_all(Iterator begin, Iterator end) {
    {
        std::unique_lock lock(mutex);
        if (eosFlag) return false; // avoid adding new values after end of stream

        while (begin != end) {
            queue.push_back(*begin);
            begin++;
        }
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
    queue.pop_front();

    return next_elem;
}

template<typename InputType>
std::optional<InputType> Stream<InputType>::next(bool* is_eos) {
    std::unique_lock<std::mutex> lock(mutex);

    if (queue.empty()) {
        *is_eos = eosFlag;
        return {};
    }
    *is_eos = false;
    auto next_elem = std::optional<InputType>{std::move(queue.front())};
    queue.pop_front();

    return next_elem;
}

#endif //STREAMQUEUE_H
