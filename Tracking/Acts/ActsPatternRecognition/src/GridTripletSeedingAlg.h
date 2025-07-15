/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GRIDTRIPLETSEEDINGALG_GRIDTRIPLETSEEDINGALG_H
#define ACTSTRK_GRIDTRIPLETSEEDINGALG_GRIDTRIPLETSEEDINGALG_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "ActsToolInterfaces/ISeedingTool.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "TrkSpacePoint/SpacePointContainer.h"

// Handle Keys
#include "ActsEvent/TrackParametersContainer.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

namespace ActsTrk {

class GridTripletSeedingAlg : public AthReentrantAlgorithm {

 public:
  GridTripletSeedingAlg(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode execute(const EventContext &ctx) const override;

 private:
  // Tool Handles
  ToolHandle<ActsTrk::ISeedingTool> m_seedsTool{this, "SeedTool", "",
                                                "Seed Tool"};
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "",
                                              "Monitoring tool"};

  // Handle Keys
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
      this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey{
      this, "AtlasFieldCacheCondObj", "fieldCondObj",
      "Name of the Magnetic Field conditions object key"};

  SG::ReadHandleKeyArray<xAOD::SpacePointContainer> m_spacePointKey{
      this, "InputSpacePoints", {}, "Input Space Points"};
  SG::WriteHandleKey<ActsTrk::SeedContainer> m_seedKey{this, "OutputSeeds", "",
                                                       "Output Seeds"};

  Gaudi::Property<bool> m_fastTracking{this, "useFastTracking", false};
  Gaudi::Property<bool> m_usePixel{this, "UsePixel", true};

 public:
  enum EStat { kNSpacepoints, kNSeeds, kNStat };

 private:
  mutable std::array<std::atomic<unsigned int>, kNStat> m_stat
      ATLAS_THREAD_SAFE{};
};

}  // namespace ActsTrk

#endif
