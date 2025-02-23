#pragma once

#include "observer.hpp"
#include "subscription.hpp"


namespace RxLite {

/**
 * @brief Represents a sequence of values over time.
 * 
 * An `Observable<T>` is the core building block of RxLite. It defines a data stream
 * that can emit values of type `T`, an error, or a completion signal.
 * Observables can be subscribed to, transformed, and composed using operators.
 * 
 * @tparam T The type of values emitted by this observable.
 */
template <typename T>
class Observable {
public:
    /**
     * @brief Creates an observable with a given subscriber function.
     * 
     * The subscriber function is responsible for emitting values to the provided observer.
     * 
     * @param subscriberFunc A function that takes an `Observer<T>` and defines how values are emitted.
     */
    Observable(std::function<void(const Observer<T>&)> subscriberFunc)
        : subscriberFunc([subscriberFunc] (const Observer<T>& observer) {
            subscriberFunc(observer);
            return std::make_shared<Observer<T>>(observer);
        }) {}

    /**
     * @brief Subscribes an observer to the observable.
     * 
     * When an observer subscribes, the observable starts emitting values to it.
     * The returned `Subscription` can be used to unsubscribe from the observable.
     * 
     * @param subscriber The observer that will receive emitted values.
     * @return Subscription An object representing the active subscription.
     */
    Subscription subscribe(Observer<T> subscriber) {
        class SubscriptionCreator : public Subscription {
        public:
            SubscriptionCreator(impl::SharedObserver sharedObserver) : Subscription(std::move(sharedObserver)) {}
        };

        auto sharedObserver = subscriberFunc(subscriber);
        return SubscriptionCreator(sharedObserver);
    }

    /**
     * @brief Transforms the values emitted by this observable using a mapping function.
     * 
     * The `map` operator applies the given function to each emitted value, 
     * creating a new `Observable<U>` with transformed values.
     * 
     * @tparam U The type of values emitted by the new observable.
     * @param mapper A function that converts values of type `T` to type `U`.
     * @return Observable<U> A new observable that emits transformed values.
     */
    template <typename U>
    Observable<U> map(std::function<U(T)> mapper) {
        std::function<impl::SharedObserver(const Observer<T>&)> mappedSubscriber = [this, mapper](const Observer<T>& observer) {
            Observer<T> intermediateObserver(
                [&mapper, sharedObserver = std::make_shared<Observer<T>>(observer)](const T& t) {
                    Observer<U>& observer = sharedObserver->template as<Observer<U>>();
                    observer.next(mapper(t)); 
                }
            );
    
            auto sharedIntermediateObserver = std::make_shared<Observer<T>>(intermediateObserver);
            subscriber(sharedIntermediateObserver);
            return sharedIntermediateObserver;
        };
    
        return Observable<U>(mappedSubscriber);
    }

protected:
    Observable(std::function<impl::SharedObserver(const Observer<T>&)> subscriberFunc)
        : subscriberFunc(std::move(subscriberFunc)) {}

private:
    std::function<impl::SharedObserver(const Observer<T>&)> subscriberFunc;
};

} // namespace RxLite
