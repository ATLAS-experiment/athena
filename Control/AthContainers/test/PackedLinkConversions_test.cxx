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
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>


#ifndef XAOD_STANDALONE
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( std::vector<int>, 12345, 0 )
#endif


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


int main()
{
  std::cout << "AthContainers/PackedLinkConversions_test\n";
  test_PackedLinkConstConverter();
  test_PackedLinkVectorConstConverter();
  return 0;
}
