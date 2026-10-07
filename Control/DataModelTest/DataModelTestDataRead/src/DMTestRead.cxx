/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  src/DMTestRead.cxx
 * @author snyder@bnl.gov
 * @date Nov 2005
 * @brief Algorithm to test reading @c DataVector data.
 *
 *        We read information using all four types,
 *        @c BVec, @c DVec, @c BDer, @c DDer.
 *        When we run, we have the inheritance relationship set up
 *        between @c BVec and @c DVec.
 */

#undef NDEBUG

#include "DMTestRead.h"
#include "DataModelAthenaPool/ElementLinkVectorCnv_p1.h"
#include "DataModelAthenaPool/ElementLinkCnv_p3.h"
#include "DataModelAthenaPool/DataLinkCnv_p1.h"
#include "DataModelTestDataRead/BVec.h"
#include "DataModelTestDataRead/BDer.h"
#include "DataModelTestDataRead/DVec.h"
#include "DataModelTestDataRead/DDer.h"
#include "DataModelTestDataRead/ELVec.h"
#include "AthContainers/ClassName.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/errorcheck.h"
#include "CxxUtils/checker_macros.h"
#include <iostream>
#include <print>
#include <sstream>
#include <cassert>


namespace DMTest {


/**
 * @brief Constructor.
 * @param name The algorithm name.
 * @param svc The service locator.
 */
DMTestRead::DMTestRead (const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm (name, pSvcLocator)
{
}


/**
 * @brief Print out one of our test objects of type @c VEC from storegate.
 * @param key The storegate key to use.
 */
template <class VEC>
StatusCode DMTestRead::print_vec (const std::string& key) const
{
  if (!evtStore()->contains<VEC> (key))
  {
    ATH_MSG_INFO("{} not in SG; ignored.", key);
    return StatusCode::SUCCESS;
  }

  const VEC* vec = nullptr;
  ATH_CHECK( evtStore()->retrieve (vec, key) );
  std::ostringstream ost;
  std::print (ost, "{} as {}: ", key, ClassName<VEC>::name());
  for (unsigned i=0; i < vec->size(); i++)
    std::print (ost, "{} ", (*vec)[i]->m_x);
  ATH_MSG_INFO (ost.str());

  return StatusCode::SUCCESS;
}


StatusCode DMTestRead::print_elvec (const std::string& key) const
{
  const ELVec* vec = nullptr;
  ATH_CHECK( evtStore()->retrieve (vec, key) );
  std::vector<ElementLink<BVec> > el = vec->m_el;
  std::ostringstream ost;
  std::print (ost, "{}: ", key);
  for (size_t i = 0; i < el.size(); i++) {
    const DMTest::B* b = *el[i];
    el[i].toPersistent();
    std::print (ost, "{} ", b->m_x);
  }
  ATH_MSG_INFO (ost.str());
  return StatusCode::SUCCESS;
}


StatusCode DMTestRead::remap_test() const
{
  const ELVec* vec = nullptr;
  ATH_CHECK( evtStore()->retrieve (vec, "elv_remap") );

  ElementLinkCnv_p3<ElementLink<BVec> > elcnv;
  std::vector<ElementLink<BVec> > el2; // Transient
  el2.resize (vec->m_el2_p.size());
  for (size_t i=0; i < vec->m_el2_p.size(); i++)
    elcnv.persToTrans (&vec->m_el2_p[i], &el2[i], msg());

  std::ostringstream ost1;
  std::print (ost1, "elv_remap: ");
  for (size_t i = 0; i < el2.size(); i++) {
    const DMTest::B* b = *el2[i];
    std::print (ost1, "{} ", b->m_x);
  }
  ATH_MSG_INFO (ost1.str());

  ElementLinkVectorCnv_p1<ElementLinkVector<BVec> > elvcnv;
  ElementLinkVector<BVec> elv2;    // Transient
  elvcnv.persToTrans (&vec->m_elv2_p, &elv2, msg());

  std::ostringstream ost2;
  std::print (ost2, "elv_remap v2: ");
  for (size_t i = 0; i < elv2.size(); i++) {
    const DMTest::B* b = *elv2[i];
    std::print (ost2, "{} ", b->m_x);
  }
  ATH_MSG_INFO (ost2.str());

  DataLinkCnv_p1<DataLink<BVec> > dlcnv;
  std::vector<DataLink<BVec> > dl2; // Transient
  dl2.resize (vec->m_dl2_p.size());
  for (size_t i=0; i < vec->m_dl2_p.size(); i++)
    dlcnv.persToTrans (&vec->m_dl2_p[i], &dl2[i], msg());

  const BVec* b3 = nullptr;
  ATH_CHECK( evtStore()->retrieve (b3, "b3") );
  assert (dl2[0].cptr() == b3);
  assert (dl2[1].cptr() == b3);

  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode DMTestRead::execute(const EventContext& /*ctx*/)
{
  // Test reading our four types.
  ATH_CHECK( print_vec<BVec> ("bvec") );
  ATH_CHECK( print_vec<BDer> ("bder") );
  ATH_CHECK( print_vec<DVec> ("dvec") );
  ATH_CHECK( print_vec<DDer> ("dder") );

  // Test using implicit symlinks.
  ATH_CHECK( print_vec<BVec> ("bder") );
  ATH_CHECK( print_vec<BVec> ("dvec") );
  ATH_CHECK( print_vec<BVec> ("dder") );
  ATH_CHECK( print_vec<DVec> ("dder") );

  ATH_CHECK( print_elvec ("elvec") );

  ATH_CHECK( remap_test() );

  return StatusCode::SUCCESS;
}

} // namespace DMTest
