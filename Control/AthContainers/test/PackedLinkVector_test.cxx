/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/PackedLinkVector_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Regression tests for PackedLinkVector and related types.
 */


#undef NDEBUG

#include "AthContainers/tools/PackedLinkVector.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthLinks/DataLink.h"
#include <iostream>
#include <cassert>


#ifdef XAOD_STANDALONE

// Declared in DataLink.h but not defined.
template <typename STORABLE>
bool operator== (const DataLink<STORABLE>& a,
                 const DataLink<STORABLE>& b)
{
  return a.key() == b.key() && a.source() == b.source();
}

#else

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( std::vector<int>, 12345, 0 )

#endif


class AuxStoreInternalTest
  : public SG::AuxStoreInternal
{
public:
  using SG::AuxStoreInternal::addVector;
};


// PackedLinkVectorHolder
void test1()
{
  std::cout << "test1\n";

  using Container = std::vector<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_links_id = r.getAuxID<DataLink<Container> >
    ("foo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<SG::PackedLink<Container> >
    ("foo", "",
     SG::AuxVarFlags::None,
     foo_links_id);

  std::vector<SG::PackedLink<Container> > v;

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (foo_links_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<DataLink<Container> >*>(linkedVec->toVector());

  SG::PackedLinkVectorHolder<Container> vh (foo_id, &v, linkedVec.get(), false);

  AuxStoreInternalTest store2;
  std::unique_ptr<SG::IAuxTypeVector> linkedVec2 = r.makeVector (foo_links_id, 0, 0);
  auto lv2 = reinterpret_cast<std::vector<DataLink<Container> >*>(linkedVec2->toVector());
  std::vector<SG::PackedLink<Container> > v2;
  auto vh2 = std::make_unique<SG::PackedLinkVectorHolder<Container> > (foo_id, &v2, linkedVec2.get(), false);
  store2.addVector (std::move (vh2), false);
  store2.addVector (std::move (linkedVec2), false);

  v2.emplace_back (1, 101);
  v2.emplace_back (1, 102);
  v2.emplace_back (2, 103);
  v2.emplace_back (0, 0);
  v2.emplace_back (1, 105);
  v2.emplace_back (3, 106);
  v2.emplace_back (4, 107);
  v2.emplace_back (1, 108);
  v2.emplace_back (0, 0);
  v2.emplace_back (2, 110);

  lv2->emplace_back ();
  lv2->emplace_back (123);
  lv2->emplace_back (124);
  lv2->emplace_back (125);
  lv2->emplace_back (126);

  assert (!vh.insertMove (0, v2.data()+2, v2.data()+5, store2));
  assert (v.size() == 3);
  assert (lv->size() == 3);
  assert (v[0] == SG::PackedLink<Container> (1, 103));
  assert (v[1] == SG::PackedLink<Container> (0,   0));
  assert (v[2] == SG::PackedLink<Container> (2, 105));
  assert (lv->at(0) == DataLink<Container>());
  assert (lv->at(1) == DataLink<Container>(124));
  assert (lv->at(2) == DataLink<Container>(123));

  vh.insertMove (2, v2.data()+6, v2.data()+9, store2);
  assert (v.size() == 6);
  assert (lv->size() == 4);
  assert (v[0] == SG::PackedLink<Container> (1, 103));
  assert (v[1] == SG::PackedLink<Container> (0,   0));
  assert (v[2] == SG::PackedLink<Container> (3, 107));
  assert (v[3] == SG::PackedLink<Container> (2, 108));
  assert (v[4] == SG::PackedLink<Container> (0,   0));
  assert (v[5] == SG::PackedLink<Container> (2, 105));
  assert (lv->at(0) == DataLink<Container>());
  assert (lv->at(1) == DataLink<Container>(124));
  assert (lv->at(2) == DataLink<Container>(123));
  assert (lv->at(3) == DataLink<Container>(126));
}


// PackedLinkVector
void test2()
{
  std::cout << "test2\n";

  using Container = std::vector<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_links_id = r.getAuxID<DataLink<Container> >
    ("foo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<SG::PackedLink<Container> >
    ("foo", "",
     SG::AuxVarFlags::None,
     foo_links_id);

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (foo_links_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<DataLink<Container> >*>(linkedVec->toVector());
  SG::PackedLinkVector<Container> pv (foo_id,
                                      0, 0, std::move (linkedVec));
  auto linkedVeca = pv.linkedVector();
  assert (linkedVeca->toVector() == lv);
  assert (!pv.linkedVector());

  SG::PackedLinkVector<Container> pv2 (pv);
  auto linkedVec2a = pv2.linkedVector();
  assert (linkedVec2a->toVector() != lv);

  SG::PackedLinkVector<Container> pv3 (std::move (pv2));

  auto pv4 = pv.clone();
  auto linkedVec4a = pv4->linkedVector();
  assert (linkedVec4a->toVector() != lv);
}


// PackedLinkVVectorHolder
void test3()
{
  std::cout << "test3\n";

  using Container = std::vector<int>;
  using PLink_t = SG::PackedLink<Container>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t vfoo_links_id = r.getAuxID<DataLink<Container> >
    ("vfoo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t vfoo_id = r.getAuxID<std::vector<PLink_t> >
    ("vfoo", "",
     SG::AuxVarFlags::None,
     vfoo_links_id);

  std::vector<std::vector<PLink_t> > v;

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (vfoo_links_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<DataLink<Container> >*>(linkedVec->toVector());

  SG::PackedLinkVVectorHolder<Container> vh (vfoo_id, &v, linkedVec.get(), false);

  AuxStoreInternalTest store2;
  std::unique_ptr<SG::IAuxTypeVector> linkedVec2 = r.makeVector (vfoo_links_id, 0, 0);
  auto lv2 = reinterpret_cast<std::vector<DataLink<Container> >*>(linkedVec2->toVector());
  std::vector<std::vector<PLink_t> > v2;
  auto vh2 = std::make_unique<SG::PackedLinkVVectorHolder<Container> > (vfoo_id, &v2, linkedVec2.get(), false);
  store2.addVector (std::move (vh2), false);
  store2.addVector (std::move (linkedVec2), false);

  v2.emplace_back (std::vector<PLink_t>{{1, 101}, {1, 102}, {2, 103}});
  v2.emplace_back (std::vector<PLink_t>{{0,   0}, {1, 105}, {3, 106}});
  v2.emplace_back (std::vector<PLink_t>{{4, 107}, {1, 108}, {0,   0}});
  v2.emplace_back (std::vector<PLink_t>{{2, 110}});

  lv2->emplace_back ();
  lv2->emplace_back (123);
  lv2->emplace_back (124);
  lv2->emplace_back (125);
  lv2->emplace_back (126);

  assert (!vh.insertMove (0, v2.data()+1, v2.data()+3, store2));
  assert (v.size() == 2);
  assert (lv->size() == 4);
  assert (v[0] == (std::vector<PLink_t>{{0,   0}, {1, 105}, {2, 106}}));
  assert (v[1] == (std::vector<PLink_t>{{3, 107}, {1, 108}, {0,   0}}));
  assert (v2[1].empty());
  assert (v2[2].empty());
  assert (lv->at(0) == DataLink<Container>());
  assert (lv->at(1) == DataLink<Container>(123));
  assert (lv->at(2) == DataLink<Container>(125));
  assert (lv->at(3) == DataLink<Container>(126));

  vh.insertMove (1, v2.data()+3, v2.data()+4, store2);
  assert (v.size() == 3);
  assert (lv->size() == 5);
  assert (v[0] == (std::vector<PLink_t>{{0,   0}, {1, 105}, {2, 106}}));
  assert (v[1] == (std::vector<PLink_t>{{4, 110}}));
  assert (v[2] == (std::vector<PLink_t>{{3, 107}, {1, 108}, {0,   0}}));
  assert (v2[0].size() == 3);
  assert (v2[1].empty());
  assert (v2[2].empty());
  assert (v2[3].empty());
  assert (lv->at(0) == DataLink<Container>());
  assert (lv->at(1) == DataLink<Container>(123));
  assert (lv->at(2) == DataLink<Container>(125));
  assert (lv->at(3) == DataLink<Container>(126));
  assert (lv->at(4) == DataLink<Container>(124));
}


// PackedLinkVVector
void test4()
{
  std::cout << "test4\n";

  using Container = std::vector<int>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t vfoo_links_id = r.getAuxID<DataLink<Container> >
    ("vfoo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t vfoo_id = r.getAuxID<std::vector<SG::PackedLink<Container> > >
    ("vfoo", "",
     SG::AuxVarFlags::None,
     vfoo_links_id);

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (vfoo_links_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<DataLink<Container> >*>(linkedVec->toVector());
  SG::PackedLinkVVector<Container> pv (vfoo_id,
                                       0, 0, std::move (linkedVec));
  auto linkedVeca = pv.linkedVector();
  assert (linkedVeca->toVector() == lv);
  assert (!pv.linkedVector());

  SG::PackedLinkVVector<Container> pv2 (pv);
  auto linkedVec2a = pv2.linkedVector();
  assert (linkedVec2a->toVector() != lv);

  SG::PackedLinkVVector<Container> pv3 (std::move (pv2));

  auto pv4 = pv.clone();
  auto linkedVec4a = pv4->linkedVector();
  assert (linkedVec4a->toVector() != lv);
}


int main()
{
  std::cout << "AthContainers/PackedLinkVector_test\n";
  test1();
  test2();
  test3();
  test4();
  return 0;
}
