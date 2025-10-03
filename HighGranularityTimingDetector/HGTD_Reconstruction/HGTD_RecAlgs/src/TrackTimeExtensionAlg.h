/**
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/TrackTimeExtensionAlg.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author
 * @date September, 2021
 *
 * @brief Calls the track extension tool and adds the output in addition to
 * truth information as decorations to the track particles.
 * In future iterations, the output of the extension can be a dedicated object
 * written to file, with the TrackParticle linking to its exension (if it has
 * one).
 *
 * TODO:
 * - add all relevant decorations
 * - access the truth information to classify extensions
 */

#ifndef HGTD_RECALGS_TRACKTIMEEXTENSIONALG_H
#define HGTD_RECALGS_TRACKTIMEEXTENSIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "GeneratorObjects/McEventCollection.h"
#include "HGTD_PrepRawData/HGTD_ClusterContainer.h"
#include "HGTD_RecToolInterfaces/IHGTD_ClusterTruthTool.h"
#include "HGTD_RecToolInterfaces/IHGTD_TrackTimeExtensionTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "InDetSimData/InDetSimDataCollection.h"

#include <string>

namespace HGTD {

class TrackTimeExtensionAlg : public AthReentrantAlgorithm {

public:
  TrackTimeExtensionAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrackTimeExtensionAlg() {}
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  struct DecorHandles
  {
    DecorHandles (const TrackTimeExtensionAlg& tool, const EventContext& ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerHasExtensionHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerExtensionChi2Handle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterRawTimeHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterTimeHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> layerClusterTruthClassHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerClusterShadowedHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerClusterMergedHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerPrimaryExpectedHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> extrapXHandle;
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> extrapYHandle;
  };
  friend struct DecorHandles;

  StatusCode decorateTrackParticle(DecorHandles& dh,
                                   const xAOD::TrackParticle* track_ptkl,
                                   const HGTD::ExtensionObject& extension,
                                   const InDetSimDataCollection* sdo_collection,
                                   const HepMC::GenEvent* hs_event,
                                   bool skip_deco = false) const;

  ToolHandle<IHGTD_TrackTimeExtensionTool> m_extension_tool{this, "TimeExtensionTool", "HGTD_IterativeExtensionTool/HGTD_IterativeExtensionTool", "Tool for extending ID tracks into the HGTD"};

  SG::ReadHandleKey<HGTD_ClusterContainer> m_clustercont_rh_key{this, "HGTDClusterContainerName", "HGTD_Cluster", "Name of the HGTD_Cluster container"};
  SG::ReadHandleKey<InDetSimDataCollection> m_sdo_coll_rh_key{this, "HGTD_SDOCollectionName", "HGTD_SDO_Map", "Name of the SDO container"};
  SG::ReadHandleKey<McEventCollection> m_mc_coll_rh_key{this, "MC_CollectionName", "TruthEvent", "Name of the McEventCollection containing the truth information"};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trk_ptkl_rh_key{this, "TrackParticleContainerName", "InDetTrackParticles", "Name of the TrackParticle container"};

  StringProperty m_deco_prefix{this, "DecorationPrefix", "HGTD", "Prefix to better identify decorations"};

  FloatProperty m_eta_cut{this, "MinEtaAcceptance", 2.38, "Minimum eta to consider a track for extension"};

  ToolHandle<IHGTD_ClusterTruthTool> m_truth_tool{this, "TruthTool", "ClusterTruthTool/ClusterTruthTool", "Tool for classifying HGTD clusters with truth information"};

  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerHasExtensionKey{this, "HGTD_has_extension", m_trk_ptkl_rh_key, "HGTD_has_extension", "deco with a handle for an extension"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerExtensionChi2Key{this, "HGTD_extension_chi2", m_trk_ptkl_rh_key, "HGTD_extension_chi2", "deco with a handle for a ch2 of extension"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterRawTimeKey{this, "HGTD_cluster_raw_time", m_trk_ptkl_rh_key, "HGTD_cluster_raw_time", "deco with a handle for layer cluster raw time"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTimeKey{this, "HGTD_cluster_time", m_trk_ptkl_rh_key, "HGTD_cluster_time", "deco with a handle for cluster time"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTruthClassKey{this, "HGTD_cluster_truth_class", m_trk_ptkl_rh_key, "HGTD_cluster_truth_class", "deco with a handle for a truth time"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterShadowedKey{this, "HGTD_cluster_shadowed", m_trk_ptkl_rh_key, "HGTD_cluster_shadowed", "deco with a handle for a shadowed cluster"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterMergedKey{this, "HGTD_cluster_merged", m_trk_ptkl_rh_key, "HGTD_cluster_merged", "deco with a handle for a merged cluster"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerPrimaryExpectedKey{this, "HGTD_primary_expected", m_trk_ptkl_rh_key, "HGTD_primary_expected", "deco with a handle for an expected primary"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapXKey{this, "HGTD_extrap_x", m_trk_ptkl_rh_key, "HGTD_extrap_x", "deco with a handle for an x of extrap"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapYKey{this, "HGTD_extrap_y", m_trk_ptkl_rh_key, "HGTD_extrap_y", "deco with a handle for an y of extrap"};

};

} // namespace HGTD

#endif // HGTD_RECALGS_TRACKTIMEEXTENSIONALG_H
