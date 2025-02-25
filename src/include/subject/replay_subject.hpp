#pragma once

#include <deque>

#include "subject.hpp"


namespace RxLite {

/**
 * @brief A variant of Subject that replays previously emitted values to new subscribers.
 * 
 * ReplaySubject stores the last `bufferSize` values and emits them to any new subscriber
 * before subscribing it to future values. This allows late subscribers to receive past values.
 *
 * @tparam T The type of values emitted by the ReplaySubject.
 */
template <typename T>
class ReplaySubject;

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

template <typename T>
class ReplaySubjectBase : public SubjectBase {
protected:
    const std::shared_ptr<std::deque<T>> history;
    const size_t bufferSize;

    ReplaySubjectBase(size_t bufferSize)
        : history(std::make_shared<std::deque<T>>()), bufferSize(bufferSize) {}
};

} // namespace impl

template <typename T>
class ReplaySubject : public impl::ReplaySubjectBase<T>, public Observable<T> {
public:
    /**
     * @brief Constructs a ReplaySubject with an optional buffer size.
     * 
     * @param bufferSize The maximum number of values to retain in the replay buffer. 
     *                   If set to `0`, all values are stored indefinitely.
     */
    ReplaySubject(size_t bufferSize = 0)
        : impl::ReplaySubjectBase<T>(), Observable<T>(createOnSubscribe()) {}

    /**
     * @brief Emit a new value to all subscribers.
     * 
     * This method is called to notify all subscribers of the latest value.
     * 
     * @param value The new value to broadcast to subscribers.
     */
    void next(T value) const {
        this->broadcastValue(value);

        if (this->bufferSize && this->history->size() == this->bufferSize) {
            this->history->erase(this->history->begin());
        }

        this->history->push_back(value);
    }

private:
    std::function<Subscription(const Observer<T>&)> createOnSubscribe() {
        return [subscribers = this->subscribers, history = this->history]
            (const Observer<T>& observer) -> Subscription {
            for (const T& value : *history) {
                observer.next(value);
            }
            
            return impl::SubjectBase::makeSubscription(observer, subscribers);
        };
    }
};

} // namespace RxLite
