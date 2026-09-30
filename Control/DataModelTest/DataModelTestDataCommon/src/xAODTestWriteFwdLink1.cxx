/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWriteFwdLink1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2023
 * @brief Testing writing of forward-declared DataLink.
 */


#include "xAODTestWriteFwdLink1.h"
#include "DataModelTestDataCommon/C.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthLinks/DataLink.h"
#include "AthContainersInterfaces/IAuxStore.h"


namespace DMTest {


StatusCode xAODTestWriteFwdLink1::initialize()
{
  ATH_CHECK( m_cvecKey.initialize() );
  return StatusCode::SUCCESS;
}


StatusCode xAODTestWriteFwdLink1::execute (const EventContext& ctx) const
{
  auto cvec = std::make_unique<DMTest::CVec>();
  for (size_t i = 0; i < 10; i++)
    cvec->push_back (std::make_unique<DMTest::C>());
  cvec->setStore (DataLink<SG::IAuxStore> (m_cvecKey.key() + "Aux.", ctx));
  SG::WriteHandle<DMTest::CVec> cvec_h (m_cvecKey, ctx);
  ATH_CHECK (cvec_h.record (std::move (cvec)));
  return StatusCode::SUCCESS;
}


} // namespace DMTest

