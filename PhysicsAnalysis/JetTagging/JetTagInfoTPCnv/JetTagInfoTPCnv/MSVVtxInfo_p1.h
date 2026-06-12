/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGINFOTPCNV_MSVVtxInfo_P1_H
#define JETTAGINFOTPCNV_MSVVtxInfo_P1_H


#include "AthenaPoolUtilities/TPObjRef.h"
#include "DataModelAthenaPool/ElementLinkVector_p1.h"

#include <string>

namespace Analysis {
  class MSVVtxInfoCnv_p1;

  class MSVVtxInfo_p1 {
    friend class MSVVtxInfoCnv_p1;

  private:

    /// All of this data will be written out.
    // Points to a Trk::RecVertex
    TPObjRef m_recsvx;

    float          m_masssvx = 0;
    float          m_efracsvx = 0;
    float          m_normdist = 0;

    // Points to SVTrackInfo list - which is just some element pointers.
    ElementLinkIntVector_p1 m_trackinfo;
  };
}

#endif
