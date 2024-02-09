/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersRoot/test/RootAuxVectorFactory_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Regression test for RootAuxVectorFactory.
 */


#undef NDEBUG
#include "AthContainersRoot/RootAuxVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainersRoot/test/Foo.h"
#include "AthLinks/ElementLink.h"
#include "SGTools/TestStore.h"
#include "CxxUtils/StrFormat.h"
#include "TClass.h"
#include <iostream>
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


std::string str (int x)
{
  return CxxUtils::strformat ("%d", x);
}


void test1()
{
  std::cout << "test1\n";
  TClass* cl = TClass::GetClass ("vector<int>");
  SG::RootAuxVectorFactory fac (cl);
  SG::IAuxTypeVector* vec = new SG::RootAuxVector (&fac, 1, 10, 20);
  assert (vec->size() == 10);
  assert (vec->auxid() == 1);
  int* ptr = reinterpret_cast<int*> (vec->toPtr());
  assert (std::as_const (*vec).toPtr() == ptr);
  for (int i=0; i < 10; i++)
    ptr[i] = i+1;
  assert (vec->resize (100) == false);
  vec->reserve (100);
  ptr = reinterpret_cast<int*> (vec->toPtr());
  for (int i=0; i < 10; i++)
    assert (ptr[i] == i+1);
  std::vector<int>* pvec = reinterpret_cast<std::vector<int>*> (vec->toVector());
  assert (pvec->size() == 100);
  for (int i=0; i < 10; i++)
    assert ((*pvec)[i] == i+1);

  // 1 2 3 4 5 6 7 8 9 10

  assert (vec->resize (10) == true);
  vec->shift (7, -3);
  assert (vec->size() == 7);
  // 1 2 3 4 8 9 10
  ptr = reinterpret_cast<int*> (vec->toPtr());
  assert (ptr[0] == 1);
  assert (ptr[1] == 2);
  assert (ptr[2] == 3);
  assert (ptr[3] == 4);
  assert (ptr[4] == 8);
  assert (ptr[5] == 9);
  assert (ptr[6] == 10);

  vec->shift (3, 2);
  assert (vec->size() == 9);
  // 1 2 3 0 0 4 8 9 10
  ptr = reinterpret_cast<int*> (vec->toPtr());
  assert (ptr[0] == 1);
  assert (ptr[1] == 2);
  assert (ptr[2] == 3);
  assert (ptr[3] == 0);
  assert (ptr[4] == 0);
  assert (ptr[5] == 4);
  assert (ptr[6] == 8);
  assert (ptr[7] == 9);
  assert (ptr[8] == 10);

  std::unique_ptr<SG::IAuxTypeVector> vec2 = vec->clone();
  assert (vec2->auxid() == 1);
  int* ptr2 = reinterpret_cast<int*> (vec2->toPtr());
  assert (ptr != ptr2);
  assert (ptr2[0] == 1);
  assert (ptr2[1] == 2);
  assert (ptr2[2] == 3);
  assert (ptr2[3] == 0);
  assert (ptr2[4] == 0);
  assert (ptr2[5] == 4);
  assert (ptr2[6] == 8);
  assert (ptr2[7] == 9);
  assert (ptr2[8] == 10);

  std::vector<int> vec3 { 20, 21, 22, 23, 24 };
  assert (vec->insertMove (3, vec3.data(), vec3.data() + 5));
  assert (vec->size() == 14);
  assert (ptr[0] == 1);
  assert (ptr[1] == 2);
  assert (ptr[2] == 3);
  assert (ptr[3] == 20);
  assert (ptr[4] == 21);
  assert (ptr[5] == 22);
  assert (ptr[6] == 23);
  assert (ptr[7] == 24);
  assert (ptr[8] == 0);
  assert (ptr[9] == 0);
  assert (ptr[10] == 4);
  assert (ptr[11] == 8);
  assert (ptr[12] == 9);
  assert (ptr[13] == 10);

  std::vector<int> vec4 { 30, 31, 32, 33, 34 };
  assert (vec->insertMove (14, vec4.data(), vec4.data() + 5));
  assert (vec->size() == 19);
  assert (ptr[0] == 1);
  assert (ptr[1] == 2);
  assert (ptr[2] == 3);
  assert (ptr[3] == 20);
  assert (ptr[4] == 21);
  assert (ptr[5] == 22);
  assert (ptr[6] == 23);
  assert (ptr[7] == 24);
  assert (ptr[8] == 0);
  assert (ptr[9] == 0);
  assert (ptr[10] == 4);
  assert (ptr[11] == 8);
  assert (ptr[12] == 9);
  assert (ptr[13] == 10);
  assert (ptr[14] == 30);
  assert (ptr[15] == 31);
  assert (ptr[16] == 32);
  assert (ptr[17] == 33);
  assert (ptr[18] == 34);

  assert (vec->resize (0) == false);
  assert (vec->toPtr() == 0);

  delete vec;
}


