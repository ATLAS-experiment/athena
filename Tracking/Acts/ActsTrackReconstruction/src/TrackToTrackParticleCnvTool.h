/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVTOOL_H
#define ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVTOOL_H 1

#include "AthenaBaseComps/AthAlgTool.h"

#include "ActsToolInterfaces/ITrackToTrackParticleCnvTool.h"

#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"

#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Definitions/PdgParticle.hpp"
#include "xAODTracking/TrackingPrimitives.h"

#include "Gaudi/Property.h"

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
    using Stepper = Acts::EigenStepper<>;
    using Navigator = Acts::Navigator;
    using Propagator = Acts::Propagator<Stepper, Navigator>;

    Acts::BoundTrackParameters parametersAtPerigee(const EventContext& ctx,
                                                   const ActsTrk::TrackContainer::ConstTrackProxy& track,
                                                   const Acts::Surface& perigee_surface) const;

    ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool
       {this, "ExtrapolationTool", ""};

    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool
       {this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey
       {this, "AtlasFieldCacheCondObj", "fieldCondObj",
        "Name of the Magnetic Field conditions object key"};

    SG::ReadCondHandleKeyArray<InDetDD::SiDetectorElementCollection> m_siDetEleCollKey
       {this, "SiDetectorElementCollections", {},
        "Pixel and strip element collections to get geometry information about measurements."};

    Gaudi::Property<std::vector<unsigned int>> m_siDetEleCollToMeasurementType
       {this, "SiDetEleCollToMeasurementType", {},
        "One value per si detector collection: Pixel = 1, Strip = 2"};

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

    std::unique_ptr<Propagator> m_propagator;
  };

}

#endif
