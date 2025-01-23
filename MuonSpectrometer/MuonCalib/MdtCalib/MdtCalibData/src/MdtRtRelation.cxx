/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/MdtRtRelation.h"
#include "MdtCalibData/TrRelationLookUp.h"

namespace MuonCalib {
    MdtRtRelation::MdtRtRelation(IRtRelationPtr rt, IRtResolutionPtr reso, ITrRelationPtr tr): 
        m_rt{std::move(rt)}, 
        m_rtRes{std::move(reso)}, 
        m_tr{std::move(tr)} {
      if (m_rt&& !m_tr) { 
        m_tr = std::make_unique<TrRelationLookUp>(*m_rt); 
      }    
    }
}  // end namespace MuonCalib
