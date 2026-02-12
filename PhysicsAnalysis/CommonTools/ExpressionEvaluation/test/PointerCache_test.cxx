/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include <atomic>
#include <thread>
#include <cassert>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <span>

#include "ExpressionEvaluation/PointerCache.h"

class IAccessor {
public:
   IAccessor() { ++s_ctor; }
   ~IAccessor() { ++s_dtor; }
   static std::atomic<unsigned int> s_ctor;
   static std::atomic<unsigned int> s_dtor;
};

std::atomic<unsigned int> IAccessor::s_ctor{};
std::atomic<unsigned int> IAccessor::s_dtor{};

using AccessorCache = ExpressionParsing::PointerCache<IAccessor, std::memory_order_seq_cst>;

void accessor_task (const IAccessor &global_accessor,
                    const std::atomic<bool> &end_of_loop, AccessorCache &cache, std::atomic<unsigned int> &processed_count,
                    const std::atomic<unsigned int> &event_count,
                    std::atomic<unsigned int> &updates,
                    std::span<unsigned int> random_numbers) {
   unsigned int last_processed_event=0;
   while (!end_of_loop.load()) {
      if (event_count!=last_processed_event)  {
         last_processed_event=event_count;
         if (random_numbers[last_processed_event%random_numbers.size()]&1) {
            std::unique_ptr<IAccessor> new_accesor = std::make_unique<IAccessor>();
            cache.setIfUnset(std::move(new_accesor));
         }
         else {
            cache.setIfUnset(global_accessor);
         }
         ++updates;
         ++processed_count;
      }
   }
}

int main (int argc, char **argv) {
   std::atomic<bool> end_of_loop{};
   std::atomic<unsigned int> processed{};
   std::atomic<unsigned int> event_count{};
   std::atomic<unsigned int> updates{};

   unsigned int n_threads=argc > 1 ? atoi(argv[1]) : 12;
   unsigned int n_events=argc > 2 ? atoi(argv[2]) : 100;
   bool verbose = argc > 3 ? atoi(argv[3]) : 0;

   std::vector<unsigned int> random_numbers;
   random_numbers.reserve(n_threads*n_events);
   for (unsigned int i=0; i<random_numbers.capacity(); ++i) {
      random_numbers.push_back(rand());
   }

   {
   std::unique_ptr<IAccessor> accessor = std::make_unique<IAccessor>();
   AccessorCache cache;

   std::vector< std::thread > tasks;
   for (unsigned int i=0; i<n_threads; ++i) {
      unsigned int start=n_events*i;
      if (start+n_events>random_numbers.size()) { std::cerr << "not enough random numbers for thread." << std::endl; abort(); }
      tasks.emplace_back([&global_accesor=*accessor,&cache, &end_of_loop, &processed,&event_count, &updates,
                          numbers=std::span<unsigned int>(random_numbers.data()+n_events*i,n_events)]() {
         accessor_task(global_accesor,end_of_loop, cache,processed,event_count,updates, numbers);
      });
   }
   for (unsigned int i=0; i<n_events; ++i) {
      processed=0;
      ++event_count;
      while (processed != n_threads) { }
   }
   end_of_loop=true;
   for (std::thread &a_task : tasks) {
      a_task.join();
   }
   }
   if (verbose) {
      std::cout << "done. total updates "  << updates
                << " | dtors " <<  IAccessor::s_dtor
                << " =?=  ctors: " << IAccessor::s_ctor
                << std::endl;
   }
   return IAccessor::s_dtor == IAccessor::s_ctor ? 0 : 1;
}
