/**
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/src/VertexTimeAlg.h
 *
 * @author Jernej Debevc <jernej.debevc@cern.ch>
 *
 * @brief Algorithm for setting the vertex time and resolution.
 */

#ifndef HGTD_VERTEXTIMEALG_H
#define HGTD_VERTEXTIMEALG_H

// Local includes
#include "Clustering.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

// Gaudi includes
#include "Gaudi/Property.h"
#include "GaudiKernel/SystemOfUnits.h"

// ROOT includes
#include "TMVA/Reader.h"

// Standard library includes
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace HGTD {

class VertexTimeAlg : public AthReentrantAlgorithm {

public:
  VertexTimeAlg(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;

private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_primVxCont_key {
    this, "PrimaryVertexContainer", "PrimaryVertices",
    "Name of the primary vertex container"
  };

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackCont_key {
    this, "TrackParticleContainer", "InDetTrackParticles",
    "Name of the track container"
  };

  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_vxHasTime_key{
    this, "VertexHasValidTime", m_primVxCont_key, "hasValidTime",
    "Specifies if the vertex has a valid time"
  };

  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_vxTime_key{
    this, "VertexTime", m_primVxCont_key, "time", "Time assigned to vertices"
  };

  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_vxTimeRes_key{
    this, "VertexTimeResolution", m_primVxCont_key, "timeResolution",
    "Time resolution assigned to vertices"
  };

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackValidTime_key{
    this, "TrackHasValidTime", m_trackCont_key, "hasValidTime",
    "Specifies if the track has a valid precision time"
  };

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackTime_key{
    this, "TrackTime", m_trackCont_key, "time", "Time assigned to tracks"
  };

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackTimeRes_key{
    this, "TrackTimeResolution", m_trackCont_key, "timeResolution",
    "Time resolution assigned to tracks"
  };

  Gaudi::Property<float> m_default_vxTime {
    this, "DefaultVertexTime", 0.0 * Gaudi::Units::ns,
    "The default time assigned to the vertex if no precision time can be calculated"
  };

  Gaudi::Property<float> m_default_vxTimeRes {
    this, "DefaultVertexTimeResolution", 50.0 / std::sqrt(12.0) * Gaudi::Units::ns,
    "The default time resolution assigned to the vertex if no precision time can be calculated"
  };

  Gaudi::Property<float> m_bdt_cutvalue {
    this, "BDTCutValue", 0.2f ,
    "The BDT output has to be above this value for the cluster to be accepted as HS"
  };

  std::vector<const xAOD::TrackParticle*> vertexAssociatedHGTDTracks(
    const xAOD::Vertex* vertex, const xAOD::TrackParticleContainer* tracks,
    double min_trk_pt) const;

  bool passTrackVertexAssociation(
    const xAOD::TrackParticle* track, const xAOD::Vertex* vertex,
    double min_trk_pt, const double significance_cut = 2.5) const;

  std::vector<HGTD::Cluster<const xAOD::TrackParticle*>> clusterTracksInTime(
    const EventContext& ctx,
    const std::vector<const xAOD::TrackParticle*>& tracks,
    double cluster_distance) const;

  HGTD::Cluster<const xAOD::TrackParticle*> getHScluster(
    const EventContext& ctx,
    const std::vector<HGTD::Cluster<const xAOD::TrackParticle*>>& clusters,
    const xAOD::Vertex* vertex) const;

  float scoreCluster(
    const EventContext& ctx,
    const HGTD::Cluster<const xAOD::TrackParticle*>& cluster,
    const xAOD::Vertex* vertex) const;

  std::pair<float, float> getZOfCluster(
    const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const;

  std::pair<float, float> getOneOverPOfCluster(
    const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const;

  std::pair<float, float> getDOfCluster(
    const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const;

  float getSumPt2OfCluster(
    const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const;

  struct HSclusterBDT {
    // Reader
    std::unique_ptr<TMVA::Reader> reader { };

    // BDT inputs
    float delta_z          { 0.0f };
    float z_sigma          { 0.0f };
    float q_over_p         { 0.0f };
    float q_over_p_sigma   { 0.0f };
    float d0               { 0.0f };
    float d0_sigma         { 0.0f };
    float delta_z_resunits { 0.0f };
    float cluster_sumpt2   { 0.0f };
  };

  mutable SG::SlotSpecificObj<HSclusterBDT> m_BDT ATLAS_THREAD_SAFE;

};

} // namespace HGTD

#endif // HGTD_VERTEXTIMEALG_H
