/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/PackedLinkConstAccessor_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Regression tests for PackedLinkConstAccessor.
 */

#undef NDEBUG
#include "AthContainers/PackedLinkConstAccessor.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/exceptions.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>
#include <type_traits>


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


namespace SG {


class AuxVectorBase
  : public SG::AuxVectorData
{
public:
  AuxVectorBase (size_t sz = 10) : m_sz (sz) {}
  virtual size_t size_v() const override { return m_sz; }
  virtual size_t capacity_v() const override { return m_sz; }

  using SG::AuxVectorData::setStore;
  void set (SG::AuxElement& b, size_t index)
  {
    b.setIndex (index, this);
  }
  void set (SG::ConstAuxElement& b, size_t index)
  {
    b.setIndex (index, this);
  }
  void clear (SG::AuxElement& b)
  {
    b.setIndex (0, 0);
  }

  static
  void clearAux (SG::AuxElement& b)
  {
    b.clearAux();
  }

  static
  void copyAux (SG::AuxElement& a, const SG::AuxElement& b)
  {
    a.copyAux (b);
  }

  static
  void testAuxElementCtor (SG::AuxVectorData* container,
                           size_t index)
  {
    SG::AuxElement bx (container, index);
    assert (bx.index() == index);
    assert (bx.container() == container);
  }

private:
  size_t m_sz;
};



} // namespace SG


