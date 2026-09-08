/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGINFOTPCNV_SLTrueInfo_P1_H
#define JETTAGINFOTPCNV_SLTrueInfo_P1_H

#include "EventPrimitives/EventPrimitives.h"

namespace Analysis {

  class SLTrueInfo_p1 {
  public:

    int m_barcode = 0;
    int m_pdgCode = 0;
    int m_pdgCodeMother = 0;
    bool m_isFromBhadron = false;
    bool m_isFromDhadron = false;
    bool m_isFromGHboson = false;
    
    
    Eigen::Vector3d m_Momentum;
    Eigen::Vector3d m_ProductionVertex;
  };

}
#endif
