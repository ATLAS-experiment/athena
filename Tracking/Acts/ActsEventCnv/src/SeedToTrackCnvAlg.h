/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSEVENTCNV_ACTSTRK_SEEDTOTRACKCNVALG_H
#define ACTSEVENTCNV_ACTSTRK_SEEDTOTRACKCNVALG_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/Seed.h"
#include "ActsEvent/TrackParameters.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

// STL includes
#include <string>




namespace ActsTrk {

/**
 * @class SeedToTrackCnvAlg
 * @brief 
 **/
class SeedToTrackCnvAlg : public AthReentrantAlgorithm {
public:
  SeedToTrackCnvAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SeedToTrackCnvAlg() override = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;

private:
  ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;
  SG::ReadHandleKey<ActsTrk::SeedContainer> m_seedContainerKey{this, "SeedContainerKey", {}, "Seed containers"};
  SG::ReadHandleKey<ActsTrk::BoundTrackParametersContainer> m_actsTrackParamsKey {this, "EstimatedTrackParametersKey", {}, "Track Parameters Key"};
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
  ToolHandle< ActsTrk::IActsToTrkConverterTool > m_ATLASConverterTool{this, "ATLASConverterTool", ""};
  SG::ReadCondHandleKey< InDetDD::SiDetectorElementCollection > m_detPixEleCollKey {this, "PixelDetectorElements", "", "Key of input SiDetectorElementCollection"};
  SG::ReadCondHandleKey< InDetDD::SiDetectorElementCollection > m_detStripEleCollKey {this, "StripDetectorElements", "", "Key of input SiDetectorElementCollection"};

};

} // namespace ActsTrk

#endif // ACTSEVENTCNV_ACTSTRK_SEEDTOTRACKCNVALG_H
