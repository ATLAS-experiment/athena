/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/constraints/KeyConcept.h"

#include <cassert>
#include <print>
#include <string>

using namespace std;


int main () {

  std::println ("*** KeyConcept_test BEGIN ***");

  [[maybe_unused]] KeyConcept auto key1 = std::string{"hello"};

  [[maybe_unused]] KeyConcept auto key2 = 42;

  std::println ("*** KeyConcept_test OK ***");
  return 0;

}