void test2()
{
  std::cout << "test2\n";
  TClass* cl = TClass::GetClass ("vector<std::string>");
  SG::RootAuxVectorFactory fac (cl);
  SG::IAuxTypeVector* vec = new SG::RootAuxVector (&fac, 1, 10, 10);
  assert (vec->size() == 10);
  assert (vec->auxid() == 1);
  std::string* ptr = reinterpret_cast<std::string*> (vec->toPtr());
  for (int i=0; i < 10; i++)
    ptr[i] = str(i+1);
  vec->resize (100);
  vec->reserve (100);
  ptr = reinterpret_cast<std::string*> (vec->toPtr());
  for (int i=0; i < 10; i++)
    assert (ptr[i] == str(i+1));
  std::vector<std::string>* pvec = reinterpret_cast<std::vector<std::string>*> (vec->toVector());
  assert (pvec->size() == 100);
  for (int i=0; i < 10; i++)
    assert ((*pvec)[i] == str(i+1));

  // 1 2 3 4 5 6 7 8 9 10

  vec->resize (10);
  vec->shift (7, -3);
  assert (vec->size() == 7);
  // 1 2 3 4 8 9 10
  ptr = reinterpret_cast<std::string*> (vec->toPtr());
  assert (ptr[0] == str(1));
  assert (ptr[1] == str(2));
  assert (ptr[2] == str(3));
  assert (ptr[3] == str(4));
  assert (ptr[4] == str(8));
  assert (ptr[5] == str(9));
  assert (ptr[6] == str(10));

  vec->shift (3, 2);
  assert (vec->size() == 9);
  // 1 2 3 "" "" 4 8 9 10
  ptr = reinterpret_cast<std::string*> (vec->toPtr());
  assert (ptr[0] == str(1));
  assert (ptr[1] == str(2));
  assert (ptr[2] == str(3));
  assert (ptr[3] == "");
  assert (ptr[4] == "");
  assert (ptr[5] == str(4));
  assert (ptr[6] == str(8));
  assert (ptr[7] == str(9));
  assert (ptr[8] == str(10));

  std::vector<std::string> vec3 { str(20), str(21), str(22), str(23), str(24) };
  assert (vec->insertMove (3, vec3.data(), vec3.data() + 5));
  assert (vec->size() == 14);
  assert (ptr[0] == str(1));
  assert (ptr[1] == str(2));
  assert (ptr[2] == str(3));
  assert (ptr[3] == str(20));
  assert (ptr[4] == str(21));
  assert (ptr[5] == str(22));
  assert (ptr[6] == str(23));
  assert (ptr[7] == str(24));
  assert (ptr[8] == "");
  assert (ptr[9] == "");
  assert (ptr[10] == str(4));
  assert (ptr[11] == str(8));
  assert (ptr[12] == str(9));
  assert (ptr[13] == str(10));

  std::vector<std::string> vec4 { str(30), str(31), str(32), str(33), str(34) };
  assert (vec->insertMove (14, vec4.data(), vec4.data() + 5));
  assert (vec->size() == 19);
  assert (ptr[0] == str(1));
  assert (ptr[1] == str(2));
  assert (ptr[2] == str(3));
  assert (ptr[3] == str(20));
  assert (ptr[4] == str(21));
  assert (ptr[5] == str(22));
  assert (ptr[6] == str(23));
  assert (ptr[7] == str(24));
  assert (ptr[8] == "");
  assert (ptr[9] == "");
  assert (ptr[10] == str(4));
  assert (ptr[11] == str(8));
  assert (ptr[12] == str(9));
  assert (ptr[13] == str(10));
  assert (ptr[14] == str(30));
  assert (ptr[15] == str(31));
  assert (ptr[16] == str(32));
  assert (ptr[17] == str(33));
  assert (ptr[18] == str(34));

  delete vec;
}


