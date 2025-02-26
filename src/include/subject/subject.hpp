#pragma once

#include <list>
#include <any>

#include "observable.hpp"


namespace RxLite {

/**
 * @brief A concrete Subject class for a specific type.
 * 
 * A Subject is a special type of Observable that allows values to be multicasted to many Observers.
 * 
 * @tparam T The type of data this subject handles.
 */
template <typename T>
class Subject;

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {
 
class SubjectBase {
protected:
    const std::shared_ptr<std::list<impl::SharedObserver>> subscribers;

    SubjectBase() : subscribers(std::make_shared<std::list<impl::SharedObserver>>()) {}

    template<typename T>
    static Subscription makeSubscription(const Observer<T>& observer,
                                         const std::shared_ptr<std::list<impl::SharedObserver>>& subscribers) {
        auto sharedObserver = std::make_shared<Observer<T>>(observer);
        subscribers->push_back(sharedObserver);
        return impl::SubscriptionFactory(sharedObserver);
    }

    template <typename T>
    void broadcastValue(T& value) const {
        removeInactiveSubscribers();

        for (const auto& subscriber : *this->subscribers) {
            const auto& observer = subscriber->template as<Observer<T>>();
            observer.next(value);
        }
    }

    void broadcastError(const std::exception_ptr& err) {
        removeInactiveSubscribers();

        for (const auto& subscriber : *this->subscribers) {
            subscriber->error(err);
        }
    }

    void broadcastCompletion() const {
        removeInactiveSubscribers();

        for (const auto& subscriber : *this->subscribers) {
            subscriber->complete();
        }

        this->subscribers->clear();
    }

private:
    void removeInactiveSubscribers() const {
        this->subscribers->remove_if([](const impl::SharedObserver& subscriber) {
            return subscriber.use_count() <= 1;
        });
    }
};

} // namespace impl

template <typename T>
class Subject : public impl::SubjectBase, public Observable<T> {
public:
    Subject() : SubjectBase(), Observable<T>(createOnSubscribe()) {}
    
    /**
     * @brief Emit a new value to all subscribers.
     * 
     * This method is called to notify all subscribers of the latest value.
     * 
     * @param value The new value to broadcast to subscribers.
     */
    void next(T value) const {
        broadcastValue(value);
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
        broadcastError(err);
    }

    /**
     * @brief Completes the observable sequence.
     * 
     * This method notifies all subscribers that no more values will be emitted.
     * After calling `complete()`, any further calls to `next()` or `error()` will have no effect.
     */
    void complete() const {
        broadcastCompletion();
    }

private:
    std::function<Subscription(const Observer<T>&)> createOnSubscribe() {
        return [subscribers = this->subscribers](const Observer<T>& observer) -> Subscription {
            return impl::SubjectBase::makeSubscription(observer, subscribers);
        };
    }
};

} // namespace RxLite
