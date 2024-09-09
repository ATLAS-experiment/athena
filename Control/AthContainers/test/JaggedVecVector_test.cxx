/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecVector_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Regression tests for JaggedVecVector and related types.
 */


#undef NDEBUG

#include "AthContainers/tools/JaggedVecVector.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/AuxStoreInternal.h"
#include "CxxUtils/ranges.h"
#include <iostream>
#include <cassert>


class AuxStoreInternalTest
  : public SG::AuxStoreInternal
{
public:
  using SG::AuxStoreInternal::addVector;
};


template <class T>
T makeT1(int x, T*) { return T(x); }

bool makeT1(int x, bool) { return (x&1) != 0; }

template <class T>
T makeT(int x=0) { return makeT1(x, static_cast<T*>(nullptr)); }

template <class T>
std::vector<T> makeTVec (const std::vector<int>& v)
{
#if HAVE_STD_RANGES
  return CxxUtils::to<std::vector<T> >(v | std::views::transform (makeT<T>));
#else
  std::vector<T> out;
  for (int x : v) {
    out.push_back (makeT<T> (x));
  }
  return out;
#endif
}


// JaggedVecVectorHolder
template <class T>
void test1a (const std::string& name)
{
  using Elt = SG::JaggedVecElt<T>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_payload_id = r.getAuxID<T> (name + "_payload", "",
                                              SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<Elt>
    (name, "",
     SG::AuxVarFlags::None,
     foo_payload_id);

  std::vector<Elt> v;

  std::unique_ptr<SG::IAuxTypeVector> payloadVec = r.makeVector (foo_payload_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<T>*>(payloadVec->toVector());

  SG::JaggedVecVectorHolder<T> vh (foo_id, &v, payloadVec.get(), false);

  v.emplace_back (3); // 3
  v.emplace_back (5); // 2
  v.emplace_back (5); // 0
  v.emplace_back (7); // 2
  v.emplace_back (9); // 2
  v.emplace_back (10); // 1

  for (size_t i = 0; i < 10; i++) {
    lv->push_back (makeT<T> (i));
  }

  assert (payloadVec->size() == 10);

  vh.reserve (10);
  assert (v.capacity() == 10);

  Elt* ptr1 = v.data();
  assert (vh.resize (8) == true);
  assert (ptr1 == v.data());
  assert (v.size() == 8);
  assert (v[5] == Elt (10));
  assert (v[6] == Elt (10));
  assert (v[7] == Elt (10));

  assert (vh.resize (5));
  assert (v.size() == 5);
  assert (v.back() == Elt (9));
  assert (payloadVec->size() == 9);

  vh.shift (4, 1);
  assert (v.size() == 6);
  assert (payloadVec->size() == 9);
  assert (v[0] == Elt (3));
  assert (v[1] == Elt (5));
  assert (v[2] == Elt (5));
  assert (v[3] == Elt (7));
  assert (v[4] == Elt (7));
  assert (v[5] == Elt (9));

  vh.shift (2, -1);
  assert (v.size() == 5);
  assert (payloadVec->size() == 7);
  assert (v[0] == Elt (3));  // [0, 1, 2]
  assert (v[1] == Elt (3));  // []
  assert (v[2] == Elt (5));  // [5, 6]
  assert (v[3] == Elt (5));  // []
  assert (v[4] == Elt (7));  // [7, 8]
  assert (*lv == makeTVec<T> ({0, 1, 2, 5, 6, 7, 8}));

  AuxStoreInternalTest store2;
  std::unique_ptr<SG::IAuxTypeVector> payloadVec2 = r.makeVector (foo_payload_id, 0, 0);
  auto lv2 = reinterpret_cast<std::vector<T>*>(payloadVec2->toVector());
  std::vector<Elt> v2;
  auto vh2 = std::make_unique<SG::JaggedVecVectorHolder<T> > (foo_id, &v2, payloadVec2.get(), false);
  store2.addVector (std::move (vh2), false);
  store2.addVector (std::move (payloadVec2), false);

  v2.emplace_back (2); // 2
  v2.emplace_back (3); // 1
  v2.emplace_back (5); // 2
  for (size_t i = 0; i < 5; i++) {
    lv2->push_back (makeT<T> (i+100));
  }

  vh.insertMove (2, v2.data(), 1, 2, store2);
  assert (v.size() == 7);
  assert (lv->size() == 10);
  assert (v[0] == Elt (3));  // [0, 1, 2]
  assert (v[1] == Elt (3));  // []
  assert (v[2] == Elt (4));  // [102]
  assert (v[3] == Elt (6));  // [103, 104]
  assert (v[4] == Elt (8));  // [5, 6]
  assert (v[5] == Elt (8));  // []
  assert (v[6] == Elt (10));  // [7, 8]
  assert (*lv == makeTVec<T> ({0, 1, 2, 102, 103, 104, 5, 6, 7, 8}));

  vh.insertMove (0, v2.data(), 0, 0, store2);
  vh.insertMove (0, v2.data(), 0, 1, store2);
  assert (v.size() == 8);
  assert (lv->size() == 12);
  assert (v[0] == Elt (2));  // [100, 101]
  assert (v[1] == Elt (5));  // [0, 1, 2]
  assert (v[2] == Elt (5));  // []
  assert (v[3] == Elt (6));  // [102]
  assert (v[4] == Elt (8));  // [103, 104]
  assert (v[5] == Elt (10));  // [5, 6]
  assert (v[6] == Elt (10));  // []
  assert (v[7] == Elt (12));  // [7, 8]
  assert (lv->at(0) == 100);
  assert (lv->at(1) == 101);
  assert (*lv == makeTVec<T>({100, 101, 0, 1, 2, 102, 103, 104, 5, 6, 7, 8}));
}
void test1()
{
  std::cout << "test1\n";
  test1a<float> ("tfloat");
  test1a<int> ("tint");
  test1a<double> ("tdouble");
}


// PackedLinkVector
template <class T>
void test2a (const std::string& name)
{
  using Elt = SG::JaggedVecElt<T>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_payload_id = r.getAuxID<T> (name + "_payload", "",
                                              SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<Elt>
    (name, "",
     SG::AuxVarFlags::None,
     foo_payload_id);

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (foo_payload_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<T>*>(linkedVec->toVector());
  SG::JaggedVecVector<T> pv (foo_id, 0, 0, std::move (linkedVec));
  auto linkedVeca = pv.linkedVector();
  assert (linkedVeca->toVector() == lv);
  assert (!pv.linkedVector());

  SG::JaggedVecVector<T> pv2 (pv);
  auto linkedVec2a = pv2.linkedVector();
  assert (linkedVec2a->toVector() != lv);

  SG::JaggedVecVector<T> pv3 (std::move (pv2));

  auto pv4 = pv.clone();
  auto linkedVec4a = pv4->linkedVector();
  assert (linkedVec4a->toVector() != lv);
}
void test2()
{
  std::cout << "test2\n";
  test2a<float> ("tfloat");
}
  


int main()
{
  std::cout << "AthContainers/JaggedVecVector_test\n";
  test1();
  test2();
  return 0;
}
