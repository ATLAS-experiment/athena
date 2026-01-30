/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/ELProxy_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Regression tests for ELProxy classes.
 */


#undef NDEBUG
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/tools/ELProxy.h"
#include "AthContainers/tools/PackedLinkConversions.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include <iostream>
#include <cassert>
#include <vector>


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


private:
  size_t m_sz;
};



} // namespace SG


void test_ELProxy()
{
  std::cout << "test_ELProxy\n";

  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using DLink_t = DataLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;
  using Converter_t = SG::detail::PackedLinkConverter<Cont_t>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t dlink_id = r.getAuxID<DLink_t> 
    ("plink_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t plink_id = r.getAuxID<PLink_t> 
    ("plink", "", SG::AuxVarFlags::None, dlink_id);

  SG::AuxVectorBase v (5); 
  SG::AuxStoreInternal store;
  v.setStore (&store);
  DLink_t* dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 3, 3));
  dlink[1] = DLink_t (10);
  dlink[2] = DLink_t (12);
  SG::IAuxTypeVector* linkedVec = store.linkedVector (plink_id);

  // Proxy holding converter by reference.
  {
    Converter_t c1 (v, plink_id, dlink_id);
    PLink_t pl;
    SG::detail::ELProxyInSpan<Cont_t> prox (pl, c1);
    pl = PLink_t (0, 0);
    Link_t el1 = prox;
    assert (el1.isDefault());
    assert (prox.isDefault());

    pl = PLink_t (1, 5);
    el1 = prox;
    assert (el1 == Link_t (10, 5));
    assert (prox == Link_t (10, 5));
    assert (prox != Link_t (10, 6));
    assert (prox == prox);
#ifndef XAOD_STANDALONE
    assert (!prox.isDefaultIndex());
#endif
    assert (!prox.hasCachedElement());
    assert (!prox.isDefault());
    assert (prox.key() == 10);
    assert (prox.index() == 5);

    prox = Link_t (12, 7);
    assert (pl == PLink_t (2, 7));
    assert (prox.key() == 12);
    assert (prox.index() == 7);
    assert (prox == Link_t (12, 7));

    prox = Link_t (13, 9);
    assert (pl == PLink_t (3, 9));
    assert (prox.key() == 13);
    assert (prox.index() == 9);
    assert (prox == Link_t (13, 9));
    assert (linkedVec->size() == 4);
    dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 4, 4));
    assert (dlink[3] == DLink_t (13));
  }

  // Proxy holding converter by value.
  {
    PLink_t pl;
    SG::detail::ELProxyT<SG::detail::ELProxyValBase<Cont_t> > prox (pl, v,
                                                                    plink_id,
                                                                    dlink_id);
    pl = PLink_t (0, 0);
    Link_t el1 = prox;
    assert (el1.isDefault());
    assert (prox.isDefault());

    pl = PLink_t (1, 5);
    el1 = prox;
    assert (el1 == Link_t (10, 5));
#ifndef XAOD_STANDALONE
    assert (!prox.isDefaultIndex());
#endif
    assert (!prox.hasCachedElement());
    assert (!prox.isDefault());
    assert (prox.key() == 10);
    assert (prox.index() == 5);

    prox = Link_t (12, 7);
    assert (pl == PLink_t (2, 7));
    assert (prox.key() == 12);
    assert (prox.index() == 7);
    assert (prox == Link_t (12, 7));

    prox = Link_t (14, 9);
    assert (pl == PLink_t (4, 9));
    assert (prox.key() == 14);
    assert (prox.index() == 9);
    assert (prox == Link_t (14, 9));
    assert (linkedVec->size() == 5);
    dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 5, 5));
    assert (dlink[4] == DLink_t (14));
  }
}


