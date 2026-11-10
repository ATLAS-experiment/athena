/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file DataModelTestDataCommon/src/AuxDataTestDecor.cxx
 * @author snyder@bnl.gov
 * @date May 2014
 * @brief Algorithm to test adding decorations to xAOD data.
 */


#include "xAODTestDecor.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CVec.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthenaKernel/errorcheck.h"


namespace DMTest {


/**
 * @brief Constructor.
 * @param name The algorithm name.
 * @param svc The service locator.
 */
xAODTestDecor::xAODTestDecor (const std::string &name,
                              ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm (name, pSvcLocator)
{
}
  

/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestDecor::initialize()
{
  m_cvecDecorKey  = m_readPrefix.value() + "cvec."  + m_decorName.value();
  m_cinfoDecorKey = m_readPrefix.value() + "cinfo." + m_decorName.value();
  m_objDecorKey = m_readPrefix.value() + "cinfo." + m_decorName.value() + "Base";
  m_ctrigDecorKey = m_readPrefix.value() + "ctrig." + m_decorName.value();

  ATH_CHECK( m_eventInfoKey.initialize() );
  ATH_CHECK( m_cvecDecorKey.initialize  (m_doCVec) );
  ATH_CHECK( m_cinfoDecorKey.initialize (m_doCInfo) );
  ATH_CHECK( m_objDecorKey.initialize (m_doCInfo) );
  ATH_CHECK( m_ctrigDecorKey.initialize (m_doCTrig) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestDecor::execute (const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
  unsigned int count = eventInfo->eventNumber() + 1;

  if (m_doCInfo) {
    SG::WriteDecorHandle<C, int> cinfoDecor (m_cinfoDecorKey, ctx);
    if (!cinfoDecor.isPresent()) {
      ATH_MSG_ERROR( "cinfoDecor.isPresent check fails" );
      return StatusCode::FAILURE;
    }
    cinfoDecor(0) = cinfoDecor->anInt() + (m_offset ? 400 + m_offset : count * 2000);

    SG::WriteDecorHandle<SG::AuxElement, int> objDecor (m_objDecorKey, ctx);
    objDecor(0) = cinfoDecor->anInt() + (m_offset ? 400 + m_offset : count * 2000) + 1;
  }

  if (m_doCVec) {
    SG::WriteDecorHandle<CVec, int> cvecDecor (m_cvecDecorKey, ctx);
    if (!cvecDecor.isPresent()) {
      ATH_MSG_ERROR( "cvecDecor.isPresent check fails" );
      return StatusCode::FAILURE;
    }
    for (const C* celt : *cvecDecor) {
      cvecDecor(*celt) = celt->anInt() + (m_offset ? m_offset.value() : count*300);
    }
  }

  if (m_doCTrig) {
    SG::WriteDecorHandle<CVec, int> ctrigDecor (m_ctrigDecorKey, ctx);
    if (!ctrigDecor.isPresent()) {
      ATH_MSG_ERROR( "ctrigDecor.isPresent check fails" );
      return StatusCode::FAILURE;
    }
    for (const C* celt : *ctrigDecor) {
      ctrigDecor(*celt) = celt->anInt() + (m_offset ? 600 + m_offset : -count*20);
    }
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm finalization; called at the end of the job.
 */
StatusCode xAODTestDecor::finalize()
{
  return StatusCode::SUCCESS;
}


} // namespace DMTest

