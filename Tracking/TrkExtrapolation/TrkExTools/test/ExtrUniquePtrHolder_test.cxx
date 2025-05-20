/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @author Christos Anastopoulos
 * @brief Some tests for ExtrUniquePtrHolder
 */

#include "TrkExTools/ExtrUniquePtrHolder.h"

#include <iostream>
#include <memory>

struct Cache {
  Trk::ExtrUniquePtrHolder<int> m_ptr;
};

int main() {

  std::unique_ptr<int> clientPtr1;
  std::unique_ptr<int> clientPtr2;
  // Let's create a scope of the cache
  {
    Cache cache{};
    /// make an element
    auto uniq1 = std::make_unique<int>(1);
    // Try to get what is there
    auto nothere = cache.m_ptr.move(uniq1.get());
    std::cout << "ptr not in the cache so nullptr  : " << nothere.get()
              << std::endl;

    // Now  properly pushed
    Trk::CacheOwnedPtr<int> ptr1 = cache.m_ptr.push(std::move(uniq1));

    // push another one
    auto uniq2 = std::make_unique<int>(2);
    Trk::CacheOwnedPtr<int> ptr2 = cache.m_ptr.push(std::move(uniq2));

    // The values we got back
    std::cout << "Cache contrains ptr with value " << *ptr1 << std::endl;
    std::cout << "Cache contrains ptr with valus " << *ptr2 << std::endl;

    // Let's try to release some meaningless things
    int value = 4;
    auto meaningless1 = cache.m_ptr.move(&value);
    std::cout << "ptr not in the cache so nullptr  : " << meaningless1.get()
              << std::endl;

    auto randomUnique = std::make_unique<int>(1);
    auto meaningless2 = cache.m_ptr.move(randomUnique.get());
    std::cout << "ptr not in the cache so nullptr  : " << meaningless2.get()
              << std::endl;

    // Release the ptr to the client
    clientPtr2 = cache.m_ptr.move(ptr2);
    clientPtr1 = cache.m_ptr.move(ptr1);

    // We should not re-find this  as got released
    auto alreadycleaned = cache.m_ptr.move(ptr1);
    std::cout << "Not there since is moved out so nullptr : "
              << alreadycleaned.get() << std::endl;
  }
  // The cache is out of scope
  std::cout << "Client gets : " << *clientPtr1 << std::endl;
  std::cout << "Client gets : " << *clientPtr2 << std::endl;
}
