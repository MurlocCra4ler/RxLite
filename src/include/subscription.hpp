#pragma once

#include <vector>

#include "observer.hpp"


namespace RxLite {

using TeardownLogic = std::function<void()>;

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
    Subscription() = default;

    ~Subscription() {
        if (sharedExecution && sharedExecution.use_count() == 1) {
            if (sharedExecution && sharedExecution->running.exchange(false)) {
                sharedExecution->subscriber.unsubscribe();
                sharedExecution->teardownLogic(); 
            }
        }
    }

    /**
     * @brief Adds a child subscription.
     * 
     * This allows managing multiple subscriptions under a single parent subscription.
     * When the parent subscription is unsubscribed, all child subscriptions are also disposed.
     * 
     * @param subscription The child subscription to add.
     */
    void add(Subscription&& subscription) {
        subscriptions.push_back(std::move(subscription));
    }

    /**
     * @brief Disposes of the subscription and releases resources.
     * 
     * This method disconnects the associated observer (if any) and clears all child subscriptions.
     * After calling `unsubscribe`, the subscription is no longer active.
     */
    void unsubscribe() {
        subscriptions.clear();
        
        if (sharedExecution && sharedExecution->running.exchange(false)) {
            sharedExecution->subscriber.unsubscribe();
            sharedExecution->teardownLogic();
        }
    }

protected:
    template <typename T>
    Subscription(Subscriber<T> subscriber, TeardownLogic teardownLogic)
        : sharedExecution(std::make_shared<Execution>(std::move(subscriber), std::move(teardownLogic), true)) {}

private:
    struct Execution {
        const impl::SubscriberBase subscriber;
        const TeardownLogic teardownLogic;
        std::atomic<bool> running;

        Execution(impl::SubscriberBase subscriber, TeardownLogic teardownLogic, bool running)
            : subscriber(std::move(subscriber)), teardownLogic(std::move(teardownLogic)), running(running) {}
    };

    std::shared_ptr<Execution> sharedExecution;
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
    template <typename T>
    SubscriptionFactory(Subscriber<T> subscriber, TeardownLogic teardownLogic)
        : Subscription(std::move(subscriber), std::move(teardownLogic)) {} 
};

} // namespace impl

} // namespace RxLite
