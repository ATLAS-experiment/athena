/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>
#include "TrigConfHLTUtils/HLTUtils.h"

using namespace std::chrono;
using namespace TrigConf;

int main (int argc, char *argv []) {
  if (argc <= 1 || argv[1]==std::string("-h")) {
    std::cout << "Usage: trigconf_string2hash [IDENT] [-f FILE] [-t THREADS] [-r REPEAT]" << std::endl;
    std::cout << "  convert identifier to hash" << std::endl;
    std::cout << "  -f  file with hashes produced by hashes2file" << std::endl;
    std::cout << "  -t  number of threads" << std::endl;
    std::cout << "  -r  number of times the file should be processed" << std::endl;
    return 1;
  }

  // Command line parsing
  std::string filename;
  size_t nthreads = 0;
  size_t N = 1;
  for (int i=1; i<argc; ++i) {
    if (std::string(argv[i])=="-f") {
      filename = argv[i+1];
    }
    else if (std::string(argv[i])=="-t") {
      nthreads = std::atoi(argv[i+1]);
    }
    else if (std::string(argv[i])=="-r") {
      N = std::atoi(argv[i+1]);
    }
  }

  // Convert string to hash
  if (argc == 2) {
    std::cout << HLTUtils::string2hash(argv[1]) << std::endl;
    return 0;
  }

  // Performance measurement
  auto t1 = high_resolution_clock::now();
  if (!filename.empty()) {
    auto run = [&]() {
      for (size_t i=0; i<N; i++) HLTUtils::file2hashes(filename);
    };
    if (nthreads==0) {
      //coverity[TAINTED_SCALAR]
      run();
    }
    else {
      std::cout << "Launching " << nthreads << " threads reading file each "
                << N << " times" << std::endl;
      //coverity[TAINTED_SCALAR]
      std::vector<std::thread> threads;
      for (size_t i = 0; i<nthreads; ++i) threads.emplace_back(run);
      for (auto& thr : threads)           thr.join();
    }
  }
  auto t2 = high_resolution_clock::now();
  std::cout << duration_cast<milliseconds>(t2 - t1).count() << " ms" << std::endl;

  return 0;
}
