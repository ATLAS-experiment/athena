/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_TRIGINDETTRACKSEEDINGTOOL_H
#define TRIGINDETPATTRECOTOOLS_TRIGINDETTRACKSEEDINGTOOL_H

#include "GaudiKernel/ToolHandle.h"

#include "TrigInDetToolInterfaces/ITrigInDetTrackSeedingTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
#include <vector>

#include "TrkSpacePoint/SpacePointContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

//for GPU offloading

// #include "TrigInDetR3AccelerationService/ITrigInDetR3AccelerationSvc.h"
// #include "TrigInDetAccelerationService/ITrigInDetAccelerationSvc.h"

#include "IRegionSelector/IRegSelTool.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

#include "GNNR3_FasTrackConnector.h"
#include "GNNR3_Geometry.h"
#include "GNNR3_DataStorage.h"

#include "SeedingToolBase.h"

class AtlasDetectorID;
class SCT_ID;
class PixelID;

class TrigInDetR3TrackSeedingTool:  public SeedingToolBase, public ITrigInDetTrackSeedingTool {
 public:

  // standard AlgTool methods
  TrigInDetR3TrackSeedingTool(const std::string&,const std::string&,const IInterface*);
  virtual ~TrigInDetR3TrackSeedingTool(){};
		
  // standard Athena methods
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  //concrete implementations
  virtual TrigInDetTrackSeedingResult findSeeds(const IRoiDescriptor&, std::vector<TrigInDetTracklet>&, const EventContext&) const override final;

 protected:

  void createGraphNodes(const SpacePointCollection*, std::vector<GNNR3_Node>&, std::vector<const Trk::SpacePoint*>&, unsigned short, float, float) const;

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  BooleanProperty m_usePixelSpacePoints{this,  "UsePixelSpacePoints", true};
  BooleanProperty m_useSctSpacePoints{this, "UseSctSpacePoints", false};
  
  //offline/EF containers
  SG::ReadHandleKey<SpacePointContainer> m_sctSpacePointsContainerKey{this, "SCT_SP_ContainerName", "ITkStripTrigSpacePoints"};
  SG::ReadHandleKey<SpacePointContainer> m_pixelSpacePointsContainerKey{this, "PixelSP_ContainerName", "ITkPixelTrigSpacePoints"};

  
  /// region selector tools
  ToolHandle<IRegSelTool> m_regsel_pix { this, "RegSelTool_Pixel",  "RegSelTool/RegSelTool_Pixel" };
  ToolHandle<IRegSelTool> m_regsel_sct { this, "RegSelTool_SCT",    "RegSelTool/RegSelTool_SCT"   };

  // for GPU offloading
  
  BooleanProperty m_useGPU{this, "UseGPU", false};
  //  ServiceHandle<ITrigInDetR3AccelerationSvc> m_accelSvc {this, "TrigAccelerationSvc", ""};
  
};
#endif
