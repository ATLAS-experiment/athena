/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
/**@class MuidTrackIsolation
 AlgTool for estimating the number, total charged momentum and most
 energetic inner detector tracks in a cone surrounding a muon

  @author Konstantinos.Nikolopoulos@cern.ch, Alan.Poppleton@cern.ch
*/
//////////////////////////////////////////////////////////////////////////////

#ifndef MUIDCALOISOLATIONTOOLS_MUIDTRACKISOLATION_H
#define MUIDCALOISOLATIONTOOLS_MUIDTRACKISOLATION_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "MuidInterfaces/IMuidTrackIsolation.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "TrkExInterfaces/IIntersector.h"


namespace Trk {
    class Surface;
}

namespace Rec {

    class MuidTrackIsolation : public extends<AthAlgTool, IMuidTrackIsolation> {
    public:
        using base_class::base_class;
        virtual ~MuidTrackIsolation() = default;  // destructor

        StatusCode initialize() override;

        /**IMuidTrackIsolation interface:
           get the number of tracks and summed momentum
           in a cone at the production vertex or around the muon calo intersect*/
        std::pair<int, double> trackIsolation(const EventContext& ctx, double eta, double phi) const override;

    private:
        // isolation without extrapolation to calo
        std::pair<int, double> trackVertex(const xAOD::TrackParticleContainer* indetTracks, double eta, double phi) const;

        // isolation performing extrapolation to calo
        std::pair<int, double> trackExtrapolated(const xAOD::TrackParticleContainer* indetTracks, double eta, double phi) const;

        double m_barrelCotTheta{};
        std::unique_ptr<const Trk::Surface> m_caloBackwardDisc;
        std::unique_ptr<const Trk::Surface> m_caloCylinder;
        std::unique_ptr<const Trk::Surface> m_caloForwardDisc;
        double m_etaSafetyFactor{0.1};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inDetTracksLocation{this, "InDetTracksLocation", "InDetTrackParticles", "ID tracks"};
       
        SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackLinkKey{this, "TrackLinkKey", m_inDetTracksLocation, "trackLink"};
        
        ToolHandle<Trk::IIntersector> m_intersector{this, "RungeKuttaIntersector", "Trk::RungeKuttaIntersector/RungeKuttaIntersector"};
        Gaudi::Property<double> m_minPt{this, "MinPt", 1.0 * Gaudi::Units::GeV};
        Gaudi::Property<double> m_trackCone{this, "TrackCone", 0.2};
        double m_trackCone2{0.};
        Gaudi::Property<bool> m_trackExtrapolation{this, "TrackExtrapolation", false};
    };

}  // namespace Rec

#endif  // MUIDCALOISOLATIONTOOLS_MUIDTRACKISOLATION_H
