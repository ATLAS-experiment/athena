/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCOOLConditions/LArCondSuperCellBase.h"

// Gaudi/Athena
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"
#include "AthenaBaseComps/AthCheckMacros.h"

LArCondSuperCellBase::LArCondSuperCellBase(const std::string& name) :
  AthMessaging(name),
  m_isInitialized(false),
  m_scOnlineID(nullptr)
{
}
  
StatusCode LArCondSuperCellBase::initializeBase() {

  ATH_MSG_DEBUG( "initializeBase " );

  if (m_isInitialized) {
    ATH_MSG_DEBUG( "already initialized - returning " );
    return (StatusCode::SUCCESS);
  }
  //Get SuperCellID ...
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  ATH_CHECK_WITH_CONTEXT( detStore.isValid(), "LArCondSuperCellBase" );
  ATH_CHECK_WITH_CONTEXT( detStore->retrieve( m_scOnlineID,"LArOnline_SuperCellID"), "LArCondSuperCellBase" );

  m_isInitialized = true;
  ATH_MSG_DEBUG( "end initializeBase " );
  return (StatusCode::SUCCESS);
}
