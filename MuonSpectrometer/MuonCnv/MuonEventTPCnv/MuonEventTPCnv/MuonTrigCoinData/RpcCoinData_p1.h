/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONEVENTTPCNV_RPCOINDATA_P1_TRK_H
#define MUONEVENTTPCNV_RPCOINDATA_P1_TRK_H

//-----------------------------------------------------------------------------
//
// file:   RpcCoinData_p1.h
//
//-----------------------------------------------------------------------------
#include <vector>


namespace Muon
{
    /** Persistent representation of the transient Muon::RpcCoinData class.
    We don't write out (from Trk::PrepRawData) 
       * m_indexAndHash (can be recomputed), 
       * m_clusId (can be recomputed).
   For RpcCoinData_p1, I decided not to use MuonCoinDataCollection etc (as is used by TgcCoinData), as we can get better compression with MuonPRD_Container_p2 */
   class RpcCoinData_p1
   {
  public:
     RpcCoinData_p1() = default;
     // base
     
     /// @name Data from Trk::PrepRawData (minus m_indexAndHash and m_clusId, which are both recomputed)
     //@{
     float                          m_localPos{0.f}; //!< Equivalent to localPosition (locX) in the base class.
     float                          m_errorMat{0.f}; //!< 1-d ErrorMatrix in the base class.
     std::vector<short>             m_rdoList{}; //!< delta of Identifiers of RDOs used to make PRD
     //@}

     /// @name Data from Muon::RpcPrepData 
     //@{     
     float                          m_time{0.f};
     int                            m_ambiguityFlag{0};
     // m_triggerInfo is in RpcCoinData
     //@}
     
     /// @name Data from RpcCoinData
     unsigned short                 m_ijk{0};
     unsigned short                 m_threshold{0};
     unsigned short                 m_overlap{0};
     unsigned short                 m_parentCmId{0};
     unsigned short                 m_parentPadId{0};
     unsigned short                 m_parentSectorId{0};
     bool                           m_lowPtCm{false};  
     //@}
   };
}

#endif 