void test3()
{
  std::cout << "test3\n";
  TClass* cl = TClass::GetClass ("vector<int>");
  SG::RootAuxVectorFactory fac (cl);
  assert (fac.vecClass() == cl);
  assert (fac.getEltSize() == sizeof(int));
  assert (fac.tiVec() == &typeid(std::vector<int>));
  assert (fac.isDynamic());
  std::unique_ptr<SG::IAuxTypeVector> vec = fac.create (1, 10, 10);
  assert (vec->size() == 10);
  assert (vec->auxid() == 1);

  int* ptr = reinterpret_cast<int*> (vec->toPtr());

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  store1.addVector (std::move(vec), false);

  ptr[0] = 1;
  ptr[1] = 2;
  ptr[2] = 3;
  ptr[3] = 4;
  fac.copy (1, avd1, 5, avd1, 1, 1);
  assert (ptr[1] == 2);
  assert (ptr[5] == 2);

  fac.clear (1, avd1, 1, 2);
  assert (ptr[1] == 0);
  assert (ptr[2] == 0);
  assert (ptr[3] == 4);

  ptr[0] = 1;
  ptr[1] = 2;
  ptr[2] = 3;
  ptr[5] = 10;
  ptr[6] = 11;
  ptr[7] = 12;
  fac.swap (1, avd1, 0, avd1, 5, 2);
  assert (ptr[0] == 10);
  assert (ptr[1] == 11);
  assert (ptr[2] == 3);
  assert (ptr[5] == 1);
  assert (ptr[6] == 2);
  assert (ptr[7] == 12);

  // Testing range copy, with and without overlap.
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

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac.create (1, 10, 10);
  int* ptr2 = reinterpret_cast<int*> (vec2->toPtr());
  store2.addVector (std::move(vec2), false);

  for (size_t i = 0; i < 10; i++) {
    ptr2[i] = i+10;
  }

  fac.copy (1, avd2, 2, avd1, 6, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});
  checkvec (ptr2, {10, 11, 5, 6, 7, 15, 16, 17, 18, 19});

  AuxVectorData_test avd3;
  AuxStoreInternal_test store3;
  avd3.setStore (&store3);

  fac.copy (1, avd1, 3, avd3, 3, 3);
  checkvec (ptr, {0, 2, 5, 0, 0, 0, 5, 6, 7, 9});
}


