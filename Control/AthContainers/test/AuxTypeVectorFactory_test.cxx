/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

  virtual size_t size_v() const override { return 10; }
  virtual size_t capacity_v() const override { return 20; }
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
T makeT(int x) { return T(x); }

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

  std::unique_ptr<SG::IAuxTypeVector> v = fac.create (1, 10, 20, false);
  assert (v->auxid() == 1);
  assert (!v->isLinked());
  T* ptr = reinterpret_cast<T*> (v->toPtr());
  ptr[0] = makeT(20);
  ptr[1] = makeT(2);

  std::unique_ptr<SG::IAuxTypeVector> v2 = fac.create (1, 10, 20, true);
  assert (v2->auxid() == 1);
  assert (v2->isLinked());
  T* ptr2 = reinterpret_cast<T*> (v2->toPtr());

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

  fac.copy (1, avd2, 0, avd1, 1, 1);
  fac.copy (1, avd2, 1, avd1, 0, 1);
  assert (ptr2[0] == makeT(10));
  assert (ptr2[1] == makeT(1));

  ptr2[0] = makeT(10);
  ptr2[1] = makeT(11);

  fac.clear (1, avd2, 0, 2);
  assert (ptr2[0] == makeT());
  assert (ptr2[1] == makeT());
  assert (ptr2[2] == makeT(12));

  using vector_type = typename SG::AuxDataTraits<T, ALLOC<T> >::vector_type;
  vector_type* vec3 = new vector_type;
  vec3->push_back (makeT(3));
  vec3->push_back (makeT(2));
  vec3->push_back (makeT(1));
  std::unique_ptr<SG::IAuxTypeVector> v3 = fac.createFromData (1, vec3, nullptr, false, true, false);
  assert (v3->auxid() == 1);
  assert (v3->size() == 3);
  assert (!v3->isLinked());
  T* ptr3 = reinterpret_cast<T*> (v3->toPtr());
  assert (ptr3[0] == makeT(3));
  assert (ptr3[1] == makeT(2));
  assert (ptr3[2] == makeT(1));

  vector_type* vec4 = new vector_type;
  std::unique_ptr<SG::IAuxTypeVector> v4 = fac.createFromData (1, vec4, nullptr, false, true, true);
  assert (v4->auxid() == 1);
  assert (v4->isLinked());

  // Testing range copy, with and without overlap.
  for (size_t i = 0; i < 10; i++) {
    ptr[i] = makeT(i);
    ptr2[i] = makeT(i+10);
  }
 
  auto checkvec = [&makeT] (const T* p, const std::vector<int>& exp)
    {
      for (size_t i = 0; i < exp.size(); i++) {
        assert (p[i] == makeT(exp[i]));
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

  fac.copy (1, avd2, 2, avd1, 6, 3);
  checkvec (ptr, {0, 2, 5, 5, 6, 5, 5, 6, 7, 9});
  checkvec (ptr2, {10, 11, 5, 6, 7, 15, 16, 17, 18, 19});

  AuxVectorData_test avd3;
  AuxStoreInternal_test store3;
  avd3.setStore (&store3);

  fac.copy (1, avd1, 3, avd3, 3, 3);
  checkvec (ptr, {0, 2, 5, 0, 0, 0, 5, 6, 7, 9});
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
  std::unique_ptr<SG::IAuxTypeVector> v4 = fac.createFromData (1, vec4, nullptr, true, true, false);
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

  SG::AuxTypeVectorFactory<EL> ve1;
  SG::AuxTypeVectorFactory<std::vector<EL> > ve2;

  std::unique_ptr<SG::IAuxTypeVector> v1 = ve1.create (1, 10, 10, false);
  EL* elv = reinterpret_cast<EL*> (v1->toPtr());
  elv[1] = EL (123, 10);
  elv[2] = EL (124, 11);

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  store1.addVector (std::move(v1), false);

  ve1.copyForOutput (1, avd1, 2, avd1, 1, 2);
  assert (elv[2].key() == 123);
  assert (elv[2].index() == 10);
  assert (elv[3].key() == 124);
  assert (elv[3].index() == 11);

  std::unique_ptr<SG::IAuxTypeVector> v2 = ve2.create (2, 10, 10, false);
  std::vector<EL>* velv = reinterpret_cast<std::vector<EL>*> (v2->toPtr());
  velv[1].push_back (EL (123, 5));
  velv[1].push_back (EL (123, 6));
  velv[2].push_back (EL (124, 7));
  velv[2].push_back (EL (124, 8));

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  store2.addVector (std::move(v2), false);

  ve2.copyForOutput (2, avd2, 2, avd2, 1, 2);
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

  ve1.copyForOutput (1, avd1, 5, avd1, 2, 2);
  assert (elv[5].key() == 456);
  assert (elv[5].index() == 20);
  assert (elv[6].key() == 457);
  assert (elv[6].index() == 21);

  ve2.copyForOutput (2, avd2, 5, avd2, 2, 2);
  assert (velv[5][0].key() == 123);
  assert (velv[5][0].index() == 5);
  assert (velv[5][1].key() == 456);
  assert (velv[5][1].index() == 12);
  assert (velv[6][0].key() == 124);
  assert (velv[6][0].index() == 7);
  assert (velv[6][1].key() == 457);
  assert (velv[6][1].index() == 28);
#endif
}


int main()
{
  std::cout << "AuxTypeVectorFactory_test\n";
  test1();
  test2();
  return 0;
}
