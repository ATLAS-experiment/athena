/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// =============================================================================
#include "CaloRingerJetsReader.h"

#include <algorithm>

#include "AthenaKernel/getMessageSvc.h"
#include "PATCore/AcceptData.h"
#include "StoreGate/ReadHandle.h"

namespace Ringer {

// =============================================================================
CaloRingerJetsReader::CaloRingerJetsReader(const std::string& type,
                                 const std::string& name,
                                 const ::IInterface* parent) :
  CaloRingerInputReader(type, name, parent),
  m_clRingsBuilderJetFctor(nullptr)
{

  // declare interface
  declareInterface<ICaloRingerJetsReader>(this);

}

// =============================================================================
CaloRingerJetsReader::~CaloRingerJetsReader()
{
  delete m_clRingsBuilderJetFctor;
}

// =============================================================================
StatusCode CaloRingerJetsReader::initialize()
{

  ATH_CHECK( CaloRingerInputReader::initialize() );

  ATH_CHECK(m_inputJetContainerKey.initialize());

  if ( m_builderAvailable ) {
    // Initialize our fctor
    m_clRingsBuilderJetFctor =
      new BuildCaloRingsJetFctor<xAOD::JetContainer>(
        m_inputJetContainerKey.key(),
        m_crBuilder,
        Athena::getMessageSvc(),
        this
      );
    ATH_CHECK( m_clRingsBuilderJetFctor->initialize() );
  }

  return StatusCode::SUCCESS;

}

// =============================================================================
StatusCode CaloRingerJetsReader::finalize()
{
  return StatusCode::SUCCESS;
}

// =============================================================================
StatusCode CaloRingerJetsReader::execute()
{

  ATH_MSG_DEBUG("Entering " << name() << " execute, m_builderAvailable = " << m_builderAvailable);

   // Retrieve jets
  SG::ReadHandle<xAOD::JetContainer> jets(m_inputJetContainerKey);
  // check is only used for serial running; remove when MT scheduler used
  if(!jets.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve "<< m_inputJetContainerKey);
    return StatusCode::FAILURE;
  }

  // Check if requested to run CaloRings Builder:
  if ( m_builderAvailable ) {
    ATH_CHECK( m_clRingsBuilderJetFctor->prepareJetToLoopFor(jets->size()) );

    // loop over our particles:
    for ( const auto *const jet : *jets ){
      m_clRingsBuilderJetFctor->operator()( jet );
    }

    m_clRingsBuilderJetFctor->checkJetRelease();
  }

  return StatusCode::SUCCESS;
}

} // namespace Ringer
