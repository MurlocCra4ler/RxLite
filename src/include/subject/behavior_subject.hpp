#pragma once

#include <functional>

#include "subject.hpp"


namespace RxLite {

/**
 * @brief A special type of Subject that replays the last emitted value to new subscribers.
 * 
 * `BehaviorSubject<T>` is a variant of `Subject<T>` that stores the most recent value
 * and emits it immediately to any new subscriber upon subscription. This ensures that
 * late subscribers always receive the latest known value.
 * 
 * @tparam T The type of values emitted by this subject.
 */
template <typename T>
class BehaviorSubject;

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

template <typename T>
class BehaviorSubjectBase : public SubjectBase<T> {
protected:
    const std::shared_ptr<std::reference_wrapper<const T>> latestValue;

    BehaviorSubjectBase(const T& latestValue) : latestValue(std::make_shared<std::reference_wrapper<const T>>(latestValue)) {}
};

} // namespace impl

template <typename T>
class BehaviorSubject : public impl::BehaviorSubjectBase<T>, public Observable<T> {
public:
    BehaviorSubject(T latestValue)
        : impl::BehaviorSubjectBase<T>(std::move(latestValue)), Observable<T>(createOnSubscribe()) {}

    /**
     * @brief Emit a new value to all subscribers.
     * 
     * This method is called to notify all subscribers of the latest value.
     * 
     * @param value The new value to broadcast to subscribers.
     */
    void next(const T& value) const {
        this->broadcastValue(value);
        *this->latestValue = value;
    }

    /**
     * @brief Emits an error to all subscribers.
     * 
     * This method notifies all subscribers that an error has occurred, thereby terminating the observable sequence.
     * After calling `error()`, any subsequent calls to `next()`, `complete()`, or further invocations of `error()`
     * will have no effect.
     * 
     * @param err The exception pointer representing the error to be broadcast to subscribers.
     */
    void error(const std::exception_ptr& err) const {
        this->broadastError(err);
    }

    /**
     * @brief Completes the observable sequence.
     * 
     * This method notifies all subscribers that no more values will be emitted.
     * After calling `complete()`, any further calls to `next()` or `error()` will have no effect.
     */
    void complete() const {
        this->broadcastCompletion();
    }

private:
    std::function<void(const Subscriber<T>&)> createOnSubscribe() {
        return [sharedManager = this->sharedManager, latestValue = this->latestValue]
            (const Subscriber<T>& subscriber) {
            subscriber.next(*latestValue);
            sharedManager->add(subscriber);
        };
    }
};

} // namespace RxLite
