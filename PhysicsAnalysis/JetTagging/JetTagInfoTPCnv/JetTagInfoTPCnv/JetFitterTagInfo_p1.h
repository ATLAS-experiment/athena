/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGINFOTPCNV_JETFITTERTAGINFO_P1_H
#define JETTAGINFOTPCNV_JETFITTERTAGINFO_P1_H

///
/// Persitent class for the jet fitter tag info class.
///
#include "JetTagInfoTPCnv/BaseTagInfo_p1.h"
#include "AthenaPoolUtilities/TPObjRef.h"

#include <string>

namespace Analysis {
  class JetFitterTagInfoCnv_p1;

  class JetFitterTagInfo_p1 {
    friend class JetFitterTagInfoCnv_p1;

  private:
    /// Basic info
    TPObjRef m_BaseTagInfo;

    /// All of this data will be written out.
    int m_nVTX = 0;
    int m_nSingleTracks = 0;
    int m_nTracksAtVtx = 0;
    float m_mass = 0;
    float m_energyFraction = 0;
    float m_significance3d = 0;
    float m_deltaeta = 0;
    float m_deltaphi = 0;
  };
}

#endif
