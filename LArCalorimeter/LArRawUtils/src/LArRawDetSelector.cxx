/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AtlasDetDescr/AtlasDetectorID.h"

#include "LArRawUtils/LArRawDetSelector.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/StoreGateSvc.h"

LArRawDetSelector::LArRawDetSelector ( const LArRawChannelContainer*  )
  : m_onlineID(nullptr),
    m_caloCellID(nullptr),
    m_em(false),
    m_hec(false),
    m_fcal(false)
{
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};

  detStore->retrieve(m_onlineID).orThrow(
    "Faild to get LArOnlineID helper");

  detStore->retrieve(m_caloCellID).orThrow(
    "Faild to get LArOnlineID helper");
}

void LArRawDetSelector::setDet(const Identifier& id ){

  m_em  = m_caloCellID->is_lar_em(id) ;
  m_hec  = m_caloCellID->is_lar_hec(id) ;
  m_fcal  = m_caloCellID->is_lar_fcal(id) ;

}

void LArRawDetSelector::setDet(const HWIdentifier& chid ){

  m_em    = m_onlineID->isEMBchannel(chid) || m_onlineID->isEMECchannel(chid);
  m_hec   = m_onlineID->isHECchannel(chid);
  m_fcal  = m_onlineID->isFCALchannel(chid);

}



bool LArRawDetSelector::select(const LArRawChannel* rc){

  const HWIdentifier chid=rc->identify();

  if(m_em && (m_onlineID->isEMBchannel(chid) || m_onlineID->isEMECchannel(chid))) return true;

  if(m_hec && m_onlineID->isHECchannel(chid)) return true;

  if(m_fcal && m_onlineID->isFCALchannel(chid)) return true;

  return false;
}

