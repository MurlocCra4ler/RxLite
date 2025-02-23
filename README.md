# RxLite

## Introduction

RxLite is a lightweight C++ library for composing asynchronous and event-driven programs using observable sequences. It provides a minimal and intuitive API centered around the **Observable** type, with supporting types like **Observer, Subscription, and Subject**, enabling a functional approach to handling asynchronous events.

Think of RxLite as a **lightweight, modern reactive framework for C++**.

RxLite combines the **Observer pattern** with the **Iterator pattern** and applies **functional programming principles** to collections of events, providing a powerful yet simple way to manage asynchronous data streams.

The core concepts in RxLite are:

- **Observable**: Represents a sequence of values or events over time.
- **Observer**: A collection of callbacks that reacts to values emitted by an Observable.
- **Subscription**: Represents the execution of an Observable and allows for cancellation.
- **Operators**: Transform, filter, and combine observables using functional patterns (e.g., `map`, `filter`, `reduce`).
- **Subject**: A bridge between event sources and Observables, enabling multicasting to multiple Observers.

RxLite brings the power of reactive programming to C++ in a lightweight and easy-to-use package.

## Installation

### Using CMake FetchContent

You can easily integrate **RxLite** into your CMake project using `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    RxLite
    GIT_REPOSITORY https://github.com/MurlocCra4ler/RxLite.git
    GIT_TAG        main # or specify a release tag
)

FetchContent_MakeAvailable(RxLite)

target_link_libraries(MyProject PRIVATE RxLite)
