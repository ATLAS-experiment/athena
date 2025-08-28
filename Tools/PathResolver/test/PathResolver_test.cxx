/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PathResolver (unit) testing application. See -h for help.

#undef NDEBUG
#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>


void test1()
{
  std::cout << "test1" << std::endl;
  std::string file = PathResolver::find_file("a.txt", "DATAPATH");
  assert( file.ends_with("A/a.txt") );

  file = PathResolver::find_file("A/a.txt", "DATAPATH");
  assert( file.ends_with("A/a.txt") );

  file = PathResolver::find_file("B/a.txt", "DATAPATH");
  assert( file.ends_with("B/a.txt") );

  file = PathResolver::find_file("z.txt", "DATAPATH");
  assert( file.empty() );

  std::string dir = PathResolver::find_directory("A", "DATAPATH");
  assert( dir.ends_with("A") );

  dir = PathResolver::find_directory("Z", "DATAPATH");
  assert( dir.empty() );
}


void test2()
{
  std::cout << "test2" << std::endl;
  // Find a local file
  std::ofstream ofs;
  ofs.open("foo.txt");
  std::string file = PathResolver::find_file_from_list("foo.txt", "");
  assert( file.ends_with("foo.txt") );
}


void test3 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test3" << std::endl;
  setenv("PATHRESOLVER_DEVAREARESPONSE", "WARNING", 1);
  std::string file = PathResolver::find_calib_file("dev/foo.txt");
  assert( file.empty() );
}


void test_http ATLAS_NOT_THREAD_SAFE ()
{
  setenv("PATHRESOLVER_ALLOWHTTPDOWNLOAD", "1", 1);
  std::string file = PathResolver::find_file_from_list("Testing/testing/Testfile",
                                                       "http//cern.ch/atlas-groupdata");

  assert( file.ends_with("Testing/testing/Testfile") );
}


int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[])
{
  // No arguments, run default tests
  if (argc == 1) {
    test1();
    test2();
    test3();
    return 0;
  } else if (argc == 2 && strcmp(argv[1], "-d")==0) {
    test_http();
    return 0;
  }

  // Help
  if ((argc >= 2 && strcmp(argv[1], "-h")==0) || argc != 3 ) {
    std::cout << "Syntax: PathResolver_test.exe [Options] [FILENAME] [SEARCH_PATH]" << std::endl;
    std::cout << "   -h     this help message" << std::endl;
    std::cout << "   -d     run http-based download tests" << std::endl;
    std::cout << "If no arguments are given, the default test suite is run." << std::endl;
    return 0;
  }

  // Manual test
  const std::string filename = argv[1];
  const std::string search_path = argv[2];

  PathResolver::SetOutputLevel(MSG::DEBUG);
  const std::string location = PathResolver::find_file(filename, search_path);
  if (location.empty()) {
    std::cout << "Cannot find " << filename << " in " << search_path << std::endl;
    return 1;
  }
  std::cout << "Found " << location << std::endl;

  return 0;
}
