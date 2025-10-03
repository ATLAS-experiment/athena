/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_SEEDSTOTRACKPARAMSALG_H
#define ACTSTRKANALYSIS_SEEDSTOTRACKPARAMSALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"

#include "ActsEvent/Seed.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackParametersContainer.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include <vector>
namespace ActsTrk {

  class SeedsToTrackParamsAlg final : public AthReentrantAlgorithm {
  public:
    SeedsToTrackParamsAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
    ToolHandle< ActsTrk::IActsToTrkConverterTool > m_ATLASConverterTool{this, "ATLASConverterTool", ""};
    ToolHandle<ActsTrk::ITrackParamsEstimationTool> m_paramEstimationTool{this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_detEleCollKey{this, "DetectorElementsKey", {}, "Key of input SiDetectorElementCollection"};

    SG::ReadHandleKey<ActsTrk::SeedContainer> m_inputSeedContainerKey{this, "InputSeedContainerKey", "", "Name of the input seed container"};
    SG::WriteHandleKey<ActsTrk::BoundTrackParametersContainer> m_outputTrackParamsCollectionKey{this, "OutputTrackParamsCollectionKey","", "Name of the output track parameters collection"};

    Gaudi::Property<bool> m_autoReverseSearch{this, "autoReverseSearch", false, "Whether to run the finding in seed parameter direction (false or not specified) or reverse direction (true), automatically determined by the param estimation tool"};
    Gaudi::Property<std::vector<double>> m_useTopSpRZboundary {this, "useTopSpRZboundary", {350. * Acts::UnitConstants::mm, 1060. * Acts::UnitConstants::mm}, "R/Z boundary for using the top space point in the track parameter estimation"};

    bool shouldReverseSearch(const ActsTrk::Seed& seed) const;

  };

}

#endif
