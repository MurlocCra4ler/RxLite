#pragma once

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
     * @param onSubscribe A function that takes an `Observer<T>` which defines how values are emitted.
     */
    template <typename Func>
    requires std::invocable<Func, const Subscriber<T>&> &&
             std::same_as<std::invoke_result_t<Func, const Subscriber<T>&>, void>
    explicit Observable(Func&& onSubscribe)
        : onSubscribe([onSubscribe = std::forward<Func>(onSubscribe)](const Subscriber<T>& subscriber) {
            onSubscribe(subscriber);
            return []() {};  // Default TeardownLogic
        }) {}

    template <typename Func>
    requires std::invocable<Func, const Subscriber<T>&> &&
            std::is_same_v<std::invoke_result_t<Func, const Subscriber<T>&>, TeardownLogic>
    explicit Observable(Func&& onSubscribe)
        : onSubscribe(std::forward<Func>(onSubscribe)) {}

    /**
     * @brief Creates an observable that emits a single value and then completes.
     * 
     * This function returns an observable that emits the provided value `t` and then signals completion.
     * 
     * @param value The value to be emitted by the observable.
     * @return Observable<T> An observable that emits a single value and then completes.
     */
    static Observable<T> of(T value) {
        std::function<void(const Subscriber<T>& subscriber)> onSubscribe = [value](const Subscriber<T>& subscriber) {
            subscriber.next(value);
            subscriber.complete();
        };
    
        return Observable<T>(onSubscribe);
    }

    /**
     * @brief Creates an observable that emits a sequence of values from a vector.
     * 
     * This function returns an observable that emits each value in the provided vector `ts`
     * sequentially and then signals completion.
     * 
     * @param values A vector of values to be emitted by the observable.
     * @return Observable<T> An observable that emits all values from the vector and then completes.
     */
    static Observable<T> from(std::vector<T> values) {
        std::function<void(const Subscriber<T>& subscriber)> onSubscribe = [values](const Subscriber<T>& subscriber) {
            for (const auto& t : values) {
                subscriber.next(t);
            }
    
            subscriber.complete();
        };
    
        return Observable<T>(onSubscribe);
    }

    /**
     * @brief Subscribes an observer to the observable.
     * 
     * When an observer subscribes, the observable starts emitting values to it.
     * The returned `Subscription` can be used to unsubscribe from the observable.
     * 
     * @param observer The observer that will receive emitted values.
     * @return Subscription An object representing the active subscription.
     */
    Subscription subscribe(Observer<T> observer) const {
        Subscriber<T> subscriber = impl::SubscriberFactory(observer);
        TeardownLogic teardownLogic = onSubscribe(subscriber);
        return impl::SubscriptionFactory(subscriber, teardownLogic);
    }

    /**
     * @brief Applies a sequence of operators to the observable.
     * 
     * The `pipe` function allows chaining multiple transformation functions
     * that operate on the observable.
     *
     * @param first The first function to apply.
     * @param rest Additional functions to apply.
     * @return The transformed observable or final result after applying all functions.
     */
    template <typename First, typename... Rest>
    requires std::is_invocable_v<First, Observable<T>&>
    auto pipe(First&& first, Rest&&... rest) {
        if constexpr (sizeof...(rest) == 0) {
            return std::forward<First>(first)(*this);
        } else {
            return std::forward<First>(first)(*this).pipe(std::forward<Rest>(rest)...);
        }
    }

protected:
    std::function<TeardownLogic(const Subscriber<T>&)> onSubscribe;
};

/**
 * @brief Contains implementation details.
 * 
 * Users of RxLite should not need to interact with this directly.
 */
namespace impl {

template <typename T>
class ObservableFactory : public Observable<T> {
public:
    ObservableFactory(std::function<TeardownLogic(const Subscriber<T>&)> onSubscribe)
        : Observable<T>(std::move(onSubscribe)) {} 
};

} // namespace impl

} // namespace RxLite
