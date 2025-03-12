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

#include "TrigInDetAccelerationService/ITrigInDetAccelerationSvc.h"

#include "IRegionSelector/IRegSelTool.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

#include "GNN_FasTrackConnector.h"
#include "GNN_Geometry.h"
#include "GNN_DataStorage.h"

#include "SeedingToolBase.h"

class AtlasDetectorID;
class SCT_ID;
class PixelID;

class TrigInDetTrackSeedingTool: public SeedingToolBase<const Trk::SpacePoint*>, public ITrigInDetTrackSeedingTool {
 public:

  // standard AlgTool methods
  TrigInDetTrackSeedingTool(const std::string&,const std::string&,const IInterface*);
  virtual ~TrigInDetTrackSeedingTool(){};
		
  // standard Athena methods
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  //concrete implementations
  virtual TrigInDetTrackSeedingResult findSeeds(const IRoiDescriptor&, std::vector<TrigInDetTracklet>&, const EventContext&) const override final;

 protected:

  void createGraphNodes(const SpacePointCollection*, std::vector<GNN_Node>&, unsigned short, float, float) const;

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
  ServiceHandle<ITrigInDetAccelerationSvc> m_accelSvc {this, "TrigAccelerationSvc", ""};
  
};
#endif
