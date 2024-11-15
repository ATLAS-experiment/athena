/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRecConditions/LArHVNMap.h"

LArHVNMap::LArHVNMap(std::vector<short>& vVec, const LArHVLineID* hvHelper): 
   m_hvHelper(hvHelper),
   m_hvNcell(vVec),
   m_noCell(-1) {}

LArHVNMap::LArHVNMap(unsigned len, const LArHVLineID* hvHelper): 
   m_hvHelper(hvHelper),
   m_hvNcell(std::vector<short>(len,0)),
   m_noCell(-1) {}

// retrieving HVScaleCorr using offline ID  
short LArHVNMap::HVNcell(const HWIdentifier& id) const  {
  const IdentifierHash h=m_hvHelper-> hvlineHash(id);
  return HVNcell_Hash(h);
}

