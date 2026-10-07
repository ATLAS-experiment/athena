/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file  DataModelTestDataCommon/src/xAODTestReadPVec.cxx
 * @author snyder@bnl.gov
 * @date Sep, 2024
 * @brief Algorithm to test reading xAOD data with packed containers.
 */


#include "xAODTestReadPVec.h"
#include "DataModelTestDataCommon/PVec.h"
#include "DataModelTestDataCommon/P.h"
#include "DataModelTestDataCommon/PAuxContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "AthContainers/ConstAccessor.h"
#include <memory>
#include <sstream>
#include <print>


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestReadPVec::initialize()
{
  ATH_CHECK( m_pvecKey.initialize() );
  if (!m_writeKey.key().empty())
    ATH_CHECK( m_writeKey.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestReadPVec::execute (const EventContext& ctx) const
{
  SG::ReadHandle<DMTest::PVec> pvec (m_pvecKey, ctx);

  const static SG::ConstAccessor<unsigned int> dpInt1 ("dpInt1");
  const static SG::ConstAccessor<std::vector<float> > dpvFloat ("dpvFloat");

  for (const P* p : *pvec) {
    std::ostringstream ost;
    std::print (ost, " pInt: {} pFloat: {:.2f}", p->pInt(), p->pFloat());
    if (dpInt1.isAvailable(*p))
      std::print (ost, " dpInt1: {}", dpInt1(*p));
    std::println(ost);
    
    std::println (ost, "  pvInt: {}", p->pvInt());
    std::println (ost, "  pvFloat: {::.3f}", p->pvFloat());

    if (dpvFloat.isAvailable(*p))
    {
      std::println (ost, "  dpvFloat: {::.3f}", dpvFloat(*p));
    }

    ATH_MSG_INFO (ost.str());
  }

  if (!m_writeKey.key().empty()) {
    auto vecnew = std::make_unique<PVec>();
    auto store = std::make_unique<PAuxContainer>();
    vecnew->setStore (store.get());
    for (size_t i = 0; i < pvec->size(); i++) {
      vecnew->push_back (new P);
      *vecnew->back() = *(*pvec)[i];
    }
    SG::WriteHandle<DMTest::PVec> writevec (m_writeKey, ctx);
    ATH_CHECK( writevec.record(std::move(vecnew), std::move(store)) );
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest

