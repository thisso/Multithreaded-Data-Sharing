# Multithreaded-Data-Sharing
Multithreaded grep clone in C++ using POSIX threads — with a custom thread-safe request queue and exact word matching.
# Multithreaded Grep in C++

A minimal reimplementation of the Unix `grep` utility in modern C++, supporting both sequential and multithreaded modes.

## Features

- Exact token matching (not substrings)
- Sequential and multithreaded versions
- Thread-safe request queue implemented from scratch (no STL containers)
- POSIX threads (`<pthread.h>`) with mutex and condition variables
- Graceful handling of `EOF` (Ctrl+D)

## Structure

- `grep_sequential.cpp`: Single-threaded grep implementation
- `grep.cpp`: Multithreaded version using a producer-consumer model
- `RequestQueue.hpp / .cpp`: Custom thread-safe queue using linked list
- `grep_request.hpp`: Request data structure and formatting

## Build & Run

```bash
make            # Builds all targets: grep, grep_sequential, test_suite
./grep          # Run multithreaded version
./grep_sequential  # Run single-threaded version
