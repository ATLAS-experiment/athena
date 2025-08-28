/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DUMPEVENTDATATOJSONALG_H
#define DUMPEVENTDATATOJSONALG_H

// Core
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
//EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "TrkTrack/TrackCollection.h"
//Tools
#include "TrkExInterfaces/IExtrapolationEngine.h"
// PRDs
#include "MuonPrepRawData/CscPrepDataContainer.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
// ACTS
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
// Misc
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>


/** Algorithm to dump some Event data to JSON.
 */
class DumpEventDataToJsonAlg : public AthAlgorithm
{
public:
  /// Algorithm constructor
  DumpEventDataToJsonAlg(const std::string &name, ISvcLocator *pService);

  virtual ~DumpEventDataToJsonAlg() = default;

  /// inherited from Algorithm
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

protected:
  /** Dumps a dummy event with some objects at specific eta/phi coordinates for calibration.*/
  void prependTestEvent();

  template <class TYPE>
  StatusCode getAndFillArrayOfContainers(nlohmann::json &event, const SG::ReadHandleKeyArray<TYPE> &keys, const std::string &jsonType);

  template <class TYPE>
  StatusCode getAndFillContainer(nlohmann::json &event,
                                 const SG::ReadHandleKey<TYPE> &key,
                                 const std::string &jsonType);
  template <class TYPE>
  nlohmann::json getData(const TYPE &object);

  nlohmann::json getActsData(const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
                             const Acts::GeometryContext& gctx);


  template <class TYPE>
  void addLink(const TYPE &link, nlohmann::json &data);

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "Key for the Event Info"};

  SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_trackParticleKeys{this, "TrackParticleContainerKeys", {"InDetTrackParticles", "CombinedMuonTrackParticles"}, "Keys for TrackParticle Containers"};

  SG::ReadHandleKeyArray<xAOD::JetContainer> m_jetKeys{this, "JetContainerKeys", {"AntiKt4EMTopoJets"}, "Keys for Jet Containers"};

  SG::ReadHandleKeyArray<xAOD::MuonContainer> m_muonKeys{this, "MuonContainerKeys", {"Muons"}, "Keys for Muon Containers"};

  SG::ReadHandleKeyArray<xAOD::TauJetContainer> m_tauJetKeys{this, "TauJetContainerKeys", {"TauJets"}, "Keys for Tau Containers"};

  SG::ReadHandleKeyArray<xAOD::ElectronContainer> m_electronKeys{this, "ElectronContainerKeys", {"Electrons"}, "Keys for Electron Containers"};

  SG::ReadHandleKeyArray<xAOD::PhotonContainer> m_photonKeys{this, "PhotonContainerKeys", {"Photons"}, "Keys for Photon Containers"};

  SG::ReadHandleKeyArray<xAOD::CaloClusterContainer> m_caloClustersKeys{this, "CaloClusterContainerKeys", {"CaloCalTopoClusters"}, "Keys for CaloClusters Containers"};

  SG::ReadHandleKeyArray<CaloCellContainer> m_caloCellKey{this, "CaloCellContainerKey", {"AllCalo"}, "Key for CaloCell Container"};

  SG::ReadHandleKeyArray<TrackCollection> m_trackCollectionKeys{this, "TrackCollectionKeys", {"CombinedInDetTracks", "CombinedMuonTracks", "MuonSpectrometerTracks"}, "Keys for Track Containers"};
  
  // ACTS TrackContainer keys
  SG::ReadHandleKeyArray<ActsTrk::TrackContainer> m_trackContainerKeys{this, "TrackContainerKeys", {"ConvertedTracks"}, "Location of the converted Acts TrackContainer"};

  SG::ReadHandleKey<Muon::CscPrepDataContainer> m_cscPrepRawDataKey{this, "CscPrepRawDataKey", "CSC_Clusters", "Key for CSC PRD Container"};
  SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_mdtPrepRawDataKey{this, "MdtPrepRawDataKey", "MDT_DriftCircles", "Key for MDT PRD Container"};
  SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_rpcPrepRawDataKey{this, "RpcPrepRawDataKey", "RPC_Measurements", "Key for RPC PRD Container"};
  SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_tgcPrepRawDataKey{this, "TgcPrepRawDataKey", "TGC_MeasurementsAllBCs", "Key for TGC PRD Container"};
  SG::ReadHandleKey<Muon::MMPrepDataContainer> m_mmPrepRawDataKey{this, "MMPrepRawDataKey", "MM_Measurements", "Key for MM PRD Container"};
  SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_stgcPrepRawDataKey{this, "sTgcPrepRawDataKey", "STGC_Measurements", "Key for sTGC PRD Container"};
  SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixelPrepRawDataKey{this, "PixelPrepRawDataKey", "PixelClusters", "Key for Pixel PRD Container"};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_sctPrepRawDataKey{this, "SctPrepRawDataKey", "SCT_Clusters", "Key for SCT PRD Container"};
  SG::ReadHandleKey<InDet::TRT_DriftCircleContainer> m_trtPrepRawDataKey{this, "TrtPrepRawDataKey", "TRT_DriftCircles", "Key for TRT PRD Container"};

  ToolHandle<Trk::IExtrapolationEngine> m_extrapolator{this, "Extrapolator", "Trk::ExtrapolationEngine/AtlasExtrapolation"};

  SG::ReadHandleKey<ActsGeometryContext> m_geometryContextKey {
      this, "ActsAlignmentKey", "ActsAlignment", "cond read key for the alignment"};

  Gaudi::Property<std::string> m_outputJSON_Name{this, "OutputLocation", "EventData.json", "Default filename for "};

  Gaudi::Property<bool> m_dumpTestEvent{this, "DumpTestEvent", false, "If true, prepend a test event with some calibration data in it."};
  Gaudi::Property<bool> m_physlite{this, "PHYSLITE", false, "If true, do not try to access data which is not available in PHYSLITE files."};

  nlohmann::json m_eventData;
};
#endif
