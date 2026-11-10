/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/AuxDataTestDecor.cxx
 * @author snyder@bnl.gov
 * @date Apr 2017
 * @brief Algorithm to test reading decorations using handles.
 */


#include "xAODTestReadDecor.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CVec.h"
#include "StoreGate/ReadDecorHandle.h"
#include "AthenaKernel/errorcheck.h"
#include <print>


namespace DMTest {


/**
 * @brief Constructor.
 * @param name The algorithm name.
 * @param svc The service locator.
 */
xAODTestReadDecor::xAODTestReadDecor (const std::string &name,
                                      ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm (name, pSvcLocator)
{
}
  

/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestReadDecor::initialize()
{
  if (!m_cvecName.value().empty()) {
    m_cvecDecorKey  = m_readPrefix.value() + m_cvecName.value() +  "."  + m_decorName.value();
    ATH_CHECK( m_cvecDecorKey.initialize() );
  }

  if (!m_cinfoName.value().empty()) {
    m_cinfoDecorKey = m_readPrefix.value() + m_cinfoName.value() + "." + m_decorName.value();
    m_cinfoBaseDecorKey = m_readPrefix.value() + m_cinfoName.value() + "." + m_decorName.value() + "Base";
    m_objDecorKey = m_readPrefix.value() + m_cinfoName.value() + "." + m_decorName.value();
    ATH_CHECK( m_cinfoDecorKey.initialize() );
    ATH_CHECK( m_cinfoBaseDecorKey.initialize() );
    ATH_CHECK( m_objDecorKey.initialize() );
  }
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestReadDecor::execute (const EventContext& ctx) const
{
  if (!m_cvecDecorKey.empty()) {
    std::ostringstream ss;
    SG::ReadDecorHandle<CVec, int> cvecDecor (m_cvecDecorKey, ctx);
    if (!cvecDecor.isPresent()) return StatusCode::FAILURE;
    for (const C* celt : *cvecDecor) {
      std::print (ss, " {}", cvecDecor(*celt));
    }
    ATH_MSG_INFO ("{}:{}", m_cvecDecorKey.key(), ss.str());
  }

  if (!m_cinfoDecorKey.empty()) {
    SG::ReadDecorHandle<C, int> cinfoDecor (m_cinfoDecorKey, ctx);
    if (!cinfoDecor.isPresent()) return StatusCode::FAILURE;
    ATH_MSG_INFO ("{}: {}", m_cinfoDecorKey.key(), cinfoDecor(0));
  }

  if (!m_cinfoBaseDecorKey.empty()) {
    SG::ReadDecorHandle<C, int> cinfoBaseDecor (m_cinfoBaseDecorKey, ctx);
    if (!cinfoBaseDecor.isPresent()) return StatusCode::FAILURE;
    ATH_MSG_INFO ("{}: {}", m_cinfoBaseDecorKey.key(), cinfoBaseDecor(0));
  }

  if (!m_objDecorKey.empty()) {
    SG::ReadDecorHandle<SG::AuxElement, int> objDecor (m_objDecorKey, ctx);
    if (!objDecor.isPresent()) return StatusCode::FAILURE;
    ATH_MSG_INFO ("{}: {}", m_objDecorKey.key(), objDecor(0));
  }

  if (!m_cinfoDecorKey.empty()) {
    SG::ReadDecorHandleKey<C> testKey (m_cinfoDecorKey);
    testKey = testKey.key() + "_test";
    SG::ReadDecorHandle<C, int> cinfoDecorTest (testKey, ctx);
    if (!cinfoDecorTest.isPresent()) return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm finalization; called at the end of the job.
 */
StatusCode xAODTestReadDecor::finalize()
{
  return StatusCode::SUCCESS;
}


} // namespace DMTest

