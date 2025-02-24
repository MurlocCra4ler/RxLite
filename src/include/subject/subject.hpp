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

    template <typename T>
    void broadcastValue(T& value) const {
        removeInactiveSubscribers();

        for (const auto& subscriber : *this->subscribers) {
            const auto& observer = subscriber->template as<Observer<T>>();
            observer.next(value);
        }
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

private:
    std::function<Subscription(const Observer<T>&)> createOnSubscribe() {
        return [subscribers = this->subscribers](const Observer<T>& observer) -> Subscription {
            auto sharedObserver = std::make_shared<Observer<T>>(observer);
            subscribers->push_back(sharedObserver);
            return impl::SubscriptionFactory(sharedObserver);
        };
    }
};

} // namespace RxLite
