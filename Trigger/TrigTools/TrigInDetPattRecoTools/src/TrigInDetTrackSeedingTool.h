/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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


#include "IRegionSelector/IRegSelTool.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

#include "GNN_FasTrackConnector.h"
#include "GNN_Geometry.h"
#include "GNN_DataStorage.h"

class AtlasDetectorID;
class SCT_ID;
class PixelID;

class TrigInDetTrackSeedingTool: public extends<AthAlgTool, ITrigInDetTrackSeedingTool> {
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

  void createGraphNodes(const SpacePointCollection*, std::vector<TrigFTF_GNN_Node>&, unsigned short, float, float) const;

  std::pair<int, int> buildTheGraph(const IRoiDescriptor&, const std::unique_ptr<TrigFTF_GNN_DataStorage>&, std::vector<TrigFTF_GNN_Edge>&) const;

  int runCCA(int, std::vector<TrigFTF_GNN_Edge>&) const;
  
  ToolHandle<ITrigL2LayerNumberTool> m_layerNumberTool {this, "layerNumberTool", "TrigL2LayerNumberToolITk"};

  const AtlasDetectorID* m_atlasId = nullptr;
  const SCT_ID*  m_sctId = nullptr;
  const PixelID* m_pixelId = nullptr;

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  //offline/EF containers
  SG::ReadHandleKey<SpacePointContainer> m_sctSpacePointsContainerKey{this, "SCT_SP_ContainerName", "ITkStripTrigSpacePoints"};
  SG::ReadHandleKey<SpacePointContainer> m_pixelSpacePointsContainerKey{this, "PixelSP_ContainerName", "ITkPixelTrigSpacePoints"};

  BooleanProperty m_filter_phi{this, "DoPhiFiltering", true};
  BooleanProperty m_useBeamTilt{this, "UseBeamTilt", false};
  BooleanProperty m_usePixelSpacePoints{this,  "UsePixelSpacePoints", true};
  BooleanProperty m_useSctSpacePoints{this, "UseSctSpacePoints", false};
  BooleanProperty m_LRTmode{this, "LRTMode",false};
  BooleanProperty m_useML{this, "useML", true};

  UnsignedIntegerProperty m_nMaxPhiSlice{this, "nMaxPhiSlice", 53};
  BooleanProperty m_doubletFilterRZ{this, "Doublet_FilterRZ", true};
  BooleanProperty m_useEtaBinning{this, "UseEtaBinning", true};
  FloatProperty m_minPt{this, "pTmin", 1000.0};
  IntegerProperty m_nMaxEdges{this, "MaxGraphEdges", 2000000};
  StringProperty  m_connectionFile{this, "ConnectionFileName", "binTables_ITK_RUN4.txt"};

  float m_phiSliceWidth;
  
  /// region selector tools
  ToolHandle<IRegSelTool> m_regsel_pix { this, "RegSelTool_Pixel",  "RegSelTool/RegSelTool_Pixel" };
  ToolHandle<IRegSelTool> m_regsel_sct { this, "RegSelTool_SCT",    "RegSelTool/RegSelTool_SCT"   };

  std::unique_ptr<GNN_FasTrackConnector> m_connector = nullptr;
  std::vector<TrigInDetSiLayer> m_layerGeometry;
  std::unique_ptr<const TrigFTF_GNN_Geometry> m_geo = nullptr;

  
};
#endif
