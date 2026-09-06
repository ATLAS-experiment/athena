/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCOOLConditions/LArNoiseFlat.h"
#include "LArIdentifier/LArOnlineID.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"


LArNoiseFlat::LArNoiseFlat()
  : LArCondFlatBase ("LArNoiseFlat")
{}

LArNoiseFlat::~LArNoiseFlat() {}


LArNoiseFlat::LArNoiseFlat(const CondAttrListCollection* attrList)
  : LArCondFlatBase ("LArNoiseFlat")
{
  if (initializeBase().isFailure()) return;
 
  readBlob(attrList,"Noise",msg());

  return;
}


const float& LArNoiseFlat::noise(const HWIdentifier& hwid, int gain) const {
  const IdentifierHash hash=m_onlineHelper->channel_Hash(hwid);
  return this->getDataByHash(hash, gain);
}

