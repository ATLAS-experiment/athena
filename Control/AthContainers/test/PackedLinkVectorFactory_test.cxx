/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/PackedLinkVectorFactory_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Regression tests for PackedLinkVectorFactory.
 */


#undef NDEBUG


#include "AthContainers/tools/PackedLinkVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "TestTools/TestAlloc.h"
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

#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( std::vector<int>, 12345, 0 )

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


template <template<typename> class ALLOC = std::allocator>
void test_vector()
{
  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using DLink = DataLink<Cont>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_links_id = r.getAuxID<DLink>
    ("foo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<PLink>
    ("foo", "",
     SG::AuxVarFlags::None,
     foo_links_id);

  SG::PackedLinkVectorFactory<Cont, ALLOC<PLink> > fac;
  assert (fac.getEltSize() == sizeof(PLink));
  assert (!fac.isDynamic());
  assert (fac.tiVec() == &typeid (std::vector<PLink, ALLOC<PLink> >));

  std::unique_ptr<SG::IAuxTypeVector> vup = fac.create (foo_id, 10, 20, false);
  SG::IAuxTypeVector* v = vup.get();
  assert (v->auxid() == foo_id);
  assert (!v->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> lvup = v->linkedVector();
  SG::IAuxTypeVector* lv = lvup.get();
  assert (lv->auxid() == foo_links_id);
  assert (lv->isLinked());
  lv->resize(3);

  PLink* ptr = reinterpret_cast<PLink*> (v->toPtr());
  DLink* lptr = reinterpret_cast<DLink*> (lv->toPtr());

  std::unique_ptr<SG::IAuxTypeVector> vup2 = fac.create (foo_id, 10, 20, false);
  SG::IAuxTypeVector* v2 = vup2.get();
  assert (v2->auxid() == foo_id);
  assert (!v2->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> lvup2 = v2->linkedVector();
  SG::IAuxTypeVector* lv2 = lvup2.get();
  assert (lv2->auxid() == foo_links_id);
  assert (lv2->isLinked());
  lv2->resize(3);

  PLink* ptr2 = reinterpret_cast<PLink*> (v2->toPtr());
  DLink* lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());

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

  ptr[0] = PLink (1, 1); // 101
  ptr[1] = PLink (2, 2); // 102
  ptr[2] = PLink (1, 3); // 101
  lptr[1] = DLink (101);
  lptr[2] = DLink (102);

  ptr2[0] = PLink (1, 10); // 102
  ptr2[1] = PLink (2, 11); // 103
  ptr2[2] = PLink (1, 12); // 102
  lptr2[1] = DLink (102);
  lptr2[2] = DLink (103);

  fac.swap (foo_id, avd2, 0, avd1, 1, 2);
  assert (ptr[0] == PLink (1, 1));  // 101
  assert (ptr[1] == PLink (2, 10)); // 102
  assert (ptr[2] == PLink (3, 11)); // 103
  assert (ptr2[0] == PLink (1, 2)); // 102
  assert (ptr2[1] == PLink (3, 3)); // 101
  assert (ptr2[2] == PLink (1, 12)); // 102

  assert (lv->size() == 4);
  assert (lv2->size() == 4);
  lptr = reinterpret_cast<DLink*> (lv->toPtr());
  lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());
  assert (lptr[0] == DLink());
  assert (lptr[1] == DLink (101));
  assert (lptr[2] == DLink (102));
  assert (lptr[3] == DLink (103));
  assert (lptr2[0] == DLink());
  assert (lptr2[1] == DLink (102));
  assert (lptr2[2] == DLink (103));
  assert (lptr2[3] == DLink (101));

  avd1.setStore (static_cast<SG::IAuxStore*>(nullptr));
  avd1.setStore (static_cast<SG::IConstAuxStore*>(&store1));
  fac.copy (foo_id, avd2, 0, avd1, 1, 1);
  fac.copy (foo_id, avd2, 1, avd1, 0, 1);
  assert (ptr2[0] == PLink (1, 10)); // 102
  assert (ptr2[1] == PLink (3, 1)); // 101
  avd1.setStore (&store1);

  fac.clear (foo_id, avd2, 0, 2);
  assert (ptr2[0] == PLink (0, 0));
  assert (ptr2[1] == PLink (0, 0));
  assert (ptr2[2] == PLink (1, 12)); // 102
  assert (lv2->size() == 4);

  using vector_type = std::vector<PLink, ALLOC<PLink> >;
  vector_type* vec3 = new vector_type;
  vec3->push_back (PLink (1, 21)); // 111
  vec3->push_back (PLink (0, 0));
  vec3->push_back (PLink (2, 22)); // 112
  SG::AuxTypeVector<DLink, ALLOC<DLink> > lv3 (foo_links_id, 3, 3, true);
  auto lptr3 = reinterpret_cast<DLink*> (lv3.toPtr());
  lptr3[1] = DLink (111);
  lptr3[2] = DLink (112);

  std::unique_ptr<SG::IAuxTypeVector> v3 = fac.createFromData
    (foo_id, vec3, &lv3, false, true, false);
  assert (v3->auxid() == foo_id);
  assert (v3->size() == 3);
  assert (!v3->isLinked());
  PLink* ptr3 = reinterpret_cast<PLink*> (v3->toPtr());
  assert (ptr3[0] == PLink (1, 21)); // 111
  assert (ptr3[1] == PLink (0, 0));
  assert (ptr3[2] == PLink (2, 22)); // 112

  // Testing range copy, with and without overlap.
  ptr[0] = PLink(0, 0);
  ptr2[0] = PLink(0, 0);
  for (size_t i = 1; i < 10; i++) {
    ptr[i] = PLink(((i-1)%3)+1, i);
    ptr2[i] = PLink(((i-1)%3)+1, i+10);
  }
  lptr[1] = DLink (131);
  lptr[2] = DLink (132);
  lptr[3] = DLink (133);
  lptr2[1] = DLink (132);
  lptr2[2] = DLink (131);
  lptr2[3] = DLink (134);

  // 0,0 131,1  132,2  133,3  131,4  132,5  133,6  131,7  132,8  133,9
  // 0,0 132,11 131,12 134,13 132,14 131,15 134,16 132,17 131,18 134,19

  auto checkvec = [] (const PLink* pl,
                      const DLink* dl,
                      const std::vector<std::pair<unsigned, unsigned> >& exp)
    {
      for (size_t i = 0; i < exp.size(); i++) {
        assert (pl[i].index() == exp[i].second);
        assert (dl[pl[i].collection()].key() == exp[i].first);
      }
    };

  fac.copy (foo_id, avd1, 3, avd1, 4, 0);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {131,1}, {132,2}, {133,3}, {131,4},
             {132,5}, {133,6}, {131,7}, {132,8}, {133,9}});

  fac.copy (foo_id, avd1, 1, avd1, 2, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {133,3}, {131,4}, {131,4},
             {132,5}, {133,6}, {131,7}, {132,8}, {133,9}});

  fac.copy (foo_id, avd1, 6, avd1, 5, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {133,3}, {131,4}, {131,4},
             {132,5}, {132,5}, {133,6}, {131,7}, {133,9}});

  fac.copy (foo_id, avd1, 2, avd1, 5, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {132,5}, {132,5}, {133,6},
             {132,5}, {132,5}, {133,6}, {131,7}, {133,9}});

  fac.copy (foo_id, avd2, 2, avd1, 6, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {132,5}, {132,5}, {133,6},
             {132,5}, {132,5}, {133,6}, {131,7}, {133,9}});
  assert (lv2->size() == 5);
  lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());
  assert (lptr2[4].key() == 133);
  checkvec (ptr2, lptr2,
            {{0,0},    {132,11}, {132,5},  {133,6},  {131,7},
             {131,15}, {134,16}, {132,17}, {131,18}, {134,19}});

  AuxVectorData_test avd3;
  AuxStoreInternal_test store3;
  avd3.setStore (&store3);

  fac.copy (foo_id, avd1, 3, avd3, 3, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0}, {132,2}, {132,5}, {0,0},   {0,0},
             {0,0}, {132,5}, {133,6}, {131,7}, {133,9}});
}


