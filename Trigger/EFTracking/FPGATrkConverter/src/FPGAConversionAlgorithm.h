/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrkConverter_CONVERSIONALGORITHM_H
#define FPGATrkConverter_CONVERSIONALGORITHM_H

#include "FPGAClusterConverter.h"
#include "FPGAActsTrkConverter.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"

#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitContainer.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "ActsEvent/ProtoTrackCollection.h"

class FPGAConversionAlgorithm : public AthReentrantAlgorithm {

public: 
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  /// Constructor with parameters:
  FPGAConversionAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  /// Destructor:
  virtual ~FPGAConversionAlgorithm() = default; 

  /// Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  
protected: // was private 

  ToolHandle<IFPGAClusterConverter> m_ClusterConverter{this, "ClusterConverter", "FPGAClusterConverter", "Converter Hits to InDetCluster"};
  ToolHandle<IFPGAActsTrkConverter> m_ActsTrkConverter{this, "ActsTrkConverter", "FPGAActsTrkConverter", "Make Acts Trks from converted clusters"};

  SG::ReadHandleKey<FPGATrackSimClusterCollection> m_FPGAClusterKey{this, "FPGATrackSimClusterKey","FPGAClusters","FPGATrackSim Clusters key"};
  SG::ReadHandleKey<FPGATrackSimClusterCollection> m_FPGASPKey{this, "FPGATrackSimSPKey","FPGASpacePoints_1st","FPGATrackSim Spacepoint key"};  
  SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey{this, "FPGATrackSimHitKey","FPGAHits","FPGATrackSim Hits key"};
  SG::ReadHandleKey<FPGATrackSimRoadCollection> m_FPGARoadKey{this, "FPGATrackSimRoadKey","FPGARoads","FPGATrackSim Roads key"};
  SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrackKey","FPGATracks","FPGATrackSim Tracks key"};
  SG::ReadHandleKey<FPGATrackSimHitContainer> m_FPGAHitInRoadsKey{this, "FPGATrackSimHitInRoadsKey","FPGAHitInRoads","FPGATrackSim Hit in Roads key"};

  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_xAODPixelClusterFromFPGAClusterKey{this, "xAODPixelClusterFromFPGAClusterKey","xAODPixelClustersFromFPGACluster","FPGA->xAOD PixelClusters Container"};
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_xAODStripClusterFromFPGAClusterKey{this, "xAODStripClusterFromFPGAClusterKey","xAODStripClustersFromFPGACluster","FPGA->xAOD StripClusters Container"};
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_xAODStripSpacePointFromFPGAKey{this, "xAODStripSpacePointFromFPGAKey","xAODStripSpacePointsFromFPGA","FPGA SP->xAOD Strip SpacePoint Container"};  
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_xAODPixelSpacePointFromFPGAKey{this, "xAODPixelSpacePointFromFPGAKey","xAODPixelSpacePointsFromFPGA","FPGA Cluster->xAOD Pixel SpacePoint Container"};  
  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_xAODPixelClusterFromFPGAHitKey{this, "xAODPixelClusterFromFPGAHitKey","xAODPixelClustersFromFPGAHit","FPGA->xAOD PixelClusters Container"};
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_xAODStripClusterFromFPGAHitKey{this, "xAODStripClusterFromFPGAHitKey","xAODStripClustersFromFPGAHit","FPGA->xAOD StripClusters Container"};
  SG::WriteHandleKey<ActsTrk::ProtoTrackCollection> m_ActsProtoTrackFromFPGARoadKey{this, "ActsProtoTrackFromFPGARoadKey","ActsProtoTracksFromFPGARoad","Vector of ActsTrk::ProtoTrack from FPGARoads"};
  SG::WriteHandleKey<ActsTrk::ProtoTrackCollection> m_ActsProtoTrackFromFPGATrackKey{this, "ActsProtoTrackFromFPGATrackKey","ActsProtoTracksFromFPGATrack","Vector of ActsTrk::ProtoTrack from FPGATracks"};

  template <typename T>
  StatusCode convertCollectionToContainer(Trk::PrepRawDataCollection< T >* inputCollection, SG::WriteHandleKey<Trk::PrepRawDataContainer<Trk::PrepRawDataCollection< T >>> &outputContainerKey);

  SG::WriteHandleKey<InDet::PixelClusterContainer> m_outputPixelClusterContainerKey {this, "FPGAOutputPixelClustersName", "FPGAInDetPixelClusterContainer", "name of the output FPGA InDet pixel cluster container"};
  SG::WriteHandleKey<InDet::SCT_ClusterContainer> m_outputStripClusterContainerKey {this, "FPGAOutputStripClustersName", "FPGAInDetStripsClusterContainer", "fname of the output FPGA InDet strip cluster container"};

  Gaudi::Property<bool> m_doClusters {this, "doClusters", true, "Convert FPGATrackSimCluster"};
  Gaudi::Property<bool> m_doHits {this, "doHits", false, "Convert FPGATrackSimHit"};
  Gaudi::Property<bool> m_doActsTrk {this, "doActsTrk", false, "Run Acts ProtoTrack finding"};
  Gaudi::Property<bool> m_doSP {this, "doSP", false, "Convert SPs"};
  Gaudi::Property<bool> m_doIndet {this, "doInDet", false, "Perform also to InDet convertion. This is obsolete and not needed anymore. By default disabled to save execution time"};
  Gaudi::Property<bool> m_useRoads {this, "useRoads", false, "If set to truth it will generate prototracks based on FPGA roads instead of FPGA tracks"};

  private:

  // chrono service
  ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};
}; 



#endif

