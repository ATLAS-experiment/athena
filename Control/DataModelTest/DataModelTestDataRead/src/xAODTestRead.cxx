/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  src/xAODTestRead.cxx
 * @author snyder@bnl.gov
 * @date May 2014
 * @brief Algorithm to test reading xAOD data.
 */


#include "xAODTestRead.h"
#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/CVecWithData.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CAuxContainer.h"
#include "DataModelTestDataCommon/CTrigAuxContainer.h"
#include "DataModelTestDataRead/GVec.h"
#include "DataModelTestDataRead/G.h"
#include "DataModelTestDataRead/GAuxContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthenaKernel/errorcheck.h"
#include <memory>
#include <print>
#include <sstream>


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestRead::initialize()
{
  ATH_CHECK( m_ctrigReadKey.initialize() );
  ATH_CHECK( m_gvecReadKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_cvecWDReadKey.initialize() );
  ATH_CHECK( m_ctrigWriteKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_gvecWriteKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_cvecWDWriteKey.initialize (SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestRead::execute (const EventContext& ctx) const
{
  ATH_MSG_INFO (ctx.evt()+1);

  static const C::Accessor<int> anInt2 ("anInt2");
  static const C::Accessor<int> anInt10 ("anInt10");
  static const C::Accessor<int> dInt1 ("dInt1");
  static const C::Accessor<int> dInt100 ("dInt100");
  static const C::Accessor<int> dInt150 ("dInt150");
  static const C::Accessor<int> dInt200 ("dInt200");

  // Ordering of auxid is not reliable.  Sort by name.
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  std::vector<std::string> names;

  SG::ReadHandle<CVec> ctrig (m_ctrigReadKey, ctx);

  names.clear();
  for (SG::auxid_t auxid : ctrig->getAuxIDs())
    names.push_back (r.getName(auxid));
  std::sort (names.begin(), names.end());
  ATH_MSG_INFO ("ctrig aux items: {:n:s}", names);

  for (const C* c : *ctrig) {
    std::ostringstream ost;
    std::print (ost, " anInt1 {} aFloat: {} anInt2: {} dInt1: {}",
                c->anInt(), c->aFloat(), anInt2(*c), dInt1(*c));
    if (dInt100.isAvailable(*c))
      std::print (ost, " dInt100: {}", dInt100(*c));
    if (dInt150.isAvailable(*c))
      std::print (ost, " dInt150: {}", dInt150(*c));
    if (dInt200.isAvailable(*c))
      std::print (ost, " dInt200: {}", dInt200(*c));
    if (anInt10.isAvailable(*c))
      std::print (ost, " anInt10: {}", anInt10(*c));
    ATH_MSG_INFO (ost.str());
  }

  if (!m_ctrigWriteKey.empty()) {
    auto ctrignew = std::make_unique<CVec>();
    auto trig_store = std::make_unique<CTrigAuxContainer>();
    ctrignew->setStore (trig_store.get());
    for (size_t i = 0; i < ctrig->size(); i++) {
      ctrignew->push_back (new C);
      *ctrignew->back() = *(*ctrig)[i];
    }
    SG::WriteHandle<CVec> ctrigWrite (m_ctrigWriteKey, ctx);
    ATH_CHECK( ctrigWrite.record (std::move(ctrignew),
                                  std::move(trig_store)) );
  }

  if (!m_gvecReadKey.empty()) {
    ATH_CHECK( read_gvec (ctx) );
  }

  ATH_CHECK( read_cvec_with_data (ctx) );
  //ATH_CHECK( read_cview() );

  return StatusCode::SUCCESS;
}


/**
 * @brief Test reading container with additional data.
 */
StatusCode xAODTestRead::read_cvec_with_data (const EventContext& ctx) const
{
  SG::ReadHandle<CVecWithData> vec (m_cvecWDReadKey, ctx);

  static const C::Accessor<int> anInt10 ("anInt10");
  std::ostringstream ost;
  std::print (ost, "{} {}:", m_cvecWDReadKey.key(), vec->meta1);
  for (const C* c : *vec) {
    std::print (ost, " {}", c->anInt());
    if (anInt10.isAvailable(*c))
      std::print (ost, "({})", anInt10(*c));
  }
  ATH_MSG_INFO (ost.str());

  if (!m_cvecWDWriteKey.empty()) {
    auto vecnew = std::make_unique<CVecWithData>();
    auto store = std::make_unique<CAuxContainer>();
    vecnew->setStore (store.get());
    for (size_t i = 0; i < vec->size(); i++) {
      vecnew->push_back (new C);
      *vecnew->back() = *(*vec)[i];
    }
    SG::WriteHandle<CVecWithData> cvecWDWrite (m_cvecWDWriteKey, ctx);
    ATH_CHECK( cvecWDWrite.record (std::move(vecnew),
                                   std::move(store)) );
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Test reading GVec object.
 */
StatusCode xAODTestRead::read_gvec (const EventContext& ctx) const
{
  SG::ReadHandle<GVec> gvec (m_gvecReadKey, ctx);
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  std::vector<std::string> names;
  for (SG::auxid_t auxid : gvec->getAuxIDs())
    names.push_back (r.getName(auxid));
  std::sort (names.begin(), names.end());
  std::ostringstream ost3;
  std::println (ost3, "gvec aux items: {:n:s}", names);
  for (const G* g : *gvec) {
    std::println (ost3, " anInt {} gFloat {} gvFloat {:n}",
                  g->anInt(), g->gFloat(), g->gvFloat());
  }
  ATH_MSG_INFO (ost3.str());

  if (!m_gvecWriteKey.empty()) {
    auto gvecnew = std::make_unique<GVec>();
    auto gstore = std::make_unique<GAuxContainer>();
    gvecnew->setStore (gstore.get());
    for (size_t i = 0; i < gvec->size(); i++) {
      gvecnew->push_back (new G);
      *gvecnew->back() = *(*gvec)[i];
    }
    SG::WriteHandle<GVec> gvecWrite (m_gvecWriteKey, ctx);
    ATH_CHECK( gvecWrite.record (std::move(gvecnew),
                                 std::move(gstore)) );
  }

  return StatusCode::SUCCESS;
}


#if 0
/**
 * @brief Test reading view container.
 */
StatusCode xAODTestRead::read_cview() const
{
  if (!evtStore()->contains<CView> (m_readPrefix + "cview")) {
    ATH_MSG_INFO( "(No {}cview view container.)", m_readPrefix );
    return StatusCode::SUCCESS;
  }

  const CView* cview = nullptr;
  static const C::Accessor<int> anInt10 ("anInt10");
  CHECK( evtStore()->retrieve (cview, m_readPrefix + "cview") );
  std::ostringstream ost;
  std::print (ost, "{}cview:", m_readPrefix);
  for (const C* c : *cview) {
    std::print (ost, " {}", c->anInt());
    if (anInt10.isAvailable(*c))
      std::print (ost, "({})", anInt10(*c));
  }
  ATH_MSG_INFO (ost.str());

  if (!m_writePrefix.empty()) {
    bool LOCKED = false;
    CHECK (evtStore()->record (std::make_unique<CView> (*cview),
                               m_writePrefix + "cview", LOCKED));
  }

  return StatusCode::SUCCESS;
}
#endif


/**
 * @brief Algorithm finalization; called at the end of the job.
 */
StatusCode xAODTestRead::finalize()
{
  return StatusCode::SUCCESS;
}


} // namespace DMTest

