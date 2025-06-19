/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/PackedLinkConversions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Regression tests for PackedLink converter classes.
 */


#undef NDEBUG
#include "AthContainers/tools/PackedLinkConversions.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>


#ifndef XAOD_STANDALONE
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


void test_PackedLinkConstConverter()
{
  std::cout << "test_PackedLinkConstConverter\n";
  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using DLink_t = DataLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;
  using Converter_t = SG::detail::PackedLinkConstConverter<Cont_t>;

  std::vector<DLink_t> dlinks;
  SG::AuxDataSpanBase dlinks_spanBase {0, 0};

  Converter_t c1 (dlinks_spanBase);
  assert (c1 (PLink_t (0, 0)).isDefault());
  EXPECT_EXCEPTION( std::out_of_range, c1 (PLink_t (1, 1)) );

  dlinks.emplace_back (0);
  dlinks.emplace_back (10);
  dlinks_spanBase.beg = dlinks.data();
  dlinks_spanBase.size = dlinks.size();

  Converter_t c2 (dlinks_spanBase);
  assert (c2 (PLink_t (0, 0)).isDefault());
  assert (c2 (PLink_t (1, 1)) == Link_t (10, 1));
}


void test_PackedLinkVectorConstConverter()
{
  std::cout << "test_PackedLinkVectorConstConverter\n";
  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using DLink_t = DataLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;
  using Converter_t = SG::detail::PackedLinkVectorConstConverter<Cont_t>;

  std::vector<DLink_t> dlinks;
  SG::AuxDataSpanBase dlinks_spanBase {0, 0};

  Converter_t c1 (dlinks_spanBase);

  std::vector<PLink_t> plinks1 (3);
  auto r1 = c1 (plinks1);
  assert (r1.size() == 3);
  assert (r1[0].isDefault());
  assert (r1[1].isDefault());
  assert (r1[2].isDefault());

  plinks1[1] = PLink_t (1, 1);
  auto r2 = c1 (plinks1);
  assert (r2.size() == 3);
  assert (r2[0].isDefault());
  EXPECT_EXCEPTION( std::out_of_range, r2[1] );
  assert (r2[2].isDefault());

  dlinks.emplace_back (0);
  dlinks.emplace_back (10);
  dlinks.emplace_back (20);
  dlinks_spanBase.beg = dlinks.data();
  dlinks_spanBase.size = dlinks.size();

  Converter_t c3 (dlinks_spanBase);
  plinks1[2] = PLink_t (2, 2);
  auto r3 = c3 (plinks1);
  assert (r3.size() == 3);
  assert (r3[0].isDefault());
  assert (r3[1] == Link_t (10, 1));
  assert (r3[2] == Link_t (20, 2));
}


void test_PackedLinkConverter()
{
  std::cout << "test_PackedLinkConverter\n";
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

  Converter_t c1 (v, plink_id, dlink_id);
  assert (c1 (PLink_t (0, 0)).isDefault());
  assert (c1 (PLink_t (1, 1))== Link_t (10, 1));

  PLink_t pl;
  c1.set (pl, Link_t (12, 2));
  assert (pl == PLink_t (2, 2));

  c1.set (pl, Link_t (20, 9));
  assert (pl == PLink_t (3, 9));
  assert (linkedVec->size() == 4);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 4, 4));
  assert (dlink[3] == DLink_t (20));

  std::vector<PLink_t> vpl;
  std::vector<Link_t> vel { Link_t (30, 11), Link_t (0, 0), Link_t (12, 5) };
  c1.set (vpl, vel);
  std::vector<PLink_t> vpl_exp { {4, 11}, {0, 0}, {2, 5} };
  assert (vpl == vpl_exp);
  assert (linkedVec->size() == 5);
  dlink = reinterpret_cast<DLink_t*> (store.getData(dlink_id, 5, 5));
  assert (dlink[3] == DLink_t (20));
  assert (dlink[4] == DLink_t (30));

  std::vector<Link_t> vel2 { Link_t (12, 80), Link_t (30, 81) };
  c1.insert (vpl, 1, vel2);
  assert (linkedVec->size() == 5);
  std::vector<PLink_t> vpl_exp2 { {4, 11}, {2, 80}, {4, 81}, {0, 0}, {2, 5} };
  assert (vpl == vpl_exp2);
}



int main()
{
  std::cout << "AthContainers/PackedLinkConversions_test\n";
  test_PackedLinkConstConverter();
  test_PackedLinkVectorConstConverter();
  test_PackedLinkConverter();
  return 0;
}
