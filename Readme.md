# MySTL

A modern header-only implementation of core C++ Standard Library containers, algorithms, iterators, memory utilities, and functional components, written in C++20.

MySTL is an educational project focused on understanding how a standard library is built from the ground up. Rather than wrapping existing STL components, every major data structure and utility is implemented from scratch while following modern C++ design principles and closely mirroring the architecture of the real Standard Library.

The primary goals of this project are:

- Understand low-level container implementation.
- Explore allocator-aware programming.
- Learn generic programming and template design.
- Study iterator abstractions and algorithm implementation.
- Build production-quality C++ engineering skills.

---

# Features

## Sequence Containers

- Vector
- String (Small String Optimization)
- Deque
- List
- ForwardList

## Ordered Associative Containers

Implemented on top of a shared Red-Black Tree.

- Set
- MultiSet
- Map
- MultiMap

## Unordered Associative Containers

Implemented using separate chaining hash tables.

- UnorderedSet
- UnorderedMultiSet
- UnorderedMap
- UnorderedMultiMap

## Container Adaptors

- Stack
- Queue
- PriorityQueue

## Algorithms

Includes implementations of many standard algorithms, including:

- copy
- move
- fill
- transform
- equal
- lexicographical_compare
- find
- count
- for_each
- min/max/clamp
- heap algorithms
    - make_heap
    - push_heap
    - pop_heap

## Memory Utilities

- allocator
- allocator_traits
- construct_at
- destroy_at
- destroy
- uninitialized_copy
- uninitialized_move
- uninitialized_fill

## Functional Utilities

- comparison functors
- arithmetic functors
- transparent comparators
- reference_wrapper
- hash framework (FNV-1a)

---

# Architecture

The library is organized into multiple independent layers.

```
                +----------------------+
                |     Adaptors         |
                +----------+-----------+
                           |
        +------------------+------------------+
        |                                     |
+-------v--------+                 +----------v---------+
| Ordered        |                 | Sequence           |
| Containers     |                 | Containers         |
+-------+--------+                 +--------------------+
        |
+-------v----------------------------------------------+
|                  Red-Black Tree                      |
+-------+----------------------------------------------+
        |
+-------v----------------------------------------------+
| Utility | Memory | Functional | Algorithms           |
+-------+----------------------------------------------+
        |
+-------v----------------------------------------------+
| Type Traits | Iterators | Allocator                 |
+------------------------------------------------------+
```

The dependency graph forms a Directed Acyclic Graph (DAG), ensuring clear ownership and separation of responsibilities.

---

# Design Principles

The implementation follows several core design principles.

- Header-only library
- Modern C++20
- Zero circular dependencies
- Strong separation of concerns
- Generic programming
- Policy-based design
- Iterator abstraction
- Exception-aware resource management
- Extensive use of `allocator_traits`
- Empty Base Optimization (`[[no_unique_address]]`)
- Shared implementation where appropriate (Red-Black Tree backbone)

---

# Containers

| Container | Implementation |
|------------|----------------|
| Vector | Dynamic contiguous array |
| String | Small String Optimization (SSO) |
| Deque | Segmented block storage |
| List | Circular doubly-linked list with sentinel |
| ForwardList | Singly-linked list with head sentinel |
| Set | Red-Black Tree |
| MultiSet | Red-Black Tree |
| Map | Red-Black Tree |
| MultiMap | Red-Black Tree |
| UnorderedSet | Separate Chaining Hash Table |
| UnorderedMultiSet | Separate Chaining Hash Table |
| UnorderedMap | Separate Chaining Hash Table |
| UnorderedMultiMap | Separate Chaining Hash Table |
| Stack | Container adaptor |
| Queue | Container adaptor |
| PriorityQueue | Binary Heap |

---

# Iterator Design

Several containers share a unified iterator implementation.

Highlights include:

- const/non-const iterator conversion
- bidirectional iterators
- random access iterators
- sentinel-based iteration
- STL-compatible iterator interfaces

Ordered containers expose immutable iterators for keys, matching the behavior of the standard library.

---

# Memory Model

Memory management is built around a custom implementation of `allocator_traits`.

Features include:

- allocator rebinding
- object lifetime management
- placement construction
- explicit destruction
- exception-safe uninitialized algorithms
- RAII cleanup during partial construction

---

# Exception Safety

The library aims to provide exception-safe implementations where applicable.

Current techniques include:

- RAII cleanup guards
- copy-and-swap assignment
- strong guarantees for uninitialized algorithms
- resource rollback during failed construction

---

# Testing

Every major component has an independent GoogleTest executable.

The project currently contains tests for:

- containers
- iterators
- algorithms
- memory utilities
- functional utilities
- Red-Black Tree

---

# Build

Requirements:

- C++20 compiler
- CMake
- GoogleTest (automatically downloaded through FetchContent)

```bash
git clone https://github.com/HenrikGharagyozyan/MySTL.git

cd MySTL

cmake -B build

cmake --build build

ctest --test-dir build
```

---

# Project Statistics

| Property | Value |
|-----------|-------|
| Language | C++20 |
| Library Type | Header-only |
| Headers | 22 |
| Approximate LOC | ~8,400 |
| Unit Tests | GoogleTest |
| Build System | CMake |

---

# Current Limitations

Although the project already implements a substantial subset of the Standard Library, several areas remain intentionally simplified.

Current architectural limitations include:

- unordered containers duplicate much of their implementation instead of sharing a common hash table backbone
- allocator propagation traits are not yet implemented
- stateful allocators are not fully supported
- `mystl::String` does not yet integrate with the hash framework
- unordered containers currently provide a smaller API surface than their standard library counterparts
- some exception handling strategies are not yet fully consistent across all containers

These limitations are known and are planned to be addressed as the project evolves.

---

# Roadmap

Future improvements include:

- Shared HashTable backbone
- Complete allocator propagation support
- Full STL API compatibility
- Additional algorithms
- More container adaptors
- Expanded unit testing
- Performance benchmarking
- Documentation improvements

---

# Philosophy

The purpose of MySTL is not to replace the C++ Standard Library.

Instead, it serves as an engineering-focused exploration of how a modern generic library can be designed and implemented. Every component is developed with an emphasis on understanding data structures, memory management, iterator abstractions, template programming, and software architecture.

The project prioritizes readability, correctness, and long-term maintainability while remaining as close as practical to the design philosophy of the C++ Standard Library.