void test1()
{
  std::cout << "test1\n";

  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using DLink = DataLink<Cont>;

  SG::ConstAccessor<PLink> ptyp1 ("plink");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t plink_id = r.findAuxID ("plink");
  SG::auxid_t dlink_id = r.findAuxID ("plink_linked");

  assert (ptyp1.auxid() == plink_id);
  assert (ptyp1.linkedAuxid() == dlink_id);

  {
    SG::ConstAccessor<PLink> p2 (plink_id);
    assert (p2.auxid() == plink_id);
    EXPECT_EXCEPTION (SG::ExcAuxTypeMismatch, (SG::ConstAccessor<SG::PackedLink<std::vector<float> > > (plink_id)));

    SG::auxid_t flink_id = r.getAuxID<SG::PackedLink<std::vector<float> > > ("flink");
    EXPECT_EXCEPTION (SG::ExcNoLinkedVar, (SG::ConstAccessor<SG::PackedLink<std::vector<float> > > (flink_id)));
  }

  SG::AuxElement b;
  SG::ConstAuxElement cb;
  assert (!ptyp1.isAvailable(b));
  assert (!ptyp1.isAvailable(cb));

  SG::AuxVectorBase v;
  v.set (b, 5);
  v.set (cb, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  const SG::AuxVectorBase& cv = v;

  assert (!ptyp1.isAvailable(v));
  assert (!ptyp1.isAvailable(cv));

  PLink* plink = reinterpret_cast<PLink*> (store.getData(plink_id, 10, 10));
  DLink* dlink = reinterpret_cast<DLink*> (store.getData(dlink_id, 2, 2));
  plink[5] = PLink (1, 10);
  dlink[1] = DLink (123);

  assert (ptyp1.isAvailable(b));
  assert (ptyp1.isAvailable(cb));
  assert (ptyp1.isAvailable(v));
  assert (ptyp1.isAvailable(cv));

  assert (ptyp1 (b).key() == 123);
  assert (ptyp1 (b).index() == 10);

  assert (ptyp1 (cb).key() == 123);
  assert (ptyp1 (cb).index() == 10);

  assert (ptyp1 (v, 5).key() == 123);
  assert (ptyp1 (v, 5).index() == 10);

  assert (ptyp1.getPackedLinkArray (v) == plink);
  assert (ptyp1.getDataLinkArray (v) == dlink);

  // Null links.
  {
    SG::AuxVectorBase v2;
    SG::AuxStoreInternal store2;
    v2.setStore (&store2);
    (void)store2.getData(plink_id, 10, 10);
    (void)store2.getData(dlink_id, 0, 0);
    assert (ptyp1 (v2, 5).key() == 0);
    assert (ptyp1 (v2, 5).isDefault());
  }
}


// spans
void test2()
{
  std::cout << "test2\n";

  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using DLink = DataLink<Cont>;
  using IdxType = std::result_of<decltype(&ElementLink<Cont>::index)(ElementLink<Cont>)>::type;

  SG::ConstAccessor<PLink> ptyp1 ("plink");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t plink_id = r.findAuxID ("plink");
  SG::auxid_t dlink_id = r.findAuxID ("plink_linked");

  SG::AuxVectorBase v (5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  PLink* plink = reinterpret_cast<PLink*> (store.getData(plink_id, 5, 5));
  DLink* dlink = reinterpret_cast<DLink*> (store.getData(dlink_id, 3, 3));
  plink[0] = PLink (1, 10);
  plink[1] = PLink (2, 11);
  plink[2] = PLink (0, 0);
  plink[3] = PLink (1, 13);
  plink[4] = PLink (2, 14);
  dlink[1] = DLink (123);
  dlink[2] = DLink (124);

  v.setStore (static_cast<SG::IAuxStore*>(nullptr));
  v.setStore (static_cast<SG::IConstAuxStore*>(&store));
  auto pspan = ptyp1.getPackedLinkSpan (v);
  auto dspan = ptyp1.getDataLinkSpan (v);
  assert (pspan.size() == 5);
  assert (pspan[1] == PLink (2, 11));
  assert (pspan[3] == PLink (1, 13));
  assert (dspan.size() == 3);
  assert (dspan[1].key() == 123);
  assert (dspan[2].key() == 124);

  auto span = ptyp1.getDataSpan (v);
  assert (span.size() == 5);
  assert (!span.empty());
  assert (span[1].key() == 124);
  assert (span[1].index() == 11);
  assert (span.front().key() == 123);
  assert (span.front().index() == 10);
  assert (span.back().key() == 124);
  assert (span.back().index() == 14);

  std::vector<IdxType> idx;
  for (ElementLink<Cont> el : span) {
    idx.push_back (el.isDefault() ? 0 : el.index());
  }
  assert (idx == (std::vector<IdxType> {10, 11, 0, 13, 14}));
}


// vector<PackedLink>
void test3()
{
  std::cout << "test3\n";

  using Cont = std::vector<int>;
  using DLink = DataLink<Cont>;
  using PLink = SG::PackedLink<Cont>;
  using VElt = std::vector<PLink>;
  using IdxType = std::result_of<decltype(&ElementLink<Cont>::index)(ElementLink<Cont>)>::type;

  SG::ConstAccessor<VElt> vtyp1 ("vlink");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t vlink_id = r.findAuxID ("vlink");
  SG::auxid_t dlink_id = r.findAuxID ("vlink_linked");

  assert (vtyp1.auxid() == vlink_id);
  assert (vtyp1.linkedAuxid() == dlink_id);

  {
    SG::ConstAccessor<VElt> v2 (vlink_id);
    assert (v2.auxid() == vlink_id);
    EXPECT_EXCEPTION (SG::ExcAuxTypeMismatch, (SG::ConstAccessor<std::vector<SG::PackedLink<std::vector<float> > > > (vlink_id)));

    SG::auxid_t fvlink_id = r.getAuxID<std::vector<SG::PackedLink<std::vector<float> > > > ("fvlink");
    EXPECT_EXCEPTION (SG::ExcNoLinkedVar, (SG::ConstAccessor<std::vector<SG::PackedLink<std::vector<float> > > > (fvlink_id)));
  }

  SG::AuxElement b;
  SG::ConstAuxElement cb;
  assert (!vtyp1.isAvailable(b));
  assert (!vtyp1.isAvailable(cb));

  SG::AuxVectorBase v;
  v.set (b, 5);
  v.set (cb, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  const SG::AuxVectorBase& cv = v;

  assert (!vtyp1.isAvailable(v));
  assert (!vtyp1.isAvailable(cv));

  VElt* vlink = reinterpret_cast<VElt*> (store.getData(vlink_id, 10, 10));
  DLink* dlink = reinterpret_cast<DLink*> (store.getData(dlink_id, 2, 2));
  vlink[5] = VElt{{1, 10}, {0, 0}, {1, 11}};
  dlink[1] = DLink (123);

  assert (vtyp1.isAvailable(b));
  assert (vtyp1.isAvailable(cb));
  assert (vtyp1.isAvailable(v));
  assert (vtyp1.isAvailable(cv));

  assert (vtyp1 (b).size() == 3);
  assert (vtyp1 (b)[0].key() == 123);
  assert (vtyp1 (b)[0].index() == 10);
  assert (vtyp1 (b)[1].key() == 0);
  assert (vtyp1 (b)[2].key() == 123);
  assert (vtyp1 (b)[2].index() == 11);

  {
    std::vector<IdxType> v;
    for (const ElementLink<Cont> el : vtyp1(b)) {
      if (el.isDefault()) {
        v.push_back (0);
      }
      else {
        v.push_back (el.key());
        v.push_back (el.index());
      }
    }
    assert (v == (std::vector<IdxType> {123, 10, 0, 123, 11}));
  }

  std::vector<ElementLink<Cont> > elv = vtyp1 (b);
  assert (elv == (std::vector<ElementLink<Cont> > {{123, 10}, {}, {123, 11}}));

  assert (vtyp1 (cb).size() == 3);
  assert (vtyp1 (cb)[2].key() == 123);
  assert (vtyp1 (cb)[0].key() == 123);
  assert (vtyp1 (cb)[0].index() == 10);

  assert (vtyp1 (v, 5).size() == 3);
  assert (vtyp1 (v, 5)[2].key() == 123);
  assert (vtyp1 (v, 5)[2].index() == 11);

  assert (vtyp1.getPackedLinkVectorArray (v) == vlink);
  assert (vtyp1.getDataLinkArray (v) == dlink);

  // Empty vectors.
  {
    SG::AuxVectorBase v2;
    SG::AuxStoreInternal store2;
    v2.setStore (&store2);
    (void)store2.getData(vlink_id, 10, 10);
    (void)store2.getData(dlink_id, 0, 0);
    assert (vtyp1 (v2, 5).empty());
  }
}


// spans with vectors
void test4()
{
  std::cout << "test4\n";

  using Cont = std::vector<int>;
  using PLink = SG::PackedLink<Cont>;
  using DLink = DataLink<Cont>;
  using VElt = std::vector<PLink>;
  using IdxType = std::result_of<decltype(&ElementLink<Cont>::index)(ElementLink<Cont>)>::type;

  SG::ConstAccessor<VElt> vtyp1 ("vlink");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t vlink_id = r.findAuxID ("vlink");
  SG::auxid_t dlink_id = r.findAuxID ("vlink_linked");

  SG::AuxVectorBase v (5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  VElt* vlink = reinterpret_cast<VElt*> (store.getData(vlink_id, 5, 5));
  DLink* dlink = reinterpret_cast<DLink*> (store.getData(dlink_id, 3, 3));
  vlink[0] = VElt{{1, 10}, {0, 0}, {1, 11}};
  vlink[1] = VElt{{2, 12}};
  vlink[3] = VElt{{0, 0}};
  vlink[4] = VElt{{1, 13}, {2, 14}};
  dlink[1] = DLink (123);
  dlink[2] = DLink (124);

  v.setStore (static_cast<SG::IAuxStore*>(nullptr));
  v.setStore (static_cast<SG::IConstAuxStore*>(&store));

  auto dspan = vtyp1.getDataLinkSpan (v);
  assert (dspan.size() == 3);
  assert (dspan[1].key() == 123);
  assert (dspan[2].key() == 124);

  auto pvspan = vtyp1.getPackedLinkVectorSpan (v);
  assert (pvspan.size() == 5);
  assert (pvspan[4] == (VElt{{1, 13}, {2, 14}}));

  auto pspan = vtyp1.getPackedLinkSpan (v, 1);
  assert (pspan.size() == 1);
  assert (pspan[0] == PLink (2, 12));

  auto span = vtyp1.getDataSpan (v);
  assert (span.size() == 5);
  assert (!span.empty());
  assert (span[0].size() == 3);
  assert (span[0][2].key() == 123);
  assert (span[0][2].index() == 11);
  assert (span.back().front().key() == 123);
  assert (span.back().front().index() == 13);

  std::vector<IdxType> idx;
  for (auto s : span) {
    for (ElementLink<Cont> el : s) {
      idx.push_back (el.isDefault() ? 0 : el.index());
    }
  }
  assert (idx == (std::vector<IdxType> {10, 0, 11, 12, 0, 13, 14}));
}


// To study generated code.
unsigned int asmtest [[maybe_unused]] (const SG::AuxElement& e,
                                       SG::ConstAccessor<std::vector<SG::PackedLink<std::vector<int >> > >& acc)
{
  using Cont = std::vector<int>;
  unsigned int out = 0;
  for (const ElementLink<Cont> el : acc(e)) {
    out += el.key() + el.index();
  }
  return out;
}


int main()
{
  std::cout << "AthContainers/PackedLinkConstAccessor_test\n";
  test1();
  test2();
  test3();
  test4();
  return 0;
}
