/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecVectorFactory_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Regression tests for JaggedVecVectorFactory.
 */


#undef NDEBUG


#include "AthContainers/tools/JaggedVecVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/exceptions.h"
#include "TestTools/TestAlloc.h"
#include "TestTools/expect_exception.h"
#include "CxxUtils/ranges.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <cassert>

#ifndef XAOD_STANDALONE
#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF (std::vector<int*>, 28374627, 0)
#endif


namespace SG {
template <class T>
std::ostream& operator<< (std::ostream& s, const JaggedVecElt<T>& e)
{
  s << e.end();
  return s;
}
} // namespace SG


namespace {


template <class CONT, class ITER>
void copyRange (const CONT& c, ITER it)
{
#if HAVE_STD_RANGES
  std::ranges::copy (c, it);
#else
  std::copy (c.begin(), c.end(), it);
#endif
}


template <class T>
void compareElts (const SG::JaggedVecElt<T>* ptr, size_t n,
                  const std::vector<SG::JaggedVecElt<T> >& v)
{
  assert (n >= v.size());
  for (size_t i = 0; i  < v.size(); ++i) {
    if (ptr[i] != v[i]) {
      std::cerr << "Elt comparison failure: [";
      for (size_t ii = 0; ii  < v.size(); ++ii)
        std::cerr << ptr[ii] << " ";
      std::cerr << "] [";
      for (size_t ii = 0; ii  < v.size(); ++ii)
        std::cerr << v[ii] << " ";
      std::cerr << "]\n";
      std::abort();
    }
  }
  SG::JaggedVecElt<T> tail (v.back().end());
  for (size_t i = v.size(); i  < n; ++i) {
    assert (ptr[i] == tail);
  }
}


template <class T>
void comparePayload (const T* ptr, const std::vector<T>& v)
{
  if (!std::equal (v.begin(), v.end(), ptr)) {
    std::cerr << "Payload comparison failure: [";
    for (size_t ii = 0; ii  < v.size(); ++ii)
      std::cerr << ptr[ii] << " ";
    std::cerr << "] [";
    for (size_t ii = 0; ii  < v.size(); ++ii)
      std::cerr << v[ii] << " ";
    std::cerr << "]\n";
    std::abort();
  }
}


} // anonymous namespace


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



//using SG::AuxVectorData;
using SG::AuxVectorData_test;
using SG::AuxStoreInternal_test;


