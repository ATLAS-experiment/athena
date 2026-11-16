/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_TAU_TAU_JET_HELPERS_H
#define COLUMNAR_TAU_TAU_JET_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarTau/TauJetDef.h>
#include <ColumnarTau/TauTrackDef.h>

namespace columnar
{
  namespace TauJetHelpers
  {
    /// @brief an accessor for the number of charged tracks of a tau
    ///
    /// This is the equivalent of `xAOD::TauJet::nTracks()`, i.e. the
    /// number of tau tracks classified as charged. It uses, in order of
    /// preference:
    /// - the `nChargedTracks` detail variable, which tau reconstruction
    ///   sets to `nTracks()`
    /// - the `tauTrackLinks` together with `flagSet` on the linked tau
    ///   tracks, which is what `nTracks()` itself uses
    /// - `nTracks()` on the xAOD object, which only works in xAOD mode
    ///
    /// All of these columns are optional. Note that `nChargedTracks`
    /// can differ from `nTracks()` if tau tracks were thinned after
    /// reconstruction, as `nTracks()` skips invalid track links.
    ///
    /// @todo this always declares (and in columnar mode reads) the
    /// fallback columns, even if `nChargedTracks` is available

    template<ContainerIdConcept CI = TauJetDef,ContainerIdConcept CITrack = TauTrackDef,ColumnarModeConcept CM=ColumnarMode>
    class NChargedTracksAccessor final
    {
      ColumnAccessor<CI,int,CM> m_nChargedTracksAcc;
      ColumnAccessor<CITrack,ObjectColumn,CM> m_tauTracksAcc;
      ColumnAccessor<CI,std::vector<OptObjectId<CITrack,CM>>,CM> m_tauTrackLinksAcc;
      ColumnAccessor<CITrack,xAOD::TauTrack::TrackFlagType,CM> m_flagSetAcc;

    public:

      NChargedTracksAccessor (ColumnarTool<CM>& columnarTool)
        : m_nChargedTracksAcc (columnarTool, "nChargedTracks", {.isOptional = true}),
          m_tauTracksAcc (columnarTool, "TauTracks", {.isOptional = true}),
          m_tauTrackLinksAcc (columnarTool, "tauTrackLinks", {.isOptional = true}),
          m_flagSetAcc (columnarTool, "flagSet", {.isOptional = true})
      {}

      [[nodiscard]] int operator () (ObjectId<CI,CM> tau) const
      {
        if (m_nChargedTracksAcc.isAvailable (tau))
          return m_nChargedTracksAcc (tau);
        if (m_tauTrackLinksAcc.isAvailable (tau))
        {
          // same logic as xAOD::TauJet_v3::nTracksWithMask
          const xAOD::TauTrack::TrackFlagType mask = 1 << xAOD::TauJetParameters::classifiedCharged;
          int result = 0;
          bool flagSetAvailable = true;
          for (const auto& track : m_tauTrackLinksAcc (tau))
          {
            if (!track.has_value())
              continue;
            if (!m_flagSetAcc.isAvailable (track.value()))
            {
              flagSetAvailable = false;
              break;
            }
            if ((m_flagSetAcc (track.value()) & mask) == mask)
              ++ result;
          }
          if (flagSetAvailable)
            return result;
        }
        return tau.getXAODObject().nTracks();
      }
    };
  }
}

#endif
