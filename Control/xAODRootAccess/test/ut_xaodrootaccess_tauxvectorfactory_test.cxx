/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
  std::unique_ptr<SG::IAuxTypeVector> vec = fac.create (1, 10, 10);
  int* ptr = reinterpret_cast<int*> (vec->toPtr());
  store1.addVector (std::move(vec), false);

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac.create (1, 10, 10);
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
  std::unique_ptr<SG::IAuxTypeVector> vec = fac.create (1, 10, 10);
  std::string* ptr = reinterpret_cast<std::string*> (vec->toPtr());
  store1.addVector (std::move(vec), false);

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac.create (1, 10, 10);
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
}


int main()
{
  std::cout << "xAODRootAccess/ut_xaodrootaccess_tauxvectorfactory_test\n";
  test1();
  test2();
  return 0;
}
