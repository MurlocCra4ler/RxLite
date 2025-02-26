# A Lightweight Reactive Framework for C++

## 👋 Introduction

Rx​Lite is a lightweight C++ library for composing asynchronous and event-driven programs using observable sequences. It provides a minimal and intuitive API centered around the **Observable** type, with supporting types like **Observer, Subscription, and Subject**, enabling a functional approach to handling asynchronous events.

Think of Rx​Lite as a **lightweight, modern reactive framework for C++**.

Rx​Lite combines the **Observer pattern** with the **Iterator pattern** and applies **functional programming principles** to collections of events, providing a powerful yet simple way to manage asynchronous data streams.

The core concepts in Rx​Lite are:

- **Observable**: Represents a sequence of values or events over time.
- **Observer**: A collection of callbacks that reacts to values emitted by an Observable.
- **Subscription**: Represents the execution of an Observable and allows for cancellation.
- **Operators**: Transform, filter, and combine observables using functional patterns (e.g., `map`, `filter`, `reduce`).
- **Subject**: A bridge between event sources and Observables, enabling multicasting to multiple Observers.

Rx​Lite brings the power of reactive programming to C++ in a lightweight and easy-to-use package.

## 📝 Documentation

The latest documentation is available here: [RxLite Docs](https://murloccra4ler.github.io/RxLite/)

## 🚀 Getting Started

Below is a simple example demonstrating how to create an Observable, apply an operator,  
and subscribe to process emitted values.

```cpp
#include <iostream>

#include "RxLite.hpp"

int main() {
    RxLite::Subject<int> subject;

    // Apply an operator to modify emitted values
    RxLite::Observable<int> observable = subject.pipe(
        RxLite::map<int>([](int i) { return i * 2; })
    );

    int sum = 0;

    // Subscribe and accumulate the transformed values
    RxLite::Subscription subscription = observable.subscribe([&sum](int i) {
        sum += i;
    });

    // Emit values
    for (int i = 1; i <= 5; i++) {
        subject.next(i);
    }

    // Expected sum: (1*2) + (2*2) + ... + (5*2) = 30
    std::cout << "Sum: " << sum << std::endl;

    return 0;
}
```

## 🔧 Installation

### Using CMake FetchContent

You can easily integrate **Rx​Lite** into your CMake project using `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    RxLite
    GIT_REPOSITORY https://github.com/MurlocCra4ler/RxLite.git
    GIT_TAG        main # or specify a release tag
)

FetchContent_MakeAvailable(RxLite)

target_link_libraries(MyProject PRIVATE RxLite)
```

## 🎯 Goals for First Release

Before the first official release, the following tasks need to be completed:

- [ ] Implement all operators
- [ ] Implement schedulers
- [ ] Add multithreading support
- [ ] Add a testing framework
- [ ] Write comprehensive documentation