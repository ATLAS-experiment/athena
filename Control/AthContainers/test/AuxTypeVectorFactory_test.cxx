/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/AuxTypeVectorFactory_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Regression tests for AuxTypeVectorFactory.
 */


#undef NDEBUG


#include "AthContainers/tools/AuxTypeVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "TestTools/TestAlloc.h"
#include <iostream>
#include <cassert>

#ifndef XAOD_STANDALONE
#include "AthLinks/ElementLink.h"
#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF (std::vector<int*>, 28374627, 0)
#endif


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


template <class T>
T makeT(int x=0) { return T(x); }

template<>
bool makeT<bool>(int x) { return (x&1) != 0; }


template <class T, template<typename> class ALLOC = std::allocator>
void test_vector()
{
  auto makeT = [] (int x = 0) { return ::makeT<T>(x); };

  SG::AuxTypeVectorFactory<T, ALLOC<T> > fac;
  assert (fac.getEltSize() == sizeof(T));
  assert (!fac.isDynamic());
  if (typeid(T) == typeid(bool))
    assert (fac.tiVec() == &typeid (std::vector<char, ALLOC<char> >));
  else
    assert (fac.tiVec() == &typeid (std::vector<T, ALLOC<T> >));

  std::unique_ptr<SG::IAuxTypeVector> v = fac.create (1, 10, 20);
  assert (v->auxid() == 1);
  T* ptr = reinterpret_cast<T*> (v->toPtr());
  ptr[0] = makeT(20);
  ptr[1] = makeT(2);

  std::unique_ptr<SG::IAuxTypeVector> v2 = fac.create (1, 10, 20);
  assert (v2->auxid() == 1);
  T* ptr2 = reinterpret_cast<T*> (v2->toPtr());
  fac.copy (ptr2, 0, ptr, 1);
  fac.copy (ptr2, 1, ptr, 0);
  assert (ptr2[0] == makeT(2));
  assert (ptr2[1] == makeT(20));

  AuxVectorData_test avd1;
  AuxVectorData_test avd2;
  AuxStoreInternal_test store1;
  AuxStoreInternal_test store2;
  avd1.setStore (&store1);
  avd2.setStore (&store2);
  store1.addVector (std::move(v), false);
  store2.addVector (std::move(v2), false);

  ptr[0] = makeT(1);
  ptr[1] = makeT(2);
  ptr[2] = makeT(3);
  ptr2[0] = makeT(10);
  ptr2[1] = makeT(11);
  ptr2[2] = makeT(12);

  fac.swap (1, avd2, 0, avd1, 1, 2);
  assert (ptr[0] == makeT(1));
  assert (ptr[1] == makeT(10));
  assert (ptr[2] == makeT(11));
  assert (ptr2[0] == makeT(2));
  assert (ptr2[1] == makeT(3));
  assert (ptr2[2] == makeT(12));

  fac.clear (ptr2, 0);
  assert (ptr2[0] == makeT());
  assert (ptr2[1] == makeT(3));

  using vector_type = typename SG::AuxDataTraits<T, ALLOC<T> >::vector_type;
  vector_type* vec3 = new vector_type;
  vec3->push_back (makeT(3));
  vec3->push_back (makeT(2));
  vec3->push_back (makeT(1));
  std::unique_ptr<SG::IAuxTypeVector> v3 = fac.createFromData (1, vec3, false, true);
  assert (v3->auxid() == 1);
  assert (v3->size() == 3);
  T* ptr3 = reinterpret_cast<T*> (v3->toPtr());
  assert (ptr3[0] == makeT(3));
  assert (ptr3[1] == makeT(2));
  assert (ptr3[2] == makeT(1));
}


template <class T, template<typename> class ALLOC = std::allocator>
void test_vector2()
{
  auto makeT = [] (int x = 0) { return ::makeT<T>(x); };

  SG::AuxTypeVectorFactory<T, ALLOC<T> > fac;
  auto vec4 = new SG::PackedContainer<T, ALLOC<T> >;
  vec4->push_back (makeT(4));
  vec4->push_back (makeT(3));
  vec4->push_back (makeT(2));
  vec4->push_back (makeT(1));
  std::unique_ptr<SG::IAuxTypeVector> v4 = fac.createFromData (1, vec4, true, true);
  assert (v4->auxid() == 1);
  assert (v4->size() == 4);
  T* ptr4 = reinterpret_cast<T*> (v4->toPtr());
  assert (ptr4[0] == makeT(4));
  assert (ptr4[1] == makeT(3));
  assert (ptr4[2] == makeT(2));
  assert (ptr4[3] == makeT(1));
}


void test1()
{
  std::cout << "test1\n";
  test_vector<int>();
  test_vector2<int>();
  test_vector<bool>();
  test_vector<float>();
  test_vector2<float>();

  test_vector<int, Athena_test::TestAlloc>();
  test_vector2<int, Athena_test::TestAlloc>();
  test_vector<bool, Athena_test::TestAlloc>();
  test_vector<float, Athena_test::TestAlloc>();
  test_vector2<float, Athena_test::TestAlloc>();

  {
    SG::AuxTypeVectorFactory<int> fac1;
    assert (fac1.tiAlloc() == &typeid(std::allocator<int>));
    assert (fac1.tiAllocName() == "std::allocator<int>");
  }
  {
    SG::AuxTypeVectorFactory<int, Athena_test::TestAlloc<int> > fac2;
    assert (fac2.tiAlloc() == &typeid(Athena_test::TestAlloc<int>));
    assert (fac2.tiAllocName() == "Athena_test::TestAlloc<int>");
  }
  {
    SG::AuxTypeVectorFactory<std::vector<int> > fac3;
    assert (fac3.tiAlloc() == &typeid(std::allocator<std::vector<int> >));
    assert (fac3.tiAllocName() == "std::allocator<std::vector<int> >");
  }
}


// testing copyForOutput.
void test2()
{
  std::cout << "test2\n";

#ifndef XAOD_STANDALONE
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();

  typedef ElementLink<std::vector<int*> > EL;
  EL elv[10];
  elv[1] = EL (123, 10);

  SG::AuxTypeVectorFactory<EL> ve1;
  SG::AuxTypeVectorFactory<std::vector<EL> > ve2;

  ve1.copyForOutput (elv, 2, elv, 1);
  assert (elv[2].key() == 123);
  assert (elv[2].index() == 10);

  std::vector<EL> velv[10];
  velv[1].push_back (EL (123, 5));
  velv[1].push_back (EL (123, 6));
  ve2.copyForOutput (velv, 2, velv, 1);
  assert (velv[2][0].key() == 123);
  assert (velv[2][0].index() == 5);
  assert (velv[2][1].key() == 123);
  assert (velv[2][1].index() == 6);

  store->remap (123, 456, 10, 20);

  ve1.copyForOutput (elv, 5, elv, 1);
  assert (elv[5].key() == 456);
  assert (elv[5].index() == 20);

  store->remap (123, 456, 6, 12);
  ve2.copyForOutput (velv, 5, velv, 1);
  assert (velv[5][0].key() == 123);
  assert (velv[5][0].index() == 5);
  assert (velv[5][1].key() == 456);
  assert (velv[5][1].index() == 12);
#endif
}


int main()
{
  std::cout << "AuxTypeVectorFactory_test\n";
  test1();
  test2();
  return 0;
}
