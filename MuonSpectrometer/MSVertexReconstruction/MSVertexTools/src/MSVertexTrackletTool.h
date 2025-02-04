/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVERTEXTRACKLETTOOL_H
#define MSVERTEXTRACKLETTOOL_H

#include <utility>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"

#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "MSVertexToolInterfaces/IMSVertexTrackletTool.h"
#include "MSVertexUtils/Tracklet.h"
#include "MSVertexUtils/TrackletSegment.h"


namespace Muon {

    class MSVertexTrackletTool : virtual public IMSVertexTrackletTool, public AthAlgTool {
    public:
        MSVertexTrackletTool(const std::string& type, const std::string& name, const IInterface* parent);
        virtual ~MSVertexTrackletTool() = default;

        virtual StatusCode initialize() override;

        StatusCode findTracklets(std::vector<Tracklet>& tracklets, const EventContext& ctx) const override;

    private:
        SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_mdtTESKey{this, "mdtTES", "MDT_DriftCircles"};
        SG::WriteHandleKey<xAOD::TrackParticleContainer> m_TPContainer{this, "xAODTrackParticleContainer", "MSonlyTracklets"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        // hit properties
        // longitudinal (transverse) separation used in the barrel (endcaps)
        Gaudi::Property<double> m_d12_max{this, "d12_max", 50.*Gaudi::Units::millimeter, "max separation in mm between hits in tube 1 and 2"};
        Gaudi::Property<double> m_d13_max{this, "d13_max", 80.*Gaudi::Units::millimeter, "max separation in mm between hits in tube 1 and 3"};
        Gaudi::Property<double> m_errorCutOff{this, "errorCutOff", 0.001, "minimal hit error"};
        // tracklet segment properties
        Gaudi::Property<double> m_SeedResidual{this, "SeedResidual", 5., "max residual for tracklet seeds"};
        Gaudi::Property<double> m_minSegFinderChi2{this, "MinSegFinderChi2Prob", 0.05, "tracklet segment fitter chi^2 probability cut"};
        // tracklet properties
        Gaudi::Property<double> m_BarrelDeltaAlphaCut{this, "BarrelDeltaAlphaCut", 0.2*Gaudi::Units::radian, "maximum delta_alpha allowed in barrel MS chambers"};
        Gaudi::Property<double> m_EndcapDeltaAlphaCut{this, "EndcapDeltaAlphaCut", 0.015*Gaudi::Units::radian, "maximum delta_alpha allowed in the endcap MS chambers"};
        Gaudi::Property<double> m_maxDeltabCut{this, "maxDeltabCut", 3*Gaudi::Units::millimeter, "maximum delta_b allowed"};
        Gaudi::Property<double> m_minpTot{this, "minpTot", 800.*Gaudi::Units::MeV, "minimum measurable total momentum in MeV"};
        Gaudi::Property<double> m_maxpTot{this, "maxpTot", 10000.*Gaudi::Units::MeV, "maximum measurable total momentum in MeV beyond which tracklets are assumed to be straight"};
        Gaudi::Property<double> m_straightTrackletpTot{this, "straightTrackletpTot", 1.0e5*Gaudi::Units::MeV, "total momentum in MeV assigned to straight tracklets"};
        Gaudi::Property<double> m_straightTrackletInvPerr{this, "straightTrackletInvPerr", 5.0e-5/Gaudi::Units::MeV, "error in the inverse momentum in MeV^-1 assigned to straight tracklets"};
        Gaudi::Property<bool> m_tightTrackletRequirement{this, "tightTrackletRequirement", false, "tight tracklet requirement (affects efficiency - disabled by default)"};

        // MDT routines
        bool IgnoreMDTChamber(const Muon::MdtPrepData* mdtHit) const;
        int SortMDThits(std::vector<std::vector<const Muon::MdtPrepData*> >& SortedMdt, const EventContext& ctx) const;
        void addMDTHits(std::vector<const Muon::MdtPrepData*>& hits, std::vector<std::vector<const Muon::MdtPrepData*> >& SortedMdt) const;
        // single multilayer segment fitting, residuals, and cleaning
        std::vector<TrackletSegment> TrackletSegmentFitter(const std::vector<const Muon::MdtPrepData*>& mdts) const;
        std::vector<std::pair<double, double> > SegSeeds(const std::vector<const Muon::MdtPrepData*>& mdts) const;
        std::vector<TrackletSegment> TrackletSegmentFitterCore(const std::vector<const Muon::MdtPrepData*>& mdts, const std::vector<std::pair<double, double> >& SeedParams) const;
        static double SeedResiduals(const std::vector<const Muon::MdtPrepData*>& mdts, double slope, double inter) ;
        std::vector<TrackletSegment> CleanSegments(const std::vector<TrackletSegment>& segs) const;
        // Tracklet routines
        bool DeltabCalc(const TrackletSegment& ML1seg, const TrackletSegment& ML2seg) const;
        double TrackMomentum(const Identifier trkID, const double deltaAlpha) const ;
        double TrackMomentumError(const TrackletSegment& ml1, const TrackletSegment& ml2) const;
        double TrackMomentumError(const TrackletSegment& ml1) const;
        std::vector<Tracklet> ResolveAmbiguousTracklets(std::vector<Tracklet>& tracks) const;
        static void convertToTrackParticles(std::vector<Tracklet>& tracklets, SG::WriteHandle<xAOD::TrackParticleContainer>& container) ;
    };

}  // namespace Muon

#endif
