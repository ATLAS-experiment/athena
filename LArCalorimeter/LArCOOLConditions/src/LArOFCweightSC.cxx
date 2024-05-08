/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCOOLConditions/LArOFCweightSC.h"

LArOFCweightSC::LArOFCweightSC()
  : LArCondSuperCellBase ("LArOFCweightSC")
{}

LArOFCweightSC::LArOFCweightSC(const CondAttrListCollection* attrList)
  : LArCondSuperCellBase ("LArOFCweightSC")
{
  if (initializeBase().isFailure()) return;
 
  readBlob(attrList,"OFCbWeights",msg());

  if (m_pValues.size()!=1) {
    ATH_MSG_ERROR( "Found unexpected number of gains (" << m_pValues.size() <<"). Expected exactly one gain." );
  }
}


const float& LArOFCweightSC::getW(const HWIdentifier& hwid) const {
  const IdentifierHash hash=m_scOnlineID->channel_Hash(hwid);
  return this->getDataByHash(hash,0);
}
