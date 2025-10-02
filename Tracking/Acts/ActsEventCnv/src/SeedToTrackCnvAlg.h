/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSEVENTCNV_ACTSTRK_SEEDTOTRACKCNVALG_H
#define ACTSEVENTCNV_ACTSTRK_SEEDTOTRACKCNVALG_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/Seed.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParameters.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

#include "ActsEvent/TrackParametersContainer.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

// STL includes
#include <string>

namespace ActsTrk {

/**
 * @class SeedToTrackCnvAlg
 * @brief 
 **/
class SeedToTrackCnvAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~SeedToTrackCnvAlg() override = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;

private:
  detail::xAODUncalibMeasSurfAcc m_surfAcc{};
  ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};
  SG::ReadHandleKeyArray<ActsTrk::SeedContainer> m_seedContainerKey{this, "SeedContainerKey", {}, "Seed containers"};
  SG::ReadHandleKeyArray<ActsTrk::BoundTrackParametersContainer> m_actsTrackParamsKey {this, "EstimatedTrackParametersKey", {}, "Track Parameters Key"};
 
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
};

} // namespace ActsTrk

#endif // ACTSEVENTCNV_ACTSTRK_SEEDTOTRACKCNVALG_H
