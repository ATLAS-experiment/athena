/**
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/src/VertexTimeAlg.cxx
 *
 * @author Jernej Debevc <jernej.debevc@cern.ch>
 *
 * @brief Algorithm for setting the vertex time and resolution.
 */

#include "VertexTimeAlg.h"

// Athena includes
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTracking/TrackingPrimitives.h"

// Standard library includes
#include <cmath>

namespace HGTD {

VertexTimeAlg::VertexTimeAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode VertexTimeAlg::initialize() {

  // Initialize keys for event data access
  ATH_CHECK(m_primVxCont_key.initialize());
  ATH_CHECK(m_trackCont_key.initialize());
  ATH_CHECK(m_vxHasTime_key.initialize());
  ATH_CHECK(m_vxTime_key.initialize());
  ATH_CHECK(m_vxTimeRes_key.initialize());
  ATH_CHECK(m_trackValidTime_key.initialize());
  ATH_CHECK(m_trackTime_key.initialize());
  ATH_CHECK(m_trackTimeRes_key.initialize());

  // Initialize the BDT
  for (auto& bdt : m_BDT) {

    bdt.reader = std::make_unique<TMVA::Reader>("!Color:!Silent");

    bdt.reader->AddVariable("m_delta_z",          &bdt.delta_z);
    bdt.reader->AddVariable("m_z_sigma",          &bdt.z_sigma);
    bdt.reader->AddVariable("m_q_over_p",         &bdt.q_over_p);
    bdt.reader->AddVariable("m_q_over_p_sigma",   &bdt.q_over_p_sigma);
    bdt.reader->AddVariable("m_delta_z_resunits", &bdt.delta_z_resunits);
    bdt.reader->AddVariable("m_cluster_sumpt2",   &bdt.cluster_sumpt2);
    bdt.reader->AddVariable("m_d0",               &bdt.d0);
    bdt.reader->AddVariable("m_d0_sigma",         &bdt.d0_sigma);

    const std::string weightFile {
      PathResolver::find_file("TMVA.VBFinv.mu200.Step3p1.8var.weights.xml", "DATAPATH")
    };

    bdt.reader->BookMVA("BDT", weightFile);
  }

  return StatusCode::SUCCESS;
}

StatusCode VertexTimeAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::VertexContainer> primVxCont(m_primVxCont_key, ctx);
  SG::ReadHandle<xAOD::TrackParticleContainer> trackCont(m_trackCont_key, ctx);

