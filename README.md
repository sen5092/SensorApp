# PracticeCPP – C++ Interview Prep Project

This project is a personal C++ playground for reviewing and implementing core techniques, data structures, algorithms, and modern features in preparation for technical interviews.

---

## 🚀 CI/CD Pipeline and Tooling

This project uses **GitHub Actions** for continuous integration. Key features:

### ✅ Build & Test
- Automatically compiles the project with CMake on every push or pull request.
- Runs all Catch2 unit tests.
- Runs code coverage.
- Runs static analysis.
- Runs code formatting (review only, no apply).

### 🧠 Static Analysis (Clang)
- **Clang-Tidy** checks for bugs, performance issues, and modern C++ best practices.
- **Clang Static Analyzer (CSA)** detects logic errors and memory bugs.
- Controlled via `scripts/analyze.sh` and `.clang-tidy`.
- Reports are stored in the `build/reports/` directory.

### 📈 Code Coverage (LLVM `llvm-cov`)
- Built using Clang with instrumentation flags.
- Test coverage is generated with `llvm-profdata` and `llvm-cov`.
- Generates a line-by-line `coverage.txt` summary for source files.
- Ignores test files and third-party dependencies.
- Uploaded as artifacts in GitHub Actions.

### Code Formatting (Clang-Format)
- **Clang-Format** checks in `/src` and `/include` for all `*.cpp` and `*.hpp` files. 
- Controller via `scripts/format.sh` and `.clang-format`.
- By default, `format.sh` will apply formatting, add `--check` for dry-run and differences.

### 🛠️ Local Development Workflow

> All CI functionality can be reproduced locally using these scripts:

```bash
# Clean build with coverage
./scripts/build.sh --rebuild

# Build with coverage (default)
./scripts/build.sh

# Generate coverage report (and runs tests)
./scripts/coverage.sh

# Run static analysis (CSA + clang-tidy)
./scripts/analyze.sh --tidy --csa

# Run formatting (applies changes automatically by default)
./scripts/format.sh --check
```

---

## ✅ Completed Bit Manipulation Features

The following utility functions have been implemented and tested as part of the `BitManipulator` module:

- [x] `getNthBit` – Returns 0 or 1 for the given bit position  
- [x] `setNthBit` – Returns a copy of the number with the given bit set to 1  
- [x] `clearNthBit` – Returns a copy of the number with the given bit cleared to 0  
- [x] `printBits` – Prints a 32-bit binary representation to standard output  
- [x] `countNumberOfBitsSet` – Counts the number of bits set to 1 using Brian Kernighan’s algorithm  
- [x] `reverseBits` – Reverses the bits over a given width (e.g. 4, 8, 32 bits)  
- [x] `swapBits` – Swaps bits at two specified positions  
- [x] `isPowerOfTwo` – Returns true if the number is a power of two  
- [x] `isolateRightmostSetBit` – Returns the least significant bit that is set  

All functions are implemented as `static` methods and fully unit tested using Catch2.

---

## 📦 Data Structures (from scratch)
- [ ] Singly Linked List
- [ ] Doubly Linked List
- [ ] Stack
- [ ] Queue (circular preferred)
- [x] Hash Table
- [ ] Binary Tree (with traversal)
- [ ] Trie
- [ ] Graph (adjacency list/matrix)
- [ ] LRU Cache

---

## ⚙️ Algorithms
- [ ] Bubble Sort
- [ ] Insertion Sort
- [ ] Merge Sort
- [ ] Quick Sort
- [ ] Binary Search (iterative)
- [ ] Binary Search (recursive)
- [ ] Recursion / Backtracking (e.g., N-Queens, Subsets)
- [ ] DFS/BFS (tree or graph)
- [ ] Dynamic Programming (Fibonacci, Coin Change)

---

## 📚 STL Practice
- [ ] Vectors & Iterators
- [ ] Sets & Maps
- [ ] Queues & Priority Queues
- [ ] `std::sort`, `std::find_if`, `std::accumulate`
- [ ] `std::pair`, `std::tuple`, `std::optional`
- [ ] Range-based for loops

---

## 🔐 Object-Oriented Programming
- [x] Classes & Constructors
- [ ] Inheritance
- [ ] Virtual functions & Polymorphism
- [ ] Rule of 3 / Rule of 5
- [ ] Operator Overloading
- [ ] Interfaces via abstract classes

---

## 🧼 Memory Management
- [ ] Raw pointers with `new`/`delete`
- [ ] RAII pattern
- [ ] `unique_ptr`, `shared_ptr`, `weak_ptr`
- [ ] Dangling pointer examples
- [ ] Manual vs. smart pointer comparison

---

## 💎 Modern C++ Features
- [ ] `auto`, `decltype`, `constexpr`
- [ ] Lambda expressions
- [ ] `std::function`
- [ ] Move semantics
- [ ] `std::optional`, `std::variant`
- [ ] Structured bindings (C++17)

---

## 🔧 Multithreading (Advanced)
- [ ] `std::thread`, `std::mutex`, `std::lock_guard`
- [ ] Deadlock example + fix
- [ ] `std::condition_variable`
- [ ] Thread-safe data structure
- [ ] `std::async` / `std::future`

---

## 🧪 Unit Testing & Debugging
- [x] Catch2 integration
- [x] Binary printing with `std::bitset`
- [ ] Test coverage for all modules
- [ ] TDD examples

---

## 💼 System Design Concepts (Lite)
- [ ] Layered design (Data, Logic, UI separation)
- [ ] Interface/abstraction via pure virtual classes
- [ ] Testable modules with dependency injection

---

## ✅ Usage

```bash
# Configure and build the project
cmake -S . -B build
cmake --build build

# Run app or tests
./build/src/PracticeApp
./build/tests/PracticeTests -s
```
