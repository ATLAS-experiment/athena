/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/constraints/KeyConcept.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;


int main () {

  cerr << "*** KeyConcept_test BEGIN ***" << endl;

  [[maybe_unused]] KeyConcept auto key1 = std::string{"hello"};

  [[maybe_unused]] KeyConcept auto key2 = 42;

  cerr << "*** KeyConcept_test OK ***" <<endl;
  return 0;

}





