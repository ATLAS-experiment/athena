/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef APRTESTBASE_H
#define APRTESTBASE_H

#include "GaudiKernel/System.h"

#include <string>
#include <vector>
#include <iostream>


/// Small helper base class for pool tests 
class TestBase {
public:
  TestBase() = default;
  TestBase(const TestBase&) = delete;
  TestBase& operator=(const TestBase&) = delete;
  virtual ~TestBase() = default;

  /// load local dictionary libraries for the test
  static void loadLibraries( const std::vector<std::string>& libraries );
};


inline
void TestBase::loadLibraries( const std::vector<std::string>& libraries )
{
   for( const auto& library : libraries ) { 
    std::cout << "Loading library " << library << std::endl;

    System::ImageHandle         libhandle;
    if( System::loadDynamicLib(library, &libhandle) != 1 ) {
       std::cout << "Error! library loading failed"  << std::endl;
    }
  }
}


#endif