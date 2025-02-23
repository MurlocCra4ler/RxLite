#pragma once

#include "observable.hpp"

template <typename T>
static Observable<T> of(T t) {
    std::function<void(const SharedObserver&)> subscriber = [t](const SharedObserver& sharedObserver) {
        auto& observer = sharedObserver->as<Observer<T>>();
        observer.next(t);
        observer.complete();
    };

    return Observable<T>(subscriber);
}

template <typename T>
static Observable<T> from(std::vector<T> ts) {
    std::function<void(const SharedObserver&)> subscriber = [ts](const SharedObserver& sharedObserver) {
        auto& observer = sharedObserver->as<Observer<T>>();
        
        for (const auto& t : ts) {
            observer.next(t);
        }

        observer.complete();
    };

    return Observable<T>(subscriber);
}
