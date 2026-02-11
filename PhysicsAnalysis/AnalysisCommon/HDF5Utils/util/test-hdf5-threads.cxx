/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/Writer.h"

#include <format>
#include <iostream>
#include <thread>
#include <chrono>

struct out_t
{
  size_t thread_number;
  size_t event_number;
};
using consumer_t = H5Utils::Consumers<const out_t&>;

int main(int nargs, char* argv[]) {
  H5::H5File h5_file("mt_output.h5", H5F_ACC_TRUNC);
  size_t n_threads = 5;
  if (nargs >= 2) {
    n_threads = std::stoi(argv[1]);
  }
  size_t n_events = 1;
  if (nargs >= 3) {
    n_events = std::stoi(argv[2]);
  }
  size_t ms_sleep = 0;
  if (nargs >= 4) {
    ms_sleep = std::stoi(argv[3]);
  }
  if (nargs > 4) {
    std::cerr << std::format(
      "usage: {} [n_threads] [n_events] [sleep (ms)]\n", argv[0]);
    return 1;
  }

  // define outputs (consumers)
  consumer_t consumers;
  consumers.add("thread", [ms_sleep](const out_t& o){
    std::cout << "writing event " << o.event_number << " to thread " << o.thread_number << "\n";
    if (ms_sleep) {
      std::this_thread::sleep_for(std::chrono::milliseconds(ms_sleep));
    }
    return o.thread_number;
  });
  consumers.add("event", [](const out_t& o) {
    return o.event_number;
  });

  // set the batch size to something small to ensure some flushing
  H5Utils::WriterConfiguration<0> config;
  config.batch_size = n_events > 10 ? n_events / 10 : 1;
  config.name = "out";
  std::cout << std::format(
    "writing {} events each to {} threads, in batches of {}\n",
    n_events,
    n_threads,
    *config.batch_size);

  auto writer = H5Utils::makeWriter<0>(h5_file, consumers, config);

  std::vector<std::thread> threads;
  threads.reserve(n_threads);
  for (size_t thread_n = 0; thread_n < n_threads; thread_n++) {
    threads.emplace_back(
      [thread_n, n_events, &writer](){
        std::cout << "starting thread " << thread_n << "\n";
        for (size_t event_n = 0; event_n < n_events; event_n++) {
          writer.fill(out_t{thread_n, event_n});
        }
      });
  }
  for (auto& thread: threads) thread.join();
  return 0;
}