void test4()
{
  std::cout << "test4\n";
  TClass* cl = TClass::GetClass ("vector<std::string>");
  SG::RootAuxVectorFactory fac (cl);
  assert (fac.vecClass() == cl);
  assert (fac.getEltSize() == sizeof(std::string));
  assert (fac.tiVec() == &typeid(std::vector<std::string>));
  assert (fac.isDynamic());
  std::unique_ptr<SG::IAuxTypeVector> vec = fac.create (1, 10, 10);
  assert (vec->size() == 10);

  std::string* ptr = reinterpret_cast<std::string*> (vec->toPtr());
  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  store1.addVector (std::move(vec), false);

  ptr[0] = "1";
  ptr[1] = "2";
  ptr[2] = "3";
  ptr[3] = "4";
  fac.copy (1, avd1, 5, avd1, 1, 1);
  assert (ptr[1] == "2");
  assert (ptr[5] == "2");

  fac.clear (1, avd1, 1, 2);
  assert (ptr[1] == "");
  assert (ptr[2] == "");
  assert (ptr[3] == "4");

  fac.swap (1, avd1, 0, avd1, 5, 1);
  assert (ptr[0] == "2");
  assert (ptr[5] == "1");

  // Testing range copy, with and without overlap.
  for (size_t i = 0; i < 10; i++) {
    ptr[i] = std::to_string(i);
  }
 
  auto checkvec = [] (const std::string* p, const std::vector<int>& exp)
    {
      for (size_t i = 0; i < exp.size(); i++) {
        if (exp[i] >= 0)
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

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac.create (1, 10, 10);
  std::string* ptr2 = reinterpret_cast<std::string*> (vec2->toPtr());
  store2.addVector (std::move(vec2), false);

  for (size_t i = 0; i < 10; i++) {
    ptr2[i] = std::to_string(i+10);
  }

  fac.copy (1, avd2, 2, avd1, 6, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});
  checkvec (ptr2, {10, 11, 5, 6, 7, 15, 16, 17, 18, 19});

  AuxVectorData_test avd3;
  AuxStoreInternal_test store3;
  avd3.setStore (&store3);

  fac.copy (1, avd1, 3, avd3, 3, 3);
  checkvec (ptr, {0, 2, 5, -1, -1, -1, 5, 6, 7, 9});
}


void test5()
{
  std::cout << "test5\n";

  TClass* cl = TClass::GetClass ("vector<int>");
  SG::RootAuxVectorFactory fac (cl);

  std::vector<int>* vec1 = new std::vector<int>;
  vec1->push_back(3);
  vec1->push_back(2);
  vec1->push_back(1);
  std::unique_ptr<SG::IAuxTypeVector> v1 = fac.createFromData (1, vec1, false, true);
  assert (v1->size() == 3);
  assert (v1->auxid() == 1);
  int* ptr1 = reinterpret_cast<int*> (v1->toPtr());
  assert (ptr1[0] == 3);
  assert (ptr1[1] == 2);
  assert (ptr1[2] == 1);

  std::vector<int>* vec2 = new std::vector<int>;
  vec2->push_back(4);
  vec2->push_back(5);
  std::unique_ptr<SG::IAuxTypeVector> v2 = fac.createFromData (1, vec2, false, false);
  assert (v2->size() == 2);
  assert (v2->auxid() == 1);
  int* ptr2 = reinterpret_cast<int*> (v2->toPtr());
  assert (ptr2[0] == 4);
  assert (ptr2[1] == 5);
  delete vec2;
}


// Testing copyForOutput.
void test6()
{
  std::cout << "test6\n";

  typedef ElementLink<std::vector<AthContainersRootTest::Foo*> > EL;

  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();

  TClass* cl1 = TClass::GetClass ("std::vector<ElementLink<std::vector<AthContainersRootTest::Foo*> > >");
  SG::RootAuxVectorFactory fac1 (cl1);

  std::unique_ptr<SG::IAuxTypeVector> vec1 = fac1.create (1, 10, 10);
  EL* elv = reinterpret_cast<EL*> (vec1->toPtr());
  elv[1] = EL (123, 10);
  elv[2] = EL (124, 11);

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  store1.addVector (std::move(vec1), false);

  fac1.copyForOutput (1, avd1, 2, avd1, 1, 2);
  assert (elv[2].key() == 123);
  assert (elv[2].index() == 10);
  assert (elv[3].key() == 124);
  assert (elv[3].index() == 11);

  TClass* cl2 = TClass::GetClass ("std::vector<std::vector<ElementLink<std::vector<AthContainersRootTest::Foo*> > > >");
  SG::RootAuxVectorFactory fac2 (cl2);

  std::unique_ptr<SG::IAuxTypeVector> vec2 = fac2.create (2, 10, 10);
  std::vector<EL>* velv = reinterpret_cast<std::vector<EL>*> (vec2->toPtr());
  store1.addVector (std::move(vec2), false);

  velv[1].push_back (EL (123, 5));
  velv[1].push_back (EL (123, 6));
  velv[2].push_back (EL (124, 7));
  velv[2].push_back (EL (124, 8));
  fac2.copyForOutput (2, avd1, 2, avd1, 1, 2);
  assert (velv[2][0].key() == 123);
  assert (velv[2][0].index() == 5);
  assert (velv[2][1].key() == 123);
  assert (velv[2][1].index() == 6);
  assert (velv[3][0].key() == 124);
  assert (velv[3][0].index() == 7);
  assert (velv[3][1].key() == 124);
  assert (velv[3][1].index() == 8);

  store->remap (123, 456, 10, 20);
  store->remap (124, 457, 11, 21);
  store->remap (123, 456, 6, 12);
  store->remap (124, 457, 8, 28);

  fac1.copyForOutput (1, avd1, 5, avd1, 2, 2);
  assert (elv[5].key() == 456);
  assert (elv[5].index() == 20);
  assert (elv[6].key() == 457);
  assert (elv[6].index() == 21);

  fac2.copyForOutput (2, avd1, 5, avd1, 2, 2);
  assert (velv[5][0].key() == 123);
  assert (velv[5][0].index() == 5);
  assert (velv[5][1].key() == 456);
  assert (velv[5][1].index() == 12);
  assert (velv[6][0].key() == 124);
  assert (velv[6][0].index() == 7);
  assert (velv[6][1].key() == 457);
  assert (velv[6][1].index() == 28);

  TClass* cl3 = TClass::GetClass ("std::vector<ElementLink<std::vector<AthContainersRootTest::Foo> > >");
  SG::RootAuxVectorFactory fac3 (cl3);

  typedef ElementLink<std::vector<AthContainersRootTest::Foo> > EL2;
  std::unique_ptr<SG::IAuxTypeVector> vec3 = fac3.create (3, 10, 10);
  EL2* elv2 = reinterpret_cast<EL2*> (vec3->toPtr());
  store1.addVector (std::move(vec3), false);
  elv2[1] = EL2 (123, 10);
  elv2[2] = EL2 (124, 11);

  fac3.copyForOutput (3, avd1, 2, avd1, 1, 2);

  TClass* cl4 = TClass::GetClass ("std::vector<std::vector<ElementLink<std::vector<AthContainersRootTest::Foo> > > >");
  SG::RootAuxVectorFactory fac4 (cl4);

  std::unique_ptr<SG::IAuxTypeVector> vec4 = fac4.create (4, 10, 10);
  std::vector<EL2>* velv2 = reinterpret_cast<std::vector<EL2>*> (vec4->toPtr());
  store1.addVector (std::move(vec4), false);
  velv2[1].push_back (EL2 (123, 5));
  velv2[1].push_back (EL2 (123, 6));
  velv2[2].push_back (EL2 (456, 7));
  velv2[2].push_back (EL2 (456, 8));

  fac4.copyForOutput (4, avd1, 2, avd1, 1, 2);
}


// Testing tiAllocName.
void test7()
{
  std::cout << "test7\n";

  {
    TClass* cl = TClass::GetClass ("vector<int>");
    SG::RootAuxVectorFactory fac (cl);
    assert (fac.tiAlloc() == nullptr);
    assert (fac.tiAllocName() == "std::allocator<int>");
  }
  {
    TClass* cl = TClass::GetClass ("vector<int,std::pmr::polymorphic_allocator<int> >");
    SG::RootAuxVectorFactory fac (cl);
    assert (fac.tiAlloc() == nullptr);
    assert (fac.tiAllocName() == "std::pmr::polymorphic_allocator<int>");
  }
}


int main()
{
  std::cout << "RootAuxVectorFactory_test\n";
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  return 0;
}