template <class T, template<typename> class ALLOC = std::allocator>
void test_vector (const std::string& name)
{
  using Elt = SG::JaggedVecElt<T>;
  using vector_type = std::vector<Elt, ALLOC<Elt> >;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_payload_id = r.getAuxID<T>
    (name + "_payload", "", SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<Elt>
    (name, "",
     SG::AuxVarFlags::None,
     foo_payload_id);

  SG::JaggedVecVectorFactory<T, ALLOC<Elt> > fac;
  assert (fac.getEltSize() == sizeof(Elt));
  assert (!fac.isDynamic());
  assert (fac.tiVec() == &typeid (vector_type));

  std::unique_ptr<SG::IAuxTypeVector> vup = fac.create (foo_id, 10, 20, false);
  SG::IAuxTypeVector* v = vup.get();
  assert (v->auxid() == foo_id);
  assert (!v->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> lvup = v->linkedVector();
  SG::IAuxTypeVector* lv = lvup.get();
  assert (lv->auxid() == foo_payload_id);
  assert (lv->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> vup2 = fac.create (foo_id, 10, 20, false);
  SG::IAuxTypeVector* v2 = vup2.get();
  assert (v2->auxid() == foo_id);
  assert (!v2->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> lvup2 = v2->linkedVector();
  SG::IAuxTypeVector* lv2 = lvup2.get();
  assert (lv2->auxid() == foo_payload_id);
  assert (lv2->isLinked());

  AuxVectorData_test avd1;
  AuxVectorData_test avd2;
  AuxStoreInternal_test store1;
  AuxStoreInternal_test store2;
  avd1.setStore (&store1);
  avd2.setStore (&store2);
  store1.addVector (std::move(vup), false);
  store1.addVector (std::move(lvup), false);
  store2.addVector (std::move(vup2), false);
  store2.addVector (std::move(lvup2), false);

  // [0, 1, 2] [3] [4, 5]
  Elt* ptr = reinterpret_cast<Elt*> (v->toPtr());
  copyRange (std::vector<Elt>{{3}, {4}, {6}}, ptr);
  std::fill_n (ptr+3, 7, Elt{6});
  lv->resize(6);
  T* lptr = reinterpret_cast<T*> (lv->toPtr());
  copyRange (std::vector<int>{0, 1, 2, 3, 4, 5}, lptr);

  // [100, 101] [102, 103, 104] [105, 106]
  Elt* ptr2 = reinterpret_cast<Elt*> (v2->toPtr());
  copyRange (std::vector<Elt>{{2}, {5}, {7}}, ptr2);
  std::fill_n (ptr2+3, 7, Elt{7});
  lv2->resize(7);
  T* lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  copyRange (std::vector<int>{100, 101, 102, 103, 104, 105, 106}, lptr2);

  fac.swap (foo_id, avd1, 0, avd2, 1, 0);
  fac.swap (foo_id, avd1, 0, avd2, 1, 2);

  // [102, 103, 104] [105, 106], [4, 5]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{3}, {5}, {7}});
  assert (lv->size() == 7);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {102, 103, 104, 105, 106, 4, 5});
  // [100, 101] [0, 1, 2] [3]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {5}, {6}});
  assert (lv2->size() == 6);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {100, 101, 0, 1, 2, 3});

  fac.swap (foo_id, avd1, 0, avd2, 2, 1);

  // [3] [105, 106], [4, 5]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{1}, {3}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {3, 105, 106, 4, 5});
  // [100, 101] [0, 1, 2] [102, 103, 104]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {5}, {8}});
  assert (lv2->size() == 8);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {100, 101, 0, 1, 2, 102, 103, 104});

  fac.swap (foo_id, avd1, 2, avd2, 0, 1);

  // 1: [3] [105, 106], [100, 101]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{1}, {3}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {3, 105, 106, 100, 101});
  // 2: [4, 5] [0, 1, 2] [102, 103, 104]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {5}, {8}});
  assert (lv2->size() == 8);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {4, 5, 0, 1, 2, 102, 103, 104});

  fac.swap (foo_id, avd2, 0, avd2, 2, 1);
  // 2: [102, 103, 104] [0, 1, 2] [4, 5]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{3}, {6}, {8}});
  assert (lv2->size() == 8);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {102, 103, 104, 0, 1, 2, 4, 5});

  fac.swap (foo_id, avd1, 2, avd1, 0, 1);
  // 1: [100, 101] [105, 106], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{2}, {4}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {100, 101, 105, 106, 3});

  fac.swap (foo_id, avd2, 0, avd2, 1, 1);
  // 2: [0, 1, 2] [102, 103, 104] [4, 5]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{3}, {6}, {8}});
  assert (lv2->size() == 8);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {0, 1, 2, 102, 103, 104, 4, 5});

  fac.copy (foo_id, avd2, 0, avd1, 1, 0);
  fac.copy (foo_id, avd2, 0, avd1, 1, 2);   // Making smaller
  // 1: [100, 101] [105, 106], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{2}, {4}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {100, 101, 105, 106, 3});
  // 2: [105, 106] [3] [4, 5]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {3}, {5}});
  assert (lv2->size() == 5);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {105, 106, 3, 4, 5});

  fac.copy (foo_id, avd2, 1, avd1, 0, 1);   // Making larger
  // 1: [100, 101] [105, 106], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{2}, {4}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {100, 101, 105, 106, 3});
  // 2: [105, 106] [100, 101] [4, 5]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {4}, {6}});
  assert (lv2->size() == 6);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {105, 106, 100, 101, 4, 5});

  fac.copy (foo_id, avd2, 2, avd1, 1, 1);   // Same size
  // 1: [100, 101] [105, 106], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{2}, {4}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {100, 101, 105, 106, 3});
  // 2: [105, 106] [100, 101] [105, 106]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {4}, {6}});
  assert (lv2->size() == 6);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {105, 106, 100, 101, 105, 106});

  fac.copy (foo_id, avd1, 0, avd1, 2, 1);   // Self-copy, making smaller
  // 1: [3] [105, 106], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{1}, {3}, {4}});
  assert (lv->size() == 4);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {3, 105, 106, 3});

  fac.copy (foo_id, avd1, 0, avd1, 1, 1);   // Self-copy, making larger
  // 1: [105, 106] [105, 106], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{2}, {4}, {5}});
  assert (lv->size() == 5);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {105, 106, 105, 106, 3});

  fac.copy (foo_id, avd2, 2, avd2, 1, 1);   // Self-copy, same size
  // 2: [105, 106] [100, 101] [100, 101]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {4}, {6}});
  assert (lv2->size() == 6);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {105, 106, 100, 101, 100, 101});

  // Overlapping copies not allowed.
  EXPECT_EXCEPTION (SG::ExcJaggedVecOverlappingCopy,
                    fac.copy (foo_id, avd2, 0, avd2, 1, 2));
  EXPECT_EXCEPTION (SG::ExcJaggedVecOverlappingCopy,
                    fac.copy (foo_id, avd2, 1, avd2, 0, 2));
  

  fac.clear (foo_id, avd2, 1, 0);
  fac.clear (foo_id, avd2, 1, 1);
  // 2: [105, 106] [] [100, 101]
  assert (v2->size() == 10);
  compareElts (ptr2, 10, {{2}, {2}, {4}});
  assert (lv2->size() == 4);
  lptr2 = reinterpret_cast<T*> (lv2->toPtr());
  comparePayload (lptr2, {105, 106, 100, 101});

  fac.clear (foo_id, avd1, 0, 2);
  // 1: [] [], [3]
  assert (v->size() == 10);
  compareElts (ptr, 10, {{0}, {0}, {1}});
  assert (lv->size() == 1);
  lptr = reinterpret_cast<T*> (lv->toPtr());
  comparePayload (lptr, {3});

  vector_type* vec3 = new vector_type;
  vec3->push_back (Elt (3));
  vec3->push_back (Elt (5));
  SG::AuxTypeVector<T, ALLOC<T> > lv3 (foo_payload_id, 5, 5, true);
  T* lptr3 = reinterpret_cast<T*> (lv3.toPtr());
  copyRange (std::vector<int>{51, 52, 53, 54, 55}, lptr3);
  std::unique_ptr<SG::IAuxTypeVector> v3 = fac.createFromData
    (foo_id, vec3, &lv3, false, true, false);

  assert (v3->auxid() == foo_id);
  assert (v3->size() == 2);
  assert (!v3->isLinked());
  Elt* ptr3 = reinterpret_cast<Elt*> (v3->toPtr());
  assert (ptr3[0] == Elt (3));
  assert (ptr3[1] == Elt (5));
}


