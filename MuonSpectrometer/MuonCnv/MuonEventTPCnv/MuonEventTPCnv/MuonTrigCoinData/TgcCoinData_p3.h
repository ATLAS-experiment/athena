/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCCOINDATA_P3_TRK_H
#define TGCCOINDATA_P3_TRK_H

//-----------------------------------------------------------------------------
//
// file:   TgcCoinData_p3.h
//
//-----------------------------------------------------------------------------
#include "AthenaPoolUtilities/TPObjRef.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"
#include "MuonTrigCoinData/TgcCoinData.h"

class TgcCoinDataCnv_p3;

namespace Muon
{
   class TgcCoinData_p3
   {
  public:
     TgcCoinData_p3() = default;
 
     Identifier32::value_type m_channelIdIn{};
     Identifier32::value_type  m_channelIdOut{};
     IdentifierHash m_collectionIdHash{};
     
     unsigned int  m_indexAndHash{0};
     
     int m_type{TgcCoinData::TYPE_UNKNOWN};     
     bool m_isAside{false};
     int m_phi{0};
     bool m_isInner{false};
     bool m_isForward{false};
     bool m_isStrip{false};
     int m_trackletId{0};
     int m_trackletIdStrip{0};

     TPObjRef m_posIn{};
     TPObjRef m_posOut{};
     TPObjRef m_errMat{};
     double m_widthIn{0.};
     double m_widthOut{0.};
     
     int m_delta{0};
     int m_roi{0};
     int m_pt{0};   
     bool m_veto{false};   

     int m_sub{0};
     int m_inner{0};
     bool m_isPositiveDeltaR{false};
   };
}

#endif 
