/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef sTgcPREPDATA_p1_TRK_H
#define sTgcPREPDATA_p1_TRK_H

//-----------------------------------------------------------------------------
//
// file:   sTgcPrepData_p1.h
//
//-----------------------------------------------------------------------------
#include "AthenaPoolUtilities/TPObjRef.h"
#include "Identifier/IdentifierHash.h"

namespace Muon
{
  /** 
   * We don't write out (from Trk::PrepRawData)
   * m_indexAndHash (can be recomputed),
   */
  class sTgcPrepData_p1
  {
  public:
    sTgcPrepData_p1() = default;

    std::vector< signed char >  m_rdoList; //!< Store offsets

    /// @name Data from Trk::PrepRawData
    //@{
    float               m_locX{0}; //!< Equivalent to localPosition (locX) in the base class.
    float               m_errorMat{0}; //!< 1-d ErrorMatrix in the base class.
    //@}
    int                 m_charge{0};
    short int           m_time{0};
    uint16_t            m_bcBitMap{0};

    /// cluster quantities
    std::vector<uint16_t> m_stripNumbers;
    std::vector<short int> m_stripTimes;
    std::vector<int>      m_stripCharges;
    };
}

#endif 