void test1()
{
  std::cout << "test1\n";
  test_vector<float>  ("tfloat");
  test_vector<int>    ("tint");
  test_vector<double> ("tdouble");

  test_vector<float,  Athena_test::TestAlloc> ("tfloata");
  test_vector<int,    Athena_test::TestAlloc> ("tinta");
  test_vector<double, Athena_test::TestAlloc> ("tdoublea");

  using Elt = SG::JaggedVecElt<int>;
  {
    SG::JaggedVecVectorFactory<int> fac1;
    assert (fac1.tiAlloc() == &typeid(std::allocator<Elt>));
    assert (fac1.tiAllocName() == "std::allocator<SG::JaggedVecElt<int> >");
  }
  {
    SG::JaggedVecVectorFactory<int, Athena_test::TestAlloc<Elt> > fac2;
    assert (fac2.tiAlloc() == &typeid(Athena_test::TestAlloc<Elt>));
    assert (fac2.tiAllocName() == "Athena_test::TestAlloc<SG::JaggedVecElt<int> >");
  }

}


// testing copyForOutput.
void test2()
{
  std::cout << "test2\n";

#ifndef XAOD_STANDALONE
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();

  using EL = ElementLink<std::vector<int*> >;
  using Elt = SG::JaggedVecElt<EL>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t el_payload_id = r.getAuxID<EL>
    ("el_payload", "", SG::AuxVarFlags::Linked);
  SG::auxid_t el_id = r.getAuxID<Elt>
    ("el", "",
     SG::AuxVarFlags::None,
     el_payload_id);

  SG::JaggedVecVectorFactory<EL> fac;

  std::unique_ptr<SG::IAuxTypeVector> v1 = fac.create (el_id, 10, 10, false);
  std::unique_ptr<SG::IAuxTypeVector> lvup1 = v1->linkedVector();
  SG::IAuxTypeVector* lv1 = lvup1.get();
  Elt* ptr1 = reinterpret_cast<Elt*> (v1->toPtr());

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  store1.addVector (std::move(v1), false);
  store1.addVector (std::move(lvup1), false);

  // [(123,5) (123,6) (123,7)] [(124,8) (124,9)] [(125,10)]
  copyRange (std::vector<Elt>{{3}, {5}, {6}}, ptr1);
  std::fill_n (ptr1+3, 7, Elt{6});
  lv1->resize(6);
  EL* lptr1 = reinterpret_cast<EL*> (lv1->toPtr());
  copyRange (std::vector<EL>{{123,5}, {123,6}, {123,7},
                             {124,8}, {124,9}, {125,10}}, lptr1);

  std::unique_ptr<SG::IAuxTypeVector> v2 = fac.create (el_id, 10, 10, false);
  std::unique_ptr<SG::IAuxTypeVector> lvup2 = v2->linkedVector();
  [[maybe_unused]] SG::IAuxTypeVector* lv2 = lvup2.get();
  Elt* ptr2 = reinterpret_cast<Elt*> (v2->toPtr());

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  store2.addVector (std::move(v2), false);
  store2.addVector (std::move(lvup2), false);

  fac.copyForOutput (el_id, avd2, 0, avd1, 1, 2);
  // [(124,8) (124,9)] [(125,10)]
  compareElts (ptr2, 10, {{2}, {3}});
  assert (lv2->size() == 3);
  EL* lptr2 = reinterpret_cast<EL*> (lv2->toPtr());
  comparePayload (lptr2, {{124,8}, {124,9}, {125,10}});

  store->remap (123, 456, 6, 60);
  store->remap (124, 457, 9, 90);

  fac.copyForOutput (el_id, avd2, 1, avd1, 0, 3);
  // [(124,8) (124,9)] [(123,5) (456,60) (123,7)] [(124,8) (457,90)] [(125,10)]
  compareElts (ptr2, 10, {{2}, {5}, {7}, {8}});
  assert (lv2->size() == 8);
  lptr2 = reinterpret_cast<EL*> (lv2->toPtr());
  comparePayload (lptr2, {{124,8}, {124,9}, {123,5}, {456,60}, {123,7},
                          {124,8}, {457,90}, {125,10}});
#endif
}


int main()
{
  std::cout << "AthContainers/JaggedVecVectorFactory_test\n";
  test1();
  test2();
  return 0;
}
