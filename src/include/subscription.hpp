#pragma once

#include <vector>

#include "observer.hpp"


namespace RxLite {

/**
 * @brief Represents a disposable resource, such as the execution of an Observable.
 * 
 * A `Subscription` manages the lifecycle of an observer’s connection to an observable.
 * It provides an `unsubscribe` method to dispose of the resource and stop receiving notifications.
 * 
 * A `Subscription` may also contain child subscriptions, allowing grouped disposal of multiple
 * subscriptions at once.
 */
class Subscription {
public:
    /**
     * @brief Constructs an empty Subscription.
     * 
     * An empty subscription does not hold any observer or child subscriptions.
     */
    Subscription() {}

    /**
     * @brief Adds a child subscription.
     * 
     * This allows managing multiple subscriptions under a single parent subscription.
     * When the parent subscription is unsubscribed, all child subscriptions are also disposed.
     * 
     * @param subscription The child subscription to add.
     */
    void add(Subscription subscription) {
        subscriptions.push_back(subscription);
    }

    /**
     * @brief Disposes of the subscription and releases resources.
     * 
     * This method disconnects the associated observer (if any) and clears all child subscriptions.
     * After calling `unsubscribe`, the subscription is no longer active.
     */
    void unsubscribe() {
        if (subscriber) {
            subscriber = nullptr;
        }

        subscriptions.clear();
    }

protected:
    Subscription(impl::SharedObserver subscriber) : subscriber(subscriber) {} 

private:
    impl::SharedObserver subscriber;
    std::vector<Subscription> subscriptions;
};

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

class SubscriptionFactory : public Subscription {
public:
    SubscriptionFactory(impl::SharedObserver subscriber) : Subscription(subscriber) {} 
};

} // namespace impl

} // namespace RxLite
