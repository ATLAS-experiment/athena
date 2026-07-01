/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGINFOTPCNV_SECVTXINFO_P1_H
#define JETTAGINFOTPCNV_SECVTXINFO_P1_H

#include "CxxUtils/unused.h"

///
/// Cache all info having to do with the secondary vertex.
///

namespace Analysis
{
  class SecVtxInfo_p1
    {
      friend class SecVtxInfoCnv_p1;

    private:
      /// Base class info
      TPObjRef m_BaseTagInfo;

      /// Info stored in the SecVtxInfo subclass:

      // Unused, but shouldn't delete it since it's part of the persistent data.
      int ATH_UNUSED_MEMBER(m_numSelTracksForFit) = 0;
      float m_dist = 0;
      float m_rphidist = 0;
      float m_prob = 0;
      float m_mass = 0;
      float m_energyFraction = 0;
      int m_mult = 0;
      int m_NGood2TrackVertices = 0;

      /// Translates to enum FitTYpe.
      int m_fitType = 0;

      /// Translates to a Trk::RecVertex
      TPObjRef m_secVtxPos;

      /// Note that the TrackVec of fitted tracks is declared transient,
      /// and so isn't written out.
    };

}

#endif
