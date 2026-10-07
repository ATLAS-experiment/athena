/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  DataModelTestDataRead/src/xAODTestReadHVec.cxx
 * @author snyder@bnl.gov
 * @date Oct, 2016
 * @brief Algorithm to test reading xAOD data with schema evolution (HVec/HView)
 */


#include "xAODTestReadHVec.h"
#include "DataModelTestDataRead/HVec.h"
#include "DataModelTestDataRead/HView.h"
#include "DataModelTestDataRead/H.h"
#include "DataModelTestDataRead/HAuxContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include <memory>
#include <print>
#include <sstream>


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestReadHVec::initialize()
{
  ATH_CHECK( m_hvecKey.initialize() );
  ATH_CHECK( m_hviewKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_vecWriteKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_viewWriteKey.initialize(SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestReadHVec::execute (const EventContext& ctx) const
{
  const DMTest::HVec* hvec = SG::get (m_hvecKey, ctx);
  std::ostringstream ost1;
  std::print (ost1, "{}:", m_hvecKey.key());
  for (const H* h : *hvec)
    std::print (ost1, " {}", h->aFloat());
  ATH_MSG_INFO (ost1.str());

  if (!m_hviewKey.empty()) {
    const DMTest::HVec* hview = SG::get (m_hviewKey, ctx);
    std::ostringstream ost2;
    std::print (ost2, "{}:", m_hviewKey.key());
    for (const H* h : *hview)
      std::print (ost2, " {}", h->aFloat());
    ATH_MSG_INFO (ost2.str());


    if (!m_vecWriteKey.key().empty()) {
      auto vecnew = std::make_unique<HVec>();
      auto store = std::make_unique<HAuxContainer>();
      vecnew->setStore (store.get());
      for (size_t i = 0; i < hview->size(); i++) {
        vecnew->push_back (new H);
        *vecnew->back() = *(*hview)[i];
      }

      auto viewnew = std::make_unique<HView>();
      for (size_t i = 0; i < vecnew->size(); i++)
        viewnew->push_back (vecnew->at(vecnew->size()-1-i));

      SG::WriteHandle<DMTest::HVec> writevec (m_vecWriteKey, ctx);
      SG::WriteHandle<DMTest::HView> writeview (m_viewWriteKey, ctx);
      ATH_CHECK( writevec.record (std::move(vecnew), std::move(store)) );
      ATH_CHECK( writeview.record (std::move(viewnew)) );
    }
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest

