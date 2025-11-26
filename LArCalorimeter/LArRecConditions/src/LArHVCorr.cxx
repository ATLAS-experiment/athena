/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRecConditions/LArHVCorr.h"

LArHVCorr::LArHVCorr(std::vector<float>&& vVec, const LArOnOffIdMapping* cabling, const CaloCell_Base_ID*  caloidhelper): 
   m_larCablingSvc(cabling),
   m_calo_id(caloidhelper),
   m_hvCorr(std::move(vVec)),
   m_noCorr(1.0) {}

// retrieving HVScaleCorr using offline ID  
const float& LArHVCorr::HVScaleCorr(const Identifier& id) const  {
  const IdentifierHash h=m_calo_id->calo_cell_hash(id);
  return HVScaleCorr_oflHash(h);
}

const float& LArHVCorr::HVScaleCorr(const HWIdentifier& id) const  {
   return HVScaleCorr(m_larCablingSvc->cnvToIdentifier(id));
}