  SG::WriteDecorHandle<xAOD::VertexContainer, uint8_t> vxHasTime(
    m_vxHasTime_key, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vxTime(
    m_vxTime_key, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vxTimeRes(
    m_vxTimeRes_key, ctx);


  for (const auto& vx : *primVxCont) {

    if (vx->vertexType() == xAOD::VxType::VertexType::PriVtx) {

      std::vector<const xAOD::TrackParticle*> hgtdTracks {
        vertexAssociatedHGTDTracks(vx, trackCont.cptr(), 1.0 * Gaudi::Units::GeV)
      };

      std::vector<HGTD::Cluster<const xAOD::TrackParticle*>> trackClusters {
        clusterTracksInTime(ctx, hgtdTracks, 3.0)
      };

      // Get the cluster that was determined as HS by the BDT
      HGTD::Cluster<const xAOD::TrackParticle*> HS_cluster {
        getHScluster(ctx, trackClusters, vx)
      };

      if (HS_cluster.getEntries().size() > 0) {
        // HS cluster was found

        vxHasTime(*vx) = 1;
        vxTime(*vx) = HS_cluster.getValues().at(0);
        vxTimeRes(*vx) = HS_cluster.getSigmas().at(0);

      } else {
        // No cluster was chosen as HS

        vxHasTime(*vx) = 0;
        vxTime(*vx) = m_default_vxTime;
        vxTimeRes(*vx) = m_default_vxTimeRes;
      }

    } else {
      // Pileup vertices do not have a time

      vxHasTime(*vx) = 0;
      vxTime(*vx) = m_default_vxTime;
      vxTimeRes(*vx) = m_default_vxTimeRes;
    }
  }

  return StatusCode::SUCCESS;
}

std::vector<const xAOD::TrackParticle*>
VertexTimeAlg::vertexAssociatedHGTDTracks(
  const xAOD::Vertex* vertex, const xAOD::TrackParticleContainer* tracks,
  double min_trk_pt) const {

  std::vector<const xAOD::TrackParticle*> good_tracks { };

  for (const auto& trk : *tracks) {
    if (std::abs(trk->eta()) < 2.4 || std::abs(trk->eta()) > 4.0) {
      // Track is not within HGTD acceptance
      continue;
    }
    if (trk->pt() < 1.0 * Gaudi::Units::GeV) {
      continue;
    }
    if (passTrackVertexAssociation(trk, vertex, min_trk_pt)) {
      good_tracks.push_back(trk);
    }
  }

  return good_tracks;
}

bool VertexTimeAlg::passTrackVertexAssociation(
  const xAOD::TrackParticle* track, const xAOD::Vertex* vertex,
  double min_trk_pt, const double significance_cut) const {

  if (track->pt() < min_trk_pt || std::abs(track->eta()) > 4.0) {
    return false;
  }

  const double vx_z { vertex->z() };
  const double vx_z_variance { vertex->covariancePosition()(2, 2) };

  const double trk_z { track->vz() + track->z0() };
  const double trk_z_variance { track->definingParametersCovMatrix()(1, 1) };

  if (std::abs(trk_z - vx_z) / std::sqrt(trk_z_variance + vx_z_variance)
      < significance_cut) {
    return true;
  } else {
    return false;
  }
}

std::vector<HGTD::Cluster<const xAOD::TrackParticle*>>
VertexTimeAlg::clusterTracksInTime(
  const EventContext& ctx,
  const std::vector<const xAOD::TrackParticle*>& tracks,
  double cluster_distance) const {

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, uint8_t> trackValidTime(
    m_trackValidTime_key, ctx);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> trackTime(
    m_trackTime_key, ctx);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> trackTimeResolution(
    m_trackTimeRes_key, ctx);


  HGTD::ClusterCollection<const xAOD::TrackParticle*> collection { };

  for (const auto& track : tracks) {

    if (trackValidTime(*track)) {
      double time { trackTime(*track) };
      double timeResolution { trackTimeResolution(*track) };

      HGTD::Cluster<const xAOD::TrackParticle*> cluster(
        {time}, {timeResolution}, track);

      collection.addCluster(cluster);
    }
  }

  collection.updateDistanceCut(cluster_distance);
  collection.doClustering(HGTD::ClusterAlgo::Simultaneous);

  return collection.getClusters();
}

HGTD::Cluster<const xAOD::TrackParticle*> VertexTimeAlg::getHScluster(
  const EventContext& ctx,
  const std::vector<HGTD::Cluster<const xAOD::TrackParticle*>>& clusters,
  const xAOD::Vertex* vertex) const {

  HGTD::Cluster<const xAOD::TrackParticle*> HS_cluster { };
  float max_BDT_score { -1.0f };

  for (const auto& cluster : clusters) {

    // HS cluster has an additional requirement to have at least 3 tracks
    if (cluster.getEntries().size() < 3) {
      continue;
    }

    const float cluster_score { scoreCluster(ctx, cluster, vertex) };

    if (cluster_score > m_bdt_cutvalue && cluster_score > max_BDT_score) {

      max_BDT_score = cluster_score;
      HS_cluster = cluster;
    }
  }

  // If no cluster passes the HS criteria, the return value will be an empty
  // cluster
  return HS_cluster;
}

float VertexTimeAlg::scoreCluster(
  const EventContext& ctx,
  const HGTD::Cluster<const xAOD::TrackParticle*>& cluster,
  const xAOD::Vertex* vertex) const {

  std::pair<float, float> cluster_z { getZOfCluster(cluster) };
  std::pair<float, float> cluster_oneover_p { getOneOverPOfCluster(cluster) };
  std::pair<float, float> cluster_d0 { getDOfCluster(cluster) };
  double vertex_z_variance { vertex->covariancePosition()(2, 2) };

  auto& bdt { *m_BDT.get(ctx) };

  bdt.delta_z          = cluster_z.first - vertex->z();
  bdt.z_sigma          = cluster_z.second;
  bdt.q_over_p         = cluster_oneover_p.first;
  bdt.q_over_p_sigma   = cluster_oneover_p.second;
  bdt.d0               = cluster_d0.first;
  bdt.d0_sigma         = cluster_d0.second;
  bdt.cluster_sumpt2   = getSumPt2OfCluster(cluster);
  bdt.delta_z_resunits = (cluster_z.first - vertex->z()) /
    std::sqrt(std::pow(cluster_z.second, 2.0) + vertex_z_variance);

  return bdt.reader->EvaluateMVA("BDT");
}

std::pair<float, float> VertexTimeAlg::getZOfCluster(
  const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const {

  std::vector<const xAOD::TrackParticle*> tracks { cluster.getEntries() };

  float num { 0.0f };
  float denom { 0.0f };

  for (const auto& track : tracks) {

    float z0 { track->z0() };
    float z0_variance {
      static_cast<float>(track->definingParametersCovMatrix()(1, 1))
    };

    num += z0 / z0_variance;
    denom += 1.0f / z0_variance;
  }

  float avg_z0 = num / denom;
  float avg_z0_sigma = std::sqrt(1.0f / denom);

  return {avg_z0, avg_z0_sigma};
}

std::pair<float, float> VertexTimeAlg::getOneOverPOfCluster(
  const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const {

  std::vector<const xAOD::TrackParticle*> tracks { cluster.getEntries() };

  float num { 0.0f };
  float denom { 0.0f };

  for (const auto& track : tracks) {

    float one_over_p { std::abs(track->qOverP()) };
    float one_over_p_variance {
      static_cast<float>(track->definingParametersCovMatrix()(4, 4))
    };

    num += one_over_p / one_over_p_variance;
    denom += 1.0f / one_over_p_variance;
  }

  float avg_oneover_p = num / denom;
  float avg_oneover_p_sigma = std::sqrt(1.0f / denom);

  return {avg_oneover_p, avg_oneover_p_sigma};
}

std::pair<float, float> VertexTimeAlg::getDOfCluster(
  const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const {

  std::vector<const xAOD::TrackParticle*> tracks { cluster.getEntries() };

  float num { 0.0f };
  float denom { 0.0f };

  for (const auto& track : tracks) {

    float d0 { track->d0() };
    float d0_variance {
      static_cast<float>(track->definingParametersCovMatrix()(0, 0))
    };

    num += d0 / d0_variance;
    denom += 1.0f / d0_variance;
  }

  float avg_z0 = num / denom;
  float avg_z0_sigma = std::sqrt(1.0f / denom);

  return {avg_z0, avg_z0_sigma};
}

float VertexTimeAlg::getSumPt2OfCluster(
  const HGTD::Cluster<const xAOD::TrackParticle*>& cluster) const {

  std::vector<const xAOD::TrackParticle*> tracks { cluster.getEntries() };

  float sumpt2 { 0.0f };

  for (const auto& track : tracks) {
    sumpt2 += std::pow(track->pt(), 2.0);
  }

  return sumpt2;
}

} // namespace HGTD
