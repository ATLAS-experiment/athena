/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCOOLConditions/LArRampFlat.h" 
#include "LArIdentifier/LArOnlineID.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"

LArRampFlat::LArRampFlat()
  : LArCondFlatBase("LArRampFlat")
{}

LArRampFlat::~LArRampFlat()  {}


LArRampFlat::LArRampFlat(const CondAttrListCollection* attrList)
  : LArCondFlatBase("LArRampFlat")
{
  if (initializeBase().isFailure()) return;
 
  readBlob(attrList,msg());
}

LArRampFlat::RampRef_t LArRampFlat::ADC2DAC(const HWIdentifier& CellID, int gain) const 
{ 
  return ADC2DACByHash(m_onlineHelper->channel_Hash(CellID),gain);  
}