void test_ELProxyConverter()
{
  std::cout << "test_ELProxyConverter\n";

  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using DLink_t = DataLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t dlink_id = r.getAuxID<DLink_t> 
    ("plink_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t plink_id = r.getAuxID<PLink_t> 
    ("plink", "", SG::AuxVarFlags::None, dlink_id);

  SG::AuxVectorBase v (5); 
  SG::AuxStoreInternal store;
  v.setStore (&store);
  DLink_t* dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 3, 3));
  dlink[1] = DLink_t (10);
  dlink[2] = DLink_t (12);
  SG::IAuxTypeVector* linkedVec = store.linkedVector (plink_id);

  SG::detail::ELProxyInSpanConverter<Cont_t> pcnv (v, plink_id, dlink_id);
  Link_t el = std::as_const(pcnv) (PLink_t (1, 3));
  assert (el == Link_t (10, 3));

  PLink_t pl;
  auto prox = pcnv (pl);
  assert (prox.isDefault());
  pl = PLink_t (2, 4);
  assert (prox == Link_t (12, 4));

  prox = Link_t (10, 5);
  assert (pl == PLink_t (1, 5));

  prox = Link_t (20, 6);
  assert (pl == PLink_t (3, 6));
  assert (linkedVec->size() == 4);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 4, 4));
  assert (dlink[3] == DLink_t (20));
}


void test_ELSpanProxy()
{
  std::cout << "test_ELSpanProxy\n";

  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using DLink_t = DataLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t dlink_id = r.getAuxID<DLink_t> 
    ("plink_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t plink_id = r.getAuxID<PLink_t> 
    ("plink", "", SG::AuxVarFlags::None, dlink_id);

  SG::AuxVectorBase v (5); 
  SG::AuxStoreInternal store;
  v.setStore (&store);
  DLink_t* dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 3, 3));
  dlink[1] = DLink_t (10);
  dlink[2] = DLink_t (12);
  SG::IAuxTypeVector* linkedVec = store.linkedVector (plink_id);

  std::vector<PLink_t> velt { {0, 0}, {1, 3}, {2, 4} };
  SG::detail::ELSpanProxy<Cont_t, std::allocator<PLink_t> >
    prox (velt, v, plink_id, dlink_id);

  assert (prox.size() == 3);
  assert (prox[0].isDefault());
  assert (prox[1] == Link_t (10, 3));
  prox[1] = Link_t (13, 7);
  assert (prox[1] == Link_t (13, 7));
  assert (prox[2] == Link_t (12, 4));
  assert (linkedVec->size() == 4);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 4, 4));
  assert (dlink[3] == DLink_t (13));

  std::vector<Link_t> newlinks { {10, 11}, {14, 12}, {0, 0}, {13, 13} };
  prox = newlinks;
  assert (prox.size() == 4);
  assert (prox[0] == Link_t (10, 11));
  assert (prox[1] == Link_t (14, 12));
  assert (prox[2].isDefault());
  assert (prox[3] == Link_t (13, 13));
  assert (velt.size() == 4);
  assert (velt == (std::vector<PLink_t> { {1, 11}, {4, 12}, {0, 0}, {3, 13} }));
  assert (linkedVec->size() == 5);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 5, 5));
  assert (dlink[4] == DLink_t (14));

  std::vector<Link_t> el = prox;
  assert (el.size() == 4);
  assert (el[0] == newlinks[0]);
  assert (el[1] == newlinks[1]);
  assert (el[2].isDefault());
  assert (el[3] == newlinks[3]);

  prox.push_back (Link_t (13, 99));
  assert (prox == (std::vector<Link_t> {{10,11}, {14,12}, {}, {13,13}, {13,99}}));

  prox.resize (7, Link_t (13, 98));
  assert (prox == (std::vector<Link_t> {{10,11}, {14,12}, {}, {13,13}, {13,99}, {13,98}, {13,98}}));

  prox.erase (prox.begin() + 5);
  assert (prox == (std::vector<Link_t> {{10,11}, {14,12}, {}, {13,13}, {13,99}, {13,98}}));

  prox.resize (5);
  assert (prox == (std::vector<Link_t> {{10,11}, {14,12}, {}, {13,13}, {13,99}}));

  prox.erase (prox.begin()+2, prox.begin()+4);
  assert (prox == (std::vector<Link_t> {{10,11}, {14,12}, {13,99}}));

  assert (linkedVec->size() == 5);
  prox.insert (prox.begin()+1, Link_t (21, 97));
  assert (prox == (std::vector<Link_t> {{10,11}, {21,97}, {14,12}, {13,99}}));
  assert (linkedVec->size() == 6);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 6, 6));
  assert (dlink[5] == DLink_t (21));

  {
    std::vector<Link_t> telts {{13,61}, {21,62}};
    prox.insert (prox.begin()+2, telts.begin(), telts.end());
  }
  assert (prox == (std::vector<Link_t> {{10,11}, {21,97}, {13,61}, {21,62}, {14,12}, {13,99}}));

  prox.insert_range (prox.begin()+3, std::vector<Link_t> {{10,81}, {13,82}});
  assert (prox == (std::vector<Link_t> {{10,11}, {21,97}, {13,61}, {10,81}, {13,82}, {21,62}, {14,12}, {13,99}}));

  prox.append_range (std::vector<Link_t> {{21,83}, {14,84}});
  assert (prox == (std::vector<Link_t> {{10,11}, {21,97}, {13,61}, {10,81}, {13,82}, {21,62}, {14,12}, {13,99}, {21,83}, {14,84}}));

  prox.pop_back();
  assert (prox == (std::vector<Link_t> {{10,11}, {21,97}, {13,61}, {10,81}, {13,82}, {21,62}, {14,12}, {13,99}, {21,83}}));

  prox.assign (3, {21,79});
  assert (prox == (std::vector<Link_t> {{21,79}, {21,79}, {21,79}}));

  {
    std::vector<Link_t> telts {{13,61}, {21,62}};
    prox.assign (telts.begin(), telts.end());
  }
  assert (prox == (std::vector<Link_t> {{13,61}, {21,62}}));

  prox.assign_range (std::vector<Link_t> {{10,63}, {13,64}});
  assert (prox == (std::vector<Link_t> {{10,63}, {13,64}}));

  prox.clear();
  assert (prox.size() == 0);
  assert (velt.size() == 0);
}


