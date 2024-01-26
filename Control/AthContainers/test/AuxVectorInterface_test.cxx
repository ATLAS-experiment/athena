/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/AuxVectorInterface_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief Regression tests for AuxVectorInterface.
 */


#undef NDEBUG
#include "AthContainers/tools/AuxVectorInterface.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include <iostream>
#include <cassert>


void test1()
{
  std::cout << "test1\n";

  SG::AuxStoreInternal store;
  SG::auxid_t ityp1 = SG::AuxTypeRegistry::instance().getAuxID<int> ("anInt");
  int* i1 = reinterpret_cast<int*> (store.getData(ityp1, 10, 20));
  i1[0] = 1;
  i1[1] = 2;

  {
    SG::AuxVectorInterface vi (store);
    assert (vi.getData<int> (ityp1, 1) == 2);
    assert (vi.size_v() == 10);
    assert (vi.capacity_v() == 10);
  }
  {
    const SG::IConstAuxStore& cstore = store;
    const SG::AuxVectorInterface vi (cstore);
    assert (vi.getData<int> (ityp1, 1) == 2);
    assert (vi.size_v() == 10);
    assert (vi.capacity_v() == 10);
  }

  int iarr[3] = {3, 4, 5};

  {
    SG::AuxVectorInterface vi (ityp1, 3, iarr);
    assert (vi.getData<int> (ityp1, 1) == 4);
    assert (vi.size_v() == 3);
    assert (vi.capacity_v() == 3);
  }
  {
    const int* ciarr = iarr;
    const SG::AuxVectorInterface vi (ityp1, 3, ciarr);
    assert (vi.getData<int> (ityp1, 1) == 4);
    assert (vi.size_v() == 3);
    assert (vi.capacity_v() == 3);
  }
}


int main()
{
  std::cout << "AthContainers/AuxVectorInterface_test\n";
  test1();
  return 0;
}