void test1()
{
  std::cout << "test1\n";
  test_vector();
  test_vector<Athena_test::TestAlloc>();

  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  {
    SG::PackedLinkVectorFactory<Cont> fac1;
    assert (fac1.tiAlloc() == &typeid(std::allocator<PLink>));
    assert (fac1.tiAllocName() == "std::allocator<SG::PackedLink<std::vector<int> > >");
  }
  {
    SG::PackedLinkVectorFactory<Cont, Athena_test::TestAlloc<PLink> > fac2;
    assert (fac2.tiAlloc() == &typeid(Athena_test::TestAlloc<PLink>));
    assert (fac2.tiAllocName() == "Athena_test::TestAlloc<SG::PackedLink<std::vector<int> > >");
  }
}


template <template<typename> class ALLOC = std::allocator>
void test_vvector()
{
  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using DLink = DataLink<Cont>;
  using VElt = std::vector<PLink>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t vfoo_links_id = r.getAuxID<DLink>
    ("vfoo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t vfoo_id = r.getAuxID<std::vector<PLink> >
    ("vfoo", "",
     SG::AuxVarFlags::None,
     vfoo_links_id);

  SG::AuxTypeVectorFactory<VElt, ALLOC<VElt> > fac;
  assert (fac.getEltSize() == sizeof(VElt));
  assert (!fac.isDynamic());
  assert (fac.tiVec() == &typeid (std::vector<VElt, ALLOC<VElt> >));

  std::unique_ptr<SG::IAuxTypeVector> vup = fac.create (vfoo_id, 10, 20, false);
  SG::IAuxTypeVector* v = vup.get();
  assert (v->auxid() == vfoo_id);
  assert (!v->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> lvup = v->linkedVector();
  SG::IAuxTypeVector* lv = lvup.get();
  assert (lv->auxid() == vfoo_links_id);
  assert (lv->isLinked());
  lv->resize(3);

  VElt* ptr = reinterpret_cast<VElt*> (v->toPtr());
  DLink* lptr = reinterpret_cast<DLink*> (lv->toPtr());

  std::unique_ptr<SG::IAuxTypeVector> vup2 = fac.create (vfoo_id, 10, 20, false);
  SG::IAuxTypeVector* v2 = vup2.get();
  assert (v2->auxid() == vfoo_id);
  assert (!v2->isLinked());

  std::unique_ptr<SG::IAuxTypeVector> lvup2 = v2->linkedVector();
  SG::IAuxTypeVector* lv2 = lvup2.get();
  assert (lv2->auxid() == vfoo_links_id);
  assert (lv2->isLinked());
  lv2->resize(3);

  VElt* ptr2 = reinterpret_cast<VElt*> (v2->toPtr());
  DLink* lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());

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

  ptr[0] = VElt{{1, 1}, {2, 2}, {1, 3}};  // 101, 102, 101
  ptr[1] = VElt{{2, 4}, {0, 0}, {1, 6}};  // 102,   0, 101
  ptr[2] = VElt{{1, 7}, {2, 8}, {0, 0}};  // 101, 102,   0
  lptr[1] = DLink (101);
  lptr[2] = DLink (102);

  ptr2[0] = VElt{{1, 11}, {2, 12}, {1, 13}};  // 102, 103, 102
  ptr2[1] = VElt{{0,  0}, {1, 15}, {2, 16}};  //   0, 102, 103
  ptr2[2] = VElt{{2, 17}, {1, 18}, {0,  0}};  // 103, 102,   0
  lptr2[1] = DLink (102);
  lptr2[2] = DLink (103);

  fac.swap (vfoo_id, avd2, 0, avd1, 1, 2);
  assert (ptr[0]  == (VElt{{1,  1}, {2,  2}, {1,  3}})); // 101, 102, 101
  assert (ptr[1]  == (VElt{{2, 11}, {3, 12}, {2, 13}})); // 102, 103, 102
  assert (ptr[2]  == (VElt{{0,  0}, {2, 15}, {3, 16}})); //   0, 102, 103
  assert (ptr2[0] == (VElt{{1,  4}, {0,  0}, {3,  6}})); // 102,   0, 101
  assert (ptr2[1] == (VElt{{3,  7}, {1,  8}, {0,  0}})); // 101, 102,   0
  assert (ptr2[2] == (VElt{{2, 17}, {1, 18}, {0,  0}})); // 103, 102,   0

  assert (lv->size() == 4);
  assert (lv2->size() == 4);
  lptr = reinterpret_cast<DLink*> (lv->toPtr());
  lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());
  assert (lptr[0] == DLink());
  assert (lptr[1] == DLink (101));
  assert (lptr[2] == DLink (102));
  assert (lptr[3] == DLink (103));
  assert (lptr2[0] == DLink());
  assert (lptr2[1] == DLink (102));
  assert (lptr2[2] == DLink (103));
  assert (lptr2[3] == DLink (101));

  avd1.setStore (static_cast<SG::IAuxStore*>(nullptr));
  avd1.setStore (static_cast<SG::IConstAuxStore*>(&store1));
  fac.copy (vfoo_id, avd2, 0, avd1, 1, 1);
  fac.copy (vfoo_id, avd2, 1, avd1, 0, 1);
  assert (ptr2[0] == (VElt{{1, 11}, {2, 12}, {1, 13}})); // 102, 103, 102
  assert (ptr2[1] == (VElt{{3,  1}, {1,  2}, {3,  3}})); // 101, 102, 101
  avd1.setStore (&store1);

  fac.clear (vfoo_id, avd2, 0, 2);
  assert (ptr2[0].empty());
  assert (ptr2[1].empty());
  assert (ptr2[2] == (VElt{{2, 17}, {1, 18}, {0,  0}})); // 103, 102,   0
  assert (lv2->size() == 4);

  using vector_type = std::vector<VElt, ALLOC<VElt> >;
  vector_type* vec3 = new vector_type;
  vec3->emplace_back (VElt {{1, 21}, {0,  0}, {2, 22}});  // 111,   0, 112
  vec3->emplace_back (VElt {{2, 23}, {1, 24}, {1, 25}});  // 112, 111, 111
  vec3->emplace_back (VElt {{0,  0}, {2, 27}, {1, 28}});  //   0, 112, 111
  SG::AuxTypeVector<DLink, ALLOC<DLink> > lv3 (vfoo_links_id, 3, 3, true);
  auto lptr3 = reinterpret_cast<DLink*> (lv3.toPtr());
  lptr3[1] = DLink (111);
  lptr3[2] = DLink (112);

  std::unique_ptr<SG::IAuxTypeVector> v3 = fac.createFromData
    (vfoo_id, vec3, &lv3, false, true, false);
  assert (v3->auxid() == vfoo_id);
  assert (v3->size() == 3);
  assert (!v3->isLinked());
  VElt* ptr3 = reinterpret_cast<VElt*> (v3->toPtr());
  assert (ptr3[0] == (VElt {{1, 21}, {0,  0}, {2, 22}}));  // 111,   0, 112
  assert (ptr3[1] == (VElt {{2, 23}, {1, 24}, {1, 25}}));  // 112, 111, 111
  assert (ptr3[2] == (VElt {{0,  0}, {2, 27}, {1, 28}}));  //   0, 112, 111

  // Testing range copy, with and without overlap.
  ptr[0] = VElt{{0, 0}};
  ptr2[0] = VElt{{0, 0}};
  for (unsigned i = 1; i < 10; i++) {
    ptr[i] = VElt{{((i-1)%3)+1, i}};
    ptr2[i] = VElt{{((i-1)%3)+1, i+10}};
  }
  lptr[1] = DLink (131);
  lptr[2] = DLink (132);
  lptr[3] = DLink (133);
  lptr2[1] = DLink (132);
  lptr2[2] = DLink (131);
  lptr2[3] = DLink (134);

  // 0,0 131,1  132,2  133,3  131,4  132,5  133,6  131,7  132,8  133,9
  // 0,0 132,11 131,12 134,13 132,14 131,15 134,16 132,17 131,18 134,19

  auto checkvec = [] (const VElt* pl,
                      const DLink* dl,
                      const std::vector<std::pair<unsigned, unsigned> >& exp)
    {
      for (size_t i = 0; i < exp.size(); i++) {
        if (exp[i].second == 999) {
          assert (pl[i].empty());
        }
        else {
          assert (pl[i].size() == 1);
          assert (pl[i][0].index() == exp[i].second);
          assert (dl[pl[i][0].collection()].key() == exp[i].first);
        }
      }
    };

  fac.copy (vfoo_id, avd1, 3, avd1, 4, 0);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {131,1}, {132,2}, {133,3}, {131,4},
             {132,5}, {133,6}, {131,7}, {132,8}, {133,9}});

  fac.copy (vfoo_id, avd1, 1, avd1, 2, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {133,3}, {131,4}, {131,4},
             {132,5}, {133,6}, {131,7}, {132,8}, {133,9}});

  fac.copy (vfoo_id, avd1, 6, avd1, 5, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {133,3}, {131,4}, {131,4},
             {132,5}, {132,5}, {133,6}, {131,7}, {133,9}});

  fac.copy (vfoo_id, avd1, 2, avd1, 5, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {132,5}, {132,5}, {133,6},
             {132,5}, {132,5}, {133,6}, {131,7}, {133,9}});

  fac.copy (vfoo_id, avd2, 2, avd1, 6, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0},   {132,2}, {132,5}, {132,5}, {133,6},
             {132,5}, {132,5}, {133,6}, {131,7}, {133,9}});
  assert (lv2->size() == 5);
  lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());
  assert (lptr2[4].key() == 133);
  checkvec (ptr2, lptr2,
            {{0,0},    {132,11}, {132,5},  {133,6},  {131,7},
             {131,15}, {134,16}, {132,17}, {131,18}, {134,19}});

  AuxVectorData_test avd3;
  AuxStoreInternal_test store3;
  avd3.setStore (&store3);

  fac.copy (vfoo_id, avd1, 3, avd3, 3, 3);
  assert (lv->size() == 4);
  checkvec (ptr, lptr,
            {{0,0}, {132,2}, {132,5}, {0,999},   {0,999},
             {0,999}, {132,5}, {133,6}, {131,7}, {133,9}});
}


