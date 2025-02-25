#pragma once

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
class BehaviorSubjectBase : public SubjectBase {
protected:
    const std::shared_ptr<T> latestValue;

    BehaviorSubjectBase(T latestValue) : latestValue(std::make_shared<T>(std::move(latestValue))) {}
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
    void next(T value) const {
        this->broadcastValue(value);
        *this->latestValue = value;
    }

private:
    std::function<Subscription(const Observer<T>&)> createOnSubscribe() {
        return [subscribers = this->subscribers, latestValue = this->latestValue]
            (const Observer<T>& observer) -> Subscription {
            observer.next(*latestValue);
            return impl::SubjectBase::makeSubscription(observer, subscribers);
        };
    }
};

} // namespace RxLite
