/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file xAODRootAccess/test/ut_xaodrootaccess_tauxvectorfactory_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2023
 * @brief Unit tests for TAuxVectorFactory.  (sadly incomplete)
 */


#undef NDEBUG
#include "xAODRootAccess/tools/TAuxVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "TClass.h"
#include <iostream>
#include <sstream>
#include <cassert>


namespace SG {


class AuxVectorData_test
  : public AuxVectorData
{
public:
  using AuxVectorData::setStore;

  virtual size_t size_v() const { return 10; }
  virtual size_t capacity_v() const { return 20; }
};


class AuxStoreInternal_test
  : public AuxStoreInternal
{
public:
  using AuxStoreInternal::addVector;
};


} // namespace SG



using SG::AuxVectorData;
using SG::AuxVectorData_test;
using SG::AuxStoreInternal_test;


// Test with int.
void test1()
{
  std::cout << "test1\n";

  TClass* cl = TClass::GetClass ("vector<int>");
  xAOD::TAuxVectorFactory fac (cl);
  assert (fac.getEltSize() == sizeof(int));
  assert (fac.tiVec() == &typeid(std::vector<int>));
  assert (fac.isDynamic());

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  std::unique_ptr<SG::IAuxTypeVector> vec = fac.create (1, 10, 10, false);
  int* ptr = reinterpret_cast<int*> (vec->toPtr());
  store1.addVector (std::move(vec), false);

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac.create (1, 10, 10, false);
  int* ptr2 = reinterpret_cast<int*> (vec2->toPtr());
  store2.addVector (std::move(vec2), false);

  // Swap

  ptr[0] = 1;
  ptr[1] = 2;
  ptr[2] = 3;
  ptr[3] = 4;
  fac.swap (1, avd1, 0, avd1, 2, 2);
  assert (ptr[0] == 3);
  assert (ptr[1] == 4);
  assert (ptr[2] == 1);
  assert (ptr[3] == 2);

  ptr2[0] = 11;
  ptr2[1] = 12;
  ptr2[2] = 13;
  fac.swap (1, avd2, 0, avd1, 1, 2);
  assert (ptr[0] == 3);
  assert (ptr[1] == 11);
  assert (ptr[2] == 12);
  assert (ptr[3] == 2);
  assert (ptr2[0] == 4);
  assert (ptr2[1] == 1);
  assert (ptr2[2] == 13);

  // Clear

  fac.clear (1, avd1, 0, 2);
  assert (ptr[0] == 0);
  assert (ptr[1] == 0);
  assert (ptr[2] == 12);

  // Copy

  for (size_t i = 0; i < 10; i++) {
    ptr[i] = i;
  }
 
  auto checkvec = [] (const int* p, const std::vector<int>& exp)
    {
      for (size_t i = 0; i < exp.size(); i++) {
        assert (p[i] == exp[i]);
      }
    };

  fac.copy (1, avd1, 3, avd1, 4, 0);
  checkvec (ptr, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

  fac.copy (1, avd1, 1, avd1, 2, 3);
  checkvec (ptr, {0, 2, 3, 4, 4, 5, 6, 7, 8, 9});

  fac.copy (1, avd1, 6, avd1, 5, 3);
  checkvec (ptr, {0, 2, 3, 4, 4, 5, 5, 6, 7, 9});

  fac.copy (1, avd1, 2, avd1, 5, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});

  for (size_t i = 0; i < 10; i++) {
    ptr2[i] = i+10;
  }

  fac.copy (1, avd2, 2, avd1, 6, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});
  checkvec (ptr2, {10, 11, 5, 6, 7, 15, 16, 17, 18, 19});
}


// Test with string.
void test2()
{
  std::cout << "test2\n";

  TClass* cl = TClass::GetClass ("vector<std::string>");
  xAOD::TAuxVectorFactory fac (cl);
  assert (fac.getEltSize() == sizeof(std::string));
  assert (fac.tiVec() == &typeid(std::vector<std::string>));
  assert (fac.isDynamic());

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  std::unique_ptr<SG::IAuxTypeVector> vec = fac.create (1, 10, 10, false);
  assert (vec->auxid() == 1);
  assert (!vec->isLinked());
  std::string* ptr = reinterpret_cast<std::string*> (vec->toPtr());
  store1.addVector (std::move(vec), false);

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac.create (1, 10, 10, true);
  assert (vec2->auxid() == 1);
  assert (vec2->isLinked());
  std::string* ptr2 = reinterpret_cast<std::string*> (vec2->toPtr());
  store2.addVector (std::move(vec2), false);

  // Swap

  ptr[0] = "1";
  ptr[1] = "2";
  ptr[2] = "3";
  ptr[3] = "4";
  fac.swap (1, avd1, 0, avd1, 2, 2);
  assert (ptr[0] == "3");
  assert (ptr[1] == "4");
  assert (ptr[2] == "1");
  assert (ptr[3] == "2");

  ptr2[0] = "11";
  ptr2[1] = "12";
  ptr2[2] = "13";
  fac.swap (1, avd2, 0, avd1, 1, 2);
  assert (ptr[0] == "3");
  assert (ptr[1] == "11");
  assert (ptr[2] == "12");
  assert (ptr2[0] == "4");
  assert (ptr2[1] == "1");
  assert (ptr2[2] == "13");

  // Clear

  fac.clear (1, avd1, 0, 2);
  assert (ptr[0] == "");
  assert (ptr[1] == "");
  assert (ptr[2] == "12");

  // Copy

  for (size_t i = 0; i < 10; i++) {
    ptr[i] = std::to_string(i);
  }
 
  auto checkvec = [] (const std::string* p, const std::vector<int>& exp)
    {
      for (size_t i = 0; i < exp.size(); i++) {
        if (exp[i] >= 0)
          //cppcheck-suppress assertWithSideEffect
          assert (p[i] == std::to_string(exp[i]));
        else
          assert (p[i] == "");
      }
    };

  fac.copy (1, avd1, 3, avd1, 4, 0);
  checkvec (ptr, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

  fac.copy (1, avd1, 1, avd1, 2, 3);
  checkvec (ptr, {0, 2, 3, 4, 4, 5, 6, 7, 8, 9});

  fac.copy (1, avd1, 6, avd1, 5, 3);
  checkvec (ptr, {0, 2, 3, 4, 4, 5, 5, 6, 7, 9});

  fac.copy (1, avd1, 2, avd1, 5, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});

  for (size_t i = 0; i < 10; i++) {
    ptr2[i] = std::to_string(i+10);
  }

  fac.copy (1, avd2, 2, avd1, 6, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});
  checkvec (ptr2, {10, 11, 5, 6, 7, 15, 16, 17, 18, 19});
}


int main()
{
  std::cout << "xAODRootAccess/ut_xaodrootaccess_tauxvectorfactory_test\n";
  test1();
  test2();
  return 0;
}
