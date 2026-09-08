/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVTOOL_H
#define ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVTOOL_H 1


#include "AthenaBaseComps/AthAlgTool.h"

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "ActsToolInterfaces/ITrackToTrackParticleCnvTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"

#include "ActsEvent/ContextUtility.h"


#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"

#include "Acts/Definitions/PdgParticle.hpp"
#include "xAODTracking/TrackingPrimitives.h"
#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"


namespace ActsTrk {

  class TrackToTrackParticleCnvTool : public extends<AthAlgTool, ITrackToTrackParticleCnvTool> {

  public:
    using base_class::base_class;

    virtual StatusCode initialize() override;

    virtual StatusCode convert(xAOD::TrackParticle& trackParticle,
                               const EventContext& ctx,
                               const ActsTrk::TrackContainer::ConstTrackProxy& track,
                               const Acts::Surface& perigeeSurface,
                               const InDet::BeamSpotData* beamspotData = nullptr) const override;

  private:
  
    Acts::BoundTrackParameters parametersAtPerigee(const EventContext& ctx,
                                                   const ActsTrk::TrackContainer::ConstTrackProxy& track,
                                                   const Acts::Surface& perigee_surface) const;

   ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};


   PublicToolHandle<MuonR4::ITrackSummaryTool> m_muonSummaryTool{this, "MuonSummaryTool", ""}; 

   /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
  ContextUtility m_ctxProvider{this};

   Gaudi::Property<double> m_paramExtrapolationParLimit
       {this, "ExtrapolationPathLimit", std::numeric_limits<double>::max(),
        "PathLimit for extrapolating track parameters."};

    Gaudi::Property<bool> m_firstAndLastParamOnly
       {this, "FirstAndLastParameterOnly", true,
        "Only convert the first and the last parameter."};

    Gaudi::Property<bool> m_computeExpectedLayerPattern
       {this, "ComputeExpectedLayerPattern", true,
        "Compute the expected layer pattern. CPU expensive"};

    Gaudi::Property<bool> m_expectIfPixelContributes
       {this, "expectIfPixelContribution", true,
        "Only expect pixel hits if there are pixel hits on track."};

    Gaudi::Property<double> m_pixelExpectLayerPathLimitInMM
       {this, "PixelExpectLayerPathLimitInMM", 1000,
        "PathLimit for extrapolating to get the expected pixel layer pattern in mm."};

    Gaudi::Property<unsigned long> m_patternRecognitionInfo
       {this, "PatternRecognitionInfo", (1ul << xAOD::SiSPSeededFinder),
        "Pattern recognition info bitmask to store on converted track particles."};

    Gaudi::Property<int> m_trackFitter
       {this, "TrackFitter", static_cast<int>(xAOD::KalmanFitter),
        "Track fitter identifier to store on converted track particles."};

    static constexpr int s_expertLevel = 200;
    Gaudi::Property<int> m_hgtdDecorationLevel
       {this, "HgtdDecorationLevel", false, "HGTD specific decorations: 0 = none, >0 time and hits, >=200 mean time, chi2." };
    Gaudi::Property<int> m_itkDecorationLevel
       {this, "ITkDecorationLevel", 1, ">=200 split counts for inclined and flat barrel." };

  };

}

#endif
