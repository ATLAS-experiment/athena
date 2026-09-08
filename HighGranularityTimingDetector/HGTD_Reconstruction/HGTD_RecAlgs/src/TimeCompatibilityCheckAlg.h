/**
* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
*
* @file HGTD_RecAlgs/TimeCompatibilityCheckAlg.h
* @author Valentina Raskina <valentina.raskina@cern.ch>
* @author
* @date July, 2022
*
* 
* @brief Reads the per-layer HGTD extension decorations produced by
* HGTD_IterativeExtensionTool and selects the subset of hits that are mutually
* compatible in time (delta-t cut for 2 hits, iterative chi2 outlier removal
* for 3-4 hits). The surviving times are stored as HGTD_times_of_compatible_hits.
*
* Also counts ITk holes between the last reconstructed ITk hit and the first
* HGTD layer via IHGTD_HolesITkTool::getHolesITk (implemented by
* HGTD_IterativeExtensionTool). ITk's own hole finder stops at the last hit on
* track and does not flag surfaces beyond it; this dedicated search recovers
* those holes for tracks that have compatible HGTD measurements, proving the
* particle reached the HGTD acceptance. The count is stored as HGTD_holes_in_ITk.
* TODO:
* 
*/

#ifndef HGTD_RECALGS_TIMECOMPATIBILITYCHECKALG_H
#define HGTD_RECALGS_TIMECOMPATIBILITYCHECKALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeneratorObjects/McEventCollection.h"
#include "HGTD_RecToolInterfaces/IHGTD_HolesITkTool.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include <string>


namespace HGTD {

class TimeCompatibilityCheckAlg : public AthReentrantAlgorithm {

public:
  TimeCompatibilityCheckAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TimeCompatibilityCheckAlg() {}
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  ToolHandle<IHGTD_HolesITkTool> m_extensionTool{this, "ITkHoles", "HGTD_IterativeExtensionTool/HGTD_IterativeExtensionTool", "Tool for calculating the holes in"};



  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{this, "TrackParticleContainerName", "InDetTrackParticles", "Name of the TrackParticle container"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerHasExtensionKey{this, "HGTD_has_extension", m_trackParticleContainerKey, "HGTD_has_extension", "deco with a handle for an extension"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerExtensionChi2Key{this, "HGTD_extension_chi2", m_trackParticleContainerKey, "HGTD_extension_chi2", "deco with a handle for a ch2 of extension"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterRawTimeKey{this, "HGTD_cluster_raw_time", m_trackParticleContainerKey, "HGTD_cluster_raw_time", "deco with a handle for layer cluster raw time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTimeKey{this, "HGTD_cluster_time", m_trackParticleContainerKey, "HGTD_cluster_time", "deco with a handle for cluster time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTruthClassKey{this, "HGTD_cluster_truth_class", m_trackParticleContainerKey,  "HGTD_cluster_truth_class", "deco with a handle for a truth time"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterShadowedKey{this, "HGTD_cluster_shadowed", m_trackParticleContainerKey, "HGTD_cluster_shadowed", "deco with a handle for a shadowed cluster"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterMergedKey{this, "HGTD_cluster_merged", m_trackParticleContainerKey, "HGTD_cluster_merged", "deco with a handle for a merged cluster"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_layerPrimaryExpectedKey{this, "HGTD_primary_expected", m_trackParticleContainerKey, "HGTD_primary_expected", "deco with a handle for an expected primary"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extrapXKey{this, "HGTD_extrap_x",m_trackParticleContainerKey, "HGTD_extrap_x", "deco with a handle for an x of extrap"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extrapYKey{this, "HGTD_extrap_y", m_trackParticleContainerKey, "HGTD_extrap_y", "deco with a handle for an y of extrap"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_holesHGTDKey{this, "HGTD_holes", m_trackParticleContainerKey,  "HGTD_holes", "deco with the holes on track in HGTD"};


  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_compatibleHitsTimesKey{this, "HGTD_times_of_compatible_hits",m_trackParticleContainerKey, "HGTD_times_of_compatible_hits", "deco with a handle for the time compatible hits' times"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_lastHitInITkCutKey{this, "HGTD_last_hit_in_ITk_cut", m_trackParticleContainerKey, "HGTD_last_hit_in_ITk_cut", "deco with a handle for the last hit to be close to HGTDrequirement"};
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_holesITkKey{this, "HGTD_holes_in_ITk", m_trackParticleContainerKey, "HGTD_holes_in_ITk", "deco with a handle for the number of holes on track in ITk between the last hit on track and HGTD"};

//TODO: the resolution is fixed to 0.035 ps, should add the resolution calculation after the irradiation

struct Hit {
    float m_time = 0;
    float m_resolution = 35 * Gaudi::Units::picosecond;
};

FloatProperty m_chi2_threshold{this, "Chi2Threshold", 1.5,
                        "Quality cut for decision to keep hits compatible in time"};

FloatProperty m_delta_cut{this, "DeltaTCut", 2.0,
                        "Upper limit for a cluster delta t cut"};

// using Hit_Vec_t = std::vector<Hit>:
// using Float_Vec_t = std::vector<float>;
float calculateChi2(const std::vector<Hit>& hits) const;

std::vector<TimeCompatibilityCheckAlg::Hit>
    getValidHits(const xAOD::TrackParticle* track_particle) const;

bool passesDeltaT(const std::vector<Hit>& hits) const;

std::vector<TimeCompatibilityCheckAlg::Hit>
    getTimeCompatibleHits(const xAOD::TrackParticle* track_particle) const;

bool lastHitIsOnLastSurface(const xAOD::TrackParticle& track_particle) const;

const Trk::TrackParameters* getLastHitOnTrack(const Trk::Track& track) const;

};

} //namespace HGTD

#endif // HGTD_RECALGS_TIMECOMPATIBILITYCHECKALG_H
