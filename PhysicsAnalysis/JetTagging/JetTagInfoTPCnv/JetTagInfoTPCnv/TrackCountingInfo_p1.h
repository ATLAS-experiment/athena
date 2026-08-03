/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGINFOTPCNV_TRACKCOUNTINGINFO_P1_H
#define JETTAGINFOTPCNV_TRACKCOUNTINGINFO_P1_H

#include "JetTagInfoTPCnv/BaseTagInfo_p1.h"
#include "AthenaPoolUtilities/TPObjRef.h"

namespace Analysis { 

  class TrackCountingInfoCnv_p1;
  
  class TrackCountingInfo_p1 { 
    
    friend class TrackCountingInfoCnv_p1;
      
  private:
    /// Basic info
    TPObjRef m_baseTagInfo;

    /// All of this data will be written out.
    int m_ntrk = 0;
    float m_d0sig_2nd = 0;
    float m_d0sig_abs_2nd = 0;
    float m_d0sig_3rd = 0;
    float m_d0sig_abs_3rd = 0;
  
  }; // End class
  
} // End namespace
#endif
