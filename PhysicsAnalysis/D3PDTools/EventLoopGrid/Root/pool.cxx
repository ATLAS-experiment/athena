/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "pool.h"

#include <atomic>
#include <thread>

namespace {

  // Pull work units off the shared list by atomically claiming the next index,
  // so multiple worker threads never run the same unit.
  void worker(const WorkList& workList, std::atomic<size_t>& next)
  {
    for (size_t i = next++; i < workList.size(); i = next++) {
      workList[i]();
    }
  }

  struct Threads
  {
    Threads(const size_t n, std::function<void(void)> function)
      {
	for (size_t k = 0; k != n; k++) { threads.emplace_back( function ); }
      }
    ~Threads() { for (auto& t : threads) { t.join(); } }
  private:
    std::vector<std::thread> threads;
  };

} //namespace

void process(const WorkList& workList, const size_t nThreads)
{
  std::atomic<size_t> next{0};
  if (nThreads == 0) {
    worker(workList, next);
  } else {
    Threads(nThreads, [&](){ worker(workList, next); });
  }
}

