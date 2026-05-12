/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include "EventContainers/IdentifierMask.h"
#include <set>
#include <iostream>
#include <algorithm>

void test_basic_set_unset() {
    std::cout << "Running: test_basic_set_unset... " << std::flush;
    EventContainers::IdentifierMask mask(100);

    mask.set(5);
    mask.set(63);
    mask.set(64);
    mask.set(99);

    assert(mask.test(5) == true);
    assert(mask.test(63) == true);
    assert(mask.test(64) == true);
    assert(mask.test(99) == true);
    assert(mask.test(0) == false);
    assert(mask.test(50) == false);

    mask.unset(63);
    assert(mask.test(63) == false);
    assert(mask.test(64) == true); // Ensure adjacent word/bit is untouched

    std::cout << "Passed!" << std::endl;
}

void test_iteration() {
    std::cout << "Running: test_iteration... " << std::flush;
    size_t max = 500;
    EventContainers::IdentifierMask mask(max);
    
    std::set<size_t> expected = {0, 1, 63, 64, 127, 128, 499};
    for (auto h : expected) mask.set(h);

    std::set<size_t> actual;
    mask.forEachSetBit([&](size_t hash) {
        actual.insert(hash);
    });

    assert(actual == expected);
    assert(mask.count() == expected.size());
    std::cout << "Passed!" << std::endl;
}

void test_iteration_order() {
    std::cout << "Running: test_iteration_order... " << std::flush;
    EventContainers::IdentifierMask mask(1024);

    // Set bits in a non-sequential order to ensure iteration 
    // is governed by the bitmask structure, not insertion order.
    std::vector<size_t> to_set = {1023, 0, 512, 63, 64, 10, 256};
    for (size_t h : to_set) mask.set(h);

    std::vector<size_t> visited;
    mask.forEachSetBit([&](size_t hash) {
        visited.push_back(hash);
    });

    // The expected order is strictly ascending
    std::vector<size_t> expected = to_set;
    std::sort(expected.begin(), expected.end());

    assert(visited.size() == expected.size());
    assert(visited == expected);
    
    // Specifically check that it processes lower bits/words first
    for (size_t i = 1; i < visited.size(); ++i) {
        assert(visited[i] > visited[i-1]);
    }

    std::cout << "Passed!" << std::endl;
}

void test_clear() {
    std::cout << "Running: test_clear... " << std::flush;
    EventContainers::IdentifierMask mask(1000);
    mask.set(10);
    mask.set(500);
    mask.set(999);

    assert(mask.count() == 3);
    mask.clear();
    assert(mask.count() == 0);
    assert(mask.test(500) == false);
    std::cout << "Passed!" << std::endl;
}


int main() {
    try {
        test_basic_set_unset();
        test_iteration();
        test_clear();
        test_iteration_order();
        std::cout << "\nAll IdentifierMask tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
