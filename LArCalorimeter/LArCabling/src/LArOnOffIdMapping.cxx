/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloIdentifier/CaloCell_ID.h"

LArOnOffIdMapping::LArOnOffIdMapping(const LArOnlineID_Base* onlineId, const CaloCell_Base_ID * caloId) :

  m_onlineId(onlineId),
  m_caloId(caloId)

{
  IdentifierHash dummy,nCells;
  // hash-end of FCAL is also the hash-end of LAr
  m_caloId->calo_cell_hash_range(CaloCell_ID::LARFCAL, dummy,nCells); 
  unsigned nChan=m_onlineId->channelHashMax();
  //Note, there are more 'channels' (readouut) than phsical 'cells'. 
  //There are 12,604 disconnected channels, eg not connected to any physical cell 
  m_onlHashToOffline.assign(nChan,m_idEmpty);
  m_oflHashToOnline.assign(nCells,m_hwidEmpty);
  m_onlHashToOflHash.resize(nChan); //fills invalid default-constructed hash-value 
}


