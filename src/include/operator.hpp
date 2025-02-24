#pragma once

#include "observable.hpp"


namespace RxLite {

/**
 * @brief Defines a transformation function that maps one observable type to another.
 * 
 * This alias represents an operator that takes an `Observable<T>` as input and produces an `Observable<U>`.
 * 
 * @tparam T The input observable type.
 * @tparam U The output observable type.
 */
template <typename T, typename U>
using Operator = std::function<Observable<U>(Observable<T>&)>;

/**
 * @brief Transforms values emitted by an observable using a mapping function.
 * 
 * The `map` function applies `mapFunc` to each value emitted by the source observable,
 * producing a new observable of type `U`.
 * 
 * @tparam T The input value type.
 * @tparam U The output value type after transformation.
 * @param mapFunc A function that transforms values of type `T` to type `U`.
 * @return Operator<T, U> A function that applies the transformation to an observable.
 */
template <typename T, typename U>
Operator<T, U> map(std::function<U(const T&)> mapFunc) {
    return [mapFunc](const Observable<T>& observable) {
        return impl::ObservableFactory<U>([mapFunc, observable](const Observer<U>& observer) {
            Observer<T> intermediateObserver(
                [&mapFunc, sharedObserver = std::make_shared<Observer<U>>(observer)](const T& t) {
                    Observer<U>& observer = sharedObserver->template as<Observer<U>>();
                    observer.next(mapFunc(t)); 
                }
            );

            return observable.subscribe(intermediateObserver);
        });
    };
}

} // namespace RxLite
