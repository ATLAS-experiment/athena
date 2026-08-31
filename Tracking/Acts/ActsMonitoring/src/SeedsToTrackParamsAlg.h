/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_SEEDSTOTRACKPARAMSALG_H
#define ACTSTRKANALYSIS_SEEDSTOTRACKPARAMSALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"

#include "ActsToolInterfaces/ITrackConverterTool.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"

#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackParametersContainer.h"
#include "ActsEvent/ContextUtility.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include <vector>
namespace ActsTrk {

  class SeedsToTrackParamsAlg final : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    PublicToolHandle<IGeometryRealmConvTool> m_geometryConvTool{this, "GeometryRealmConvTool", ""};
    ToolHandle<ActsTrk::ITrackParamsEstimationTool> m_paramEstimationTool{this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_detEleCollKey{this, "DetectorElementsKey", {}, "Key of input SiDetectorElementCollection"};

    SG::ReadHandleKey<ActsTrk::SeedContainer> m_inputSeedContainerKey{this, "InputSeedContainerKey", "", "Name of the input seed container"};
    SG::WriteHandleKey<ActsTrk::BoundTrackParametersContainer> m_outputTrackParamsCollectionKey{this, "OutputTrackParamsCollectionKey","", "Name of the output track parameters collection"};

    Gaudi::Property<bool> m_autoReverseSearch{this, "autoReverseSearch", false, "Whether to run the finding in seed parameter direction (false or not specified) or reverse direction (true), automatically determined by the param estimation tool"};
    Gaudi::Property<std::vector<double>> m_useTopSpRZboundary {this, "useTopSpRZboundary", {350. * Acts::UnitConstants::mm, 1060. * Acts::UnitConstants::mm}, "R/Z boundary for using the top space point in the track parameter estimation"};
    /** @brief Auxiliary class to access the magnetic field, geometry and calibration context */
    ContextUtility m_ctxProvider{this};
    bool shouldReverseSearch(const ActsTrk::Seed& seed) const;

  };

}

#endif
