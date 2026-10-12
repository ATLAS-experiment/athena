/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#define DO_REMOVE_DUPLICATES 1
#include <cassert>
#include <print>
#include "AthContainers/DataVector.h"
using namespace std;
int main() {
  DataVector<int> *pDV = new DataVector<int>;
  int *pDuplicated = new int(0);
  pDV->push_back(pDuplicated);
  pDV->push_back(new int(1));
  pDV->push_back(pDuplicated); //this is an error: duplicates pointer
  {
    std::print ("--> We have purposely duplicated an element of pDV: ");
    std::ostream_iterator<int*> os(cout, " ");
    std::copy(pDV->begin(), pDV->end(), os);
  }
  std::println ("\n---> now we should see a warning message from ~DataVector");
  delete pDV; pDV=0;
  std::print ("<--- ~DataVector returns");
  return 0;
}
