/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSCATTERINGANGLESIGNIFICANCETOOL_H
#define MUONSCATTERINGANGLESIGNIFICANCETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonCombinedToolInterfaces/IMuonScatteringAngleSignificance.h"
#include "TrkDetDescrInterfaces/ITrackingVolumesSvc.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkGeometry/TrackingVolume.h"


namespace Trk {
    class Volume;
}

namespace Rec {

    /** @class MuonScatteringAngleSignificanceTool
        @brief Tool to calculate the scattering angle significance from the detailed
               information (parameters, scattering angles) of a track fit.
      */

    class MuonScatteringAngleSignificanceTool : public extends<AthAlgTool, IMuonScatteringAngleSignificance> {
    public:
        using base_class::base_class;
        ~MuonScatteringAngleSignificanceTool() = default;

        StatusCode initialize();

        /** Calculate ScatteringAngleSignificance of a muon, stepping down to the relevant track */
        ScatteringAngleSignificance scatteringAngleSignificance(const xAOD::Muon& muon) const;

        /** Calculate ScatteringAngleSignificance of a track */
        ScatteringAngleSignificance scatteringAngleSignificance(const Trk::Track& track) const;

    private:
        /** does track have TrackParameters at every TSOS ? Method for
            compatibility with release < 17, where SlimmedTrack property
            isn't filled.  */
        bool isSlimmed(const Trk::Track& track) const;

        // tools and services
        ToolHandle<Trk::ITrackFitter> m_fitter{this, "TrackFitter", "",
                                               "tool for unslimming via track fit"};  //!< tool for unslimming via track fit
        ServiceHandle<Trk::ITrackingVolumesSvc> m_trackingVolumesSvc{this, "TrackingVolumesSvc", "Trk::TrackingVolumesSvc/TrackingVolumesSvc",
                                                                     "geometry for analysing track lengths"};

        // constants
        std::unique_ptr<const Trk::Volume> m_calorimeterVolume{nullptr};  //!< cache the calo volume pointer
        std::unique_ptr<const Trk::Volume> m_indetVolume{nullptr};        //!< cache the ID volume pointer

        // steering flags

      
        Gaudi::Property<bool> m_inDetOnly{this, "InDetOnly", true};       //!< scatterers from ID only (or ID + calo)
        Gaudi::Property<bool> m_refitInDetOnly{this, "RefitInDetOnly", true};  //!< steer if to unslim only ID
    };

}  // namespace Rec

#endif
