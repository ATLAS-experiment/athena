/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/PackedLinkVectorHeper_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Regression tests for PackedLinkVectorHelper.
 */


#undef NDEBUG

#include "AthContainers/tools/PackedLinkVectorHelper.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/PackedLinkImpl.h"
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

#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( std::vector<int>, 12345, 0 )

#endif


// PackedLinkVectorHelper
void test1()
{
  std::cout << "test1\n";

  using Container = std::vector<int>;
  using DLink_t = DataLink<Container>;
  using PLink_t = SG::PackedLink<Container>;
  using Helper = SG::detail::PackedLinkVectorHelper<Container>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_links_id = r.getAuxID<DataLink<Container> >
    ("foo_links", "",
       SG::AuxVarFlags::Linked);

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (foo_links_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<DLink_t>*>(linkedVec->toVector());

  lv->emplace_back (0);
  lv->emplace_back (10);
  lv->emplace_back (12);
  lv->emplace_back (11);

  {
    auto span = Helper::getLinkBaseSpan (*linkedVec);
    assert (span.size() == 4);
    assert (span[0] == DLink_t());
    assert (span[3] == DLink_t (11));
  }

  std::unique_ptr<SG::IAuxTypeVector> linkedVec2 = r.makeVector (foo_links_id, 0, 0);
  [[maybe_unused]]
  auto lv2 = reinterpret_cast<std::vector<DLink_t>*>(linkedVec2->toVector());
  [[maybe_unused]]
  auto links2 = Helper::getLinkBaseSpan (*linkedVec2);
  Helper::LinkedVector linkedVector2 (*linkedVec2);
  {
    auto [ndx, flag] = Helper::findCollection (linkedVector2, 10, links2, nullptr);
    assert (ndx == 1);
    assert (!flag);
    assert (lv2->size() == 2);
    assert (links2.size() == 2);
    assert (links2[0] == DLink_t());
    assert (lv2->at(1) == DLink_t(10));
  }
  {
    auto [ndx, flag] = Helper::findCollection (linkedVector2, 12, links2, nullptr);
    assert (ndx == 2);
    assert (lv2->size() == 3);
    assert (links2.size() == 3);
    assert (links2[0] == DLink_t());
    assert (lv2->at(2) == DLink_t(12));
  }
  {
    auto [ndx, flag] = Helper::findCollection (linkedVector2, 10, links2, nullptr);
    assert (ndx == 1);
    assert (flag);
    assert (lv2->size() == 3);
    assert (links2.size() == 3);
  }
  {
    auto [ndx, flag] = Helper::findCollection (linkedVector2, 0, links2, nullptr);
    assert (ndx == 0);
    assert (flag);
    assert (lv2->size() == 3);
    assert (links2.size() == 3);
  }

  std::unique_ptr<SG::IAuxTypeVector> linkedVec3 = r.makeVector (foo_links_id, 0, 0);
  auto lv3 = reinterpret_cast<std::vector<DLink_t>*>(linkedVec3->toVector());
  auto links3 = Helper::getLinkBaseSpan (*linkedVec3);
  Helper::LinkedVector linkedVector3 (*linkedVec3);
  {
    auto [ndx, flag] = Helper::findCollection (linkedVector3, 0, links3, nullptr);
    assert (ndx == 0);
    assert (!flag);
    assert (lv3->size() == 1);
    assert (links3.size() == 1);
    assert (links3[0] == DLink_t());
  }

  {
    DLink_t dlinks[] = {DLink_t(), DLink_t(11), DLink_t(12), DLink_t(13)};
    PLink_t links[] = {{1, 101}, {3, 102}, {0, 0}, {2, 103}};
    SG::AuxDataSpanBase dlink_spanBase { dlinks, std::size (dlinks) };
    bool flag [[maybe_unused]] = Helper::updateLinks (*linkedVec,
                                                      links,
                                                      std::size(links),
                                                      dlink_spanBase,
                                                      nullptr);
    assert (links[0] == PLink_t(3, 101));
    assert (links[1] == PLink_t(4, 102));
    assert (links[2] == PLink_t(0,   0));
    assert (links[3] == PLink_t(2, 103));
    assert (lv->size() == 5);
    assert (lv->at(4) == DLink_t(13));
  }

  {
    std::unique_ptr<SG::IAuxTypeVector> linkedVecx = r.makeVector (foo_links_id, 4, 4);
    auto lvx = reinterpret_cast<std::vector<DLink_t>*>(linkedVecx->toVector());
    lvx->at(1) = DLink_t(11);
    lvx->at(2) = DLink_t(12);
    lvx->at(3) = DLink_t(13);

    PLink_t links[] = {{1, 101}, {3, 102}, {0, 0}, {2, 103}};
    bool flag [[maybe_unused]] = Helper::updateLinks (*linkedVec,
                                                      links, 4,
                                                      *linkedVecx,
                                                      nullptr);
    assert (links[0] == PLink_t(3, 101));
    assert (links[1] == PLink_t(4, 102));
    assert (links[2] == PLink_t(0,   0));
    assert (links[3] == PLink_t(2, 103));
    assert (lv->size() == 5);
    assert (lv->at(4) == DLink_t(13));
  }

  {
    using VElt = std::vector<PLink_t>;
    std::unique_ptr<SG::IAuxTypeVector> linkedVecx = r.makeVector (foo_links_id, 4, 4);
    auto lvx = reinterpret_cast<std::vector<DLink_t>*>(linkedVecx->toVector());
    lvx->at(1) = DLink_t(11);
    lvx->at(2) = DLink_t(12);
    lvx->at(3) = DLink_t(13);

    VElt links[3];
    links[0] = VElt{{1, 101}, {2, 102}, {3, 103}};
    links[1] = VElt{{2, 104}, {0,   0}, {1, 106}};
    links[2] = VElt{{0,   0}, {3, 108}, {1, 109}};
    bool flag [[maybe_unused]] = Helper::updateLinks (*linkedVec,
                                                      links, 3,
                                                      *linkedVecx,
                                                      nullptr);
    assert (links[0] == (VElt{{3, 101}, {2, 102}, {4, 103}}));
    assert (links[1] == (VElt{{2, 104}, {0,   0}, {3, 106}}));
    assert (links[2] == (VElt{{0,   0}, {4, 108}, {3, 109}}));
    assert (lv->size() == 5);
  }

  {
    using VElt = std::vector<PLink_t>;
    std::unique_ptr<SG::IAuxTypeVector> linkedVecx = r.makeVector (foo_links_id, 2, 2);
    auto lvx = reinterpret_cast<std::vector<DLink_t>*>(linkedVecx->toVector());
    lvx->at(1) = DLink_t(11);

    VElt velt;

    auto dlinks = Helper::getLinkBaseSpan (*linkedVecx);
    std::vector<ElementLink<Container> > elv {{11, 1}, {}, {12, 2}};
    bool flag [[maybe_unused]] = Helper::assignVElt (velt,
                                                     *linkedVecx,
                                                     dlinks,
                                                     elv);
    assert (lvx->size() == 3);
    lvx = reinterpret_cast<std::vector<DLink_t>*>(linkedVecx->toVector());
    assert (lvx->at(1) == DLink_t(11));
    assert (lvx->at(2) == DLink_t(12));
    assert (velt == (VElt {{1, 1}, {0, 0}, {2, 2}}));

    std::vector<ElementLink<Container> > elv2 {{12, 3}, {11, 4}};
    flag = Helper::insertVElt (velt,
                               1,
                               *linkedVecx,
                               dlinks,
                               elv2);
    assert (lvx->size() == 3);
    assert (velt == (VElt {{1, 1}, {2, 3}, {1, 4}, {0, 0}, {2, 2}}));
  }
}


void test_applyThinning()
{
  std::cout << "test_applyThinning\n";

#ifndef XAOD_STANDALONE
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();

  using Container = std::vector<int>;
  using DLink_t = DataLink<Container>;
  using PLink_t = SG::PackedLink<Container>;
  using Helper = SG::detail::PackedLinkVectorHelper<Container>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t foo_links_id = r.getAuxID<DataLink<Container> >
    ("foo_links", "",
       SG::AuxVarFlags::Linked);

  std::unique_ptr<SG::IAuxTypeVector> linkedVec = r.makeVector (foo_links_id, 0, 0);
  auto lv = reinterpret_cast<std::vector<DLink_t>*>(linkedVec->toVector());

  lv->emplace_back (0);
  lv->emplace_back (123);
  lv->emplace_back (124);

  auto dlinks = Helper::getLinkBaseSpan (*linkedVec);
  PLink_t plinks[] = {{0, 0}, {1, 10}, {2, 11}};
  assert (Helper::applyThinning (*linkedVec, plinks, std::size(plinks),
                                 dlinks, nullptr, store.get()) == true);

  store->remap (123, 456, 10, 20);
  store->remap (124, 457, 11, 21);
  store->remap (123, 456, 6, 12);
  store->remap (124, 457, 8, 28);
  (void)Helper::applyThinning (*linkedVec, plinks, std::size(plinks),
                               dlinks, nullptr, store.get());
  assert (plinks[0] == PLink_t(0, 0));
  assert (plinks[1] == PLink_t(3, 20));
  assert (plinks[2] == PLink_t(4, 21));
  assert (lv->size() == 5);
  assert (lv->at(0) == DLink_t());
  assert (lv->at(1) == DLink_t(123));
  assert (lv->at(2) == DLink_t(124));
  assert (lv->at(3) == DLink_t(456));
  assert (lv->at(4) == DLink_t(457));
#endif
}

//coverity[root_function]
int main()
{
  std::cout << "AthContainers/PackedLinkVectorHelper_test\n";
  test1();
  test_applyThinning();
  return 0;
}