void test2()
{
  std::cout << "test2\n";

  test_vvector();
  test_vvector<Athena_test::TestAlloc>();

  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using VElt = std::vector<PLink>;
  {
    SG::AuxTypeVectorFactory<VElt> fac1;
    assert (fac1.tiAlloc() == &typeid(std::allocator<VElt>));
    assert (fac1.tiAllocName() == "std::allocator<std::vector<SG::PackedLink<std::vector<int> > > >");
  }
  {
    SG::AuxTypeVectorFactory<VElt, Athena_test::TestAlloc<VElt> > fac2;
    assert (fac2.tiAlloc() == &typeid(Athena_test::TestAlloc<VElt>));
    assert (fac2.tiAllocName() == "Athena_test::TestAlloc<std::vector<SG::PackedLink<std::vector<int> > > >");
  }
}


// testing copyForOutput.
void test3()
{
  std::cout << "test3\n";

#ifndef XAOD_STANDALONE
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();

  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using DLink = DataLink<Cont>;
  using VElt = std::vector<PLink>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_links_id = r.getAuxID<DLink>
    ("foo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t foo_id = r.getAuxID<PLink>
    ("foo", "",
     SG::AuxVarFlags::None,
     foo_links_id);

  SG::auxid_t vfoo_links_id = r.getAuxID<DLink>
    ("vfoo_links", "",
       SG::AuxVarFlags::Linked);
  SG::auxid_t vfoo_id = r.getAuxID<VElt>
    ("vfoo", "",
     SG::AuxVarFlags::None,
     vfoo_links_id);

  SG::PackedLinkVectorFactory<Cont> ve1;
  SG::AuxTypeVectorFactory<VElt> ve2;

  std::unique_ptr<SG::IAuxTypeVector> v1 = ve1.create (foo_id, 10, 10, false);
  std::unique_ptr<SG::IAuxTypeVector> lvup1 = v1->linkedVector();
  SG::IAuxTypeVector* lv1 = lvup1.get();
  lv1->resize(3);

  PLink* ptr1 = reinterpret_cast<PLink*> (v1->toPtr());
  DLink* lptr1 = reinterpret_cast<DLink*> (lv1->toPtr());
  ptr1[1] = PLink (1, 10); // 123
  ptr1[2] = PLink (2, 11); // 124
  lptr1[1] = DLink (123);
  lptr1[2] = DLink (124);

  AuxVectorData_test avd1;
  AuxStoreInternal_test store1;
  avd1.setStore (&store1);
  store1.addVector (std::move(v1), false);
  store1.addVector (std::move(lvup1), false);

  ve1.copyForOutput (foo_id, avd1, 2, avd1, 1, 2);
  assert (lv1->size() == 3);
  assert (ptr1[1] == PLink (1, 10));
  assert (ptr1[2] == PLink (1, 10));
  assert (ptr1[3] == PLink (2, 11));

  std::unique_ptr<SG::IAuxTypeVector> v2 = ve2.create (vfoo_id, 10, 10, false);
  std::unique_ptr<SG::IAuxTypeVector> lvup2 = v2->linkedVector();
  SG::IAuxTypeVector* lv2 = lvup2.get();
  lv2->resize(3);

  VElt* velv = reinterpret_cast<VElt*> (v2->toPtr());
  DLink* lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());
  velv[1] = VElt{{1, 5}, {1, 6}};  // 123, 123
  velv[2] = VElt{{2, 7}, {2, 8}};  // 124, 124
  lptr2[1] = DLink (123);
  lptr2[2] = DLink (124);

  AuxVectorData_test avd2;
  AuxStoreInternal_test store2;
  avd2.setStore (&store2);
  store2.addVector (std::move(v2), false);
  store2.addVector (std::move(lvup2), false);

  ve2.copyForOutput (vfoo_id, avd2, 2, avd2, 1, 2);
  assert (velv[2] == (VElt{{1, 5}, {1, 6}}));  // 123, 123
  assert (velv[3] == (VElt{{2, 7}, {2, 8}}));  // 124, 124

  store->remap (123, 456, 10, 20);
  store->remap (124, 457, 11, 21);
  store->remap (123, 456, 6, 12);
  store->remap (124, 457, 8, 28);

  ve1.copyForOutput (1, avd1, 5, avd1, 2, 2);
  assert (lv1->size() == 5);
  lptr1 = reinterpret_cast<DLink*> (lv1->toPtr());
  assert (ptr1[1] == PLink (1, 10));
  assert (ptr1[2] == PLink (1, 10));
  assert (ptr1[3] == PLink (2, 11));
  assert (ptr1[5] == PLink (3, 20));
  assert (ptr1[6] == PLink (4, 21));
  assert (lptr1[1].key() == 123);
  assert (lptr1[2].key() == 124);
  assert (lptr1[3].key() == 456);
  assert (lptr1[4].key() == 457);

  ve2.copyForOutput (vfoo_id, avd2, 5, avd2, 2, 2);
  assert (lv2->size() == 5);
  lptr2 = reinterpret_cast<DLink*> (lv2->toPtr());
  assert (velv[5] == (VElt{{1, 5}, {3, 12}}));  // 123, 456
  assert (velv[6] == (VElt{{2, 7}, {4, 28}}));  // 124, 457
  assert (lptr2[1].key() == 123);
  assert (lptr2[2].key() == 124);
  assert (lptr2[3].key() == 456);
  assert (lptr2[4].key() == 457);
#endif
}


int main()
{
  std::cout << "AthContainers/PackedLinkVectorFactory_test\n";
  test1();
  test2();
  test3();
  return 0;
}