void test_ELSpanConverter()
{
  std::cout << "test_ELSpanConverter\n";

  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using DLink_t = DataLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t dlink_id = r.getAuxID<DLink_t> 
    ("plink_linked", "", SG::AuxVarFlags::Linked);
  SG::auxid_t plink_id = r.getAuxID<PLink_t> 
    ("plink", "", SG::AuxVarFlags::None, dlink_id);

  SG::AuxVectorBase v (5); 
  SG::AuxStoreInternal store;
  v.setStore (&store);
  DLink_t* dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 3, 3));
  dlink[1] = DLink_t (10);
  dlink[2] = DLink_t (12);
  SG::IAuxTypeVector* linkedVec = store.linkedVector (plink_id);

  SG::detail::ELSpanConverter<Cont_t, std::allocator<PLink_t> >
    cnv (v, plink_id, dlink_id);

  std::vector<PLink_t> velt { {0, 0}, {1, 3}, {2, 4} };
  auto prox = cnv (velt);

  assert (prox.size() == 3);
  assert (prox[0].isDefault());
  assert (prox[1] == Link_t (10, 3));

  std::vector<Link_t> newlinks { {10, 11}, {14, 12}, {0, 0}, {13, 13} };
  prox = newlinks;
  assert (prox.size() == 4);
  assert (prox[0] == Link_t (10, 11));
  assert (prox[1] == Link_t (14, 12));
  assert (prox[2].isDefault());
  assert (prox[3] == Link_t (13, 13));
  assert (velt.size() == 4);
  assert (velt == (std::vector<PLink_t> { {1, 11}, {3, 12}, {0, 0}, {4, 13} }));
  assert (linkedVec->size() == 5);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 5, 5));
  assert (dlink[3] == DLink_t (14));
  assert (dlink[4] == DLink_t (13));

  std::vector<Link_t> el = prox;
  assert (el.size() == 4);
  assert (el[0] == newlinks[0]);
  assert (el[1] == newlinks[1]);
  assert (el[2].isDefault());
  assert (el[3] == newlinks[3]);
}

//coverity[root_function]
int main()
{
  std::cout << "AthContainers/ELProxy_test\n";
  test_ELProxy();
  test_ELProxyConverter();
  test_ELSpanProxy();
  test_ELSpanConverter();
  return 0;
}
