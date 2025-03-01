#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <atomic>

namespace RxLite {

/**
 * @brief A concrete Observer class for a specific type.
 * 
 * The Observer class represents an observer that handles a specific type
 * of data (e.g., int, float). Observers are simply a set of callbacks,
 * one for each type of notification delivered by the Observable:
 * - `next` for handling emitted values
 * - `error` for handling errors
 * - `complete` for handling completion signals.
 *
 * The observer pattern is used to allow an object to react to changes in
 * another object (the observable) without tightly coupling the two.
 * 
 * @tparam T The type of data this observer handles.
 */
template <typename T>
class Observer;

template <typename T>
class Subscriber;

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

class ObserverBase {
public:
    virtual ~ObserverBase() = default;

    template <typename Derived>
    Derived& as() {
        return dynamic_cast<Derived&>(*this);
    }

protected:
    const std::function<void(const std::exception_ptr&)> onError;
    const std::function<void()> onComplete;

    ObserverBase(std::function<void(const std::exception_ptr&)> onError, std::function<void()> onComplete)
        : onError(std::move(onError)), onComplete(std::move(onComplete)) {}
};

class SubscriberBase {
public:
    void unsubscribe() const {
        sharedInactiveFlag->store(true, std::memory_order_relaxed);
    }

protected:
    const std::shared_ptr<std::atomic<bool>> sharedInactiveFlag;

    SubscriberBase() : sharedInactiveFlag(std::make_shared<std::atomic<bool>>(false)) {}

    bool isInactive() const {
        return sharedInactiveFlag->load(std::memory_order_relaxed);
    }
};

template <typename T>
class SubscriberManager;
    
} // namespace impl

template <typename T>
class Observer : public impl::ObserverBase {
public:
    /**
     * @brief Constructs an Observer with provided callbacks.
     * 
     * This constructor sets up the `next`, `error`, and `complete` callbacks 
     * for the observer.
     * 
     * @param onNext The callback for handling the next value.
     * @param onError (Optional) The callback for handling errors.
     * @param onComplete (Optional) The callback for handling completion.
     */
    template<typename OnNext>
    requires std::is_invocable_v<OnNext, const T&>
    Observer(OnNext&& onNext,
             std::function<void(const std::exception_ptr&)> onError = [](const std::exception_ptr&) {},
             std::function<void()> onComplete = []() {})
        : ObserverBase(std::move(onError), std::move(onComplete)), onNext(std::forward<OnNext>(onNext)) {}

private:
    const std::function<void(T)> onNext;

    friend class Subscriber<T>;
};

/**
 * @brief A Subscriber that observes events from an Observable.
 * 
 * A `Subscriber` acts as an observer that receives values, errors, 
 * and completion signals from an Observable. Once an error or completion 
 * signal is received, the subscriber is marked as inactive and will 
 * not process further events.
 * 
 * @tparam T The type of data received by the subscriber.
 */
template <typename T>
class Subscriber : public impl::SubscriberBase, public std::enable_shared_from_this<Subscriber<T>> {
public:
    /**
     * @brief Receives the next value from the Observable.
     * 
     * This function processes the emitted value unless the subscriber 
     * is already inactive.
     * 
     * @param t The value emitted by the Observable.
     */
    void next(const T& t) const {
        if (sharedInactiveFlag->load(std::memory_order_relaxed)) {
            return; 
        }

        observer.onNext(t);
    }

    /**
     * @brief Receives an error signal from the Observable.
     * 
     * This function forwards the error to the observer and marks the subscriber as inactive.
     * 
     * @param err The exception pointer representing the error.
     */
    void error(const std::exception_ptr& err) const {
        if (sharedInactiveFlag->exchange(true, std::memory_order_relaxed)) {
            return; 
        }

        observer.onError(err);
    }

    /**
     * @brief Receives the completion signal from the Observable.
     * 
     * Once this function is called, the subscriber will not process any further values.
     */
    void complete() const {
        if (sharedInactiveFlag->exchange(true, std::memory_order_relaxed)) {
            return; 
        }

        observer.onComplete();
    }

protected:
    Subscriber(Observer<T> observer)
        : impl::SubscriberBase(), observer(std::move(observer)) {}

private:
    const Observer<T> observer;

    friend class impl::SubscriberManager<T>;
};

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

template <typename T>
using SharedSubscriber = std::shared_ptr<Subscriber<T>>;

template <typename T>
class SubscriberFactory : public Subscriber<T> {
public:
    static SharedSubscriber<T> create(Observer<T> observer) {
        return std::make_shared<SubscriberFactory<T>>(observer);
    }

    SubscriberFactory(Observer<T> observer) : Subscriber<T>(std::move(observer)) {} 
};
    
} // namespace impl

} // RxLite
