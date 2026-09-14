/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TausRUsModel.h"

#include "tauRecTools/ConstituentsLoaderTauTrack.h"
#include "tauRecTools/TauGNNDataLoader.h"

#include "AthContainers/ConstAccessor.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackingPrimitives.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace {

using MomentType = xAOD::CaloCluster::MomentType;
using namespace TausRUsModel;

double deltaPhi(double phi1, double phi2) {
  double dphi = phi1 - phi2;
  while (dphi > M_PI) dphi -= 2 * M_PI;
  while (dphi <= -M_PI) dphi += 2 * M_PI;
  return dphi;
}

// --------------------------------------------------------------------------
// MARK: Cluster variables
// --------------------------------------------------------------------------

ClusterGetter momentGetter(MomentType moment) {
  return [moment](const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& cluster,
                  float& out) {
    out = static_cast<float>(cluster.clust().getMomentValue(moment));
    return true;
  };
}

std::vector<ClusterVariable> clusterVariables() {
  return {
    {"cls_dEta", [](const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& c, float& out) {
      out = c.clust().eta() - tau.eta(); return true; }},
    {"cls_dPhi", [](const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& c, float& out) {
      out = deltaPhi(c.clust().phi(), tau.phi()); return true; }},
    {"cls_ET", [](const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& c, float& out) {
      out = c.clust().et(); return true; }, {Transform::Log}},
    {"cls_E", [](const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& c, float& out) {
      out = c.clust().e(); return true; }, {Transform::Log}},
    {"cls_Eta", [](const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& c, float& out) {
      out = c.clust().eta(); return true; }},
    {"cls_Phi", [](const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& c, float& out) {
      out = c.clust().phi(); return true; }},
    {"cls_FIRST_ENG_DENS", momentGetter(MomentType::FIRST_ENG_DENS)},
    {"cls_SECOND_R",       momentGetter(MomentType::SECOND_R)},
    {"cls_EM_PROBABILITY", momentGetter(MomentType::EM_PROBABILITY)},
    {"cls_SECOND_LAMBDA",  momentGetter(MomentType::SECOND_LAMBDA)},
    {"cls_CENTER_LAMBDA",  momentGetter(MomentType::CENTER_LAMBDA)},
    {"cls_CENTER_MAG",     momentGetter(MomentType::CENTER_MAG)},
  };
}

// --------------------------------------------------------------------------
// MARK: Track variables
// --------------------------------------------------------------------------

/// Getter for an xAOD::SummaryType off the underlying TrackParticle.
TrackGetter summaryGetter(xAOD::SummaryType type) {
  return [type](const xAOD::TauJet&, const xAOD::TauTrack& track, float& out) {
    const xAOD::TrackParticle* trackParticle = track.track();
    if (!trackParticle) return false;
    uint8_t value = 0;
    if (!trackParticle->summaryValue(value, type)) return false;
    out = value;
    return true;
  };
}

/// Getter for a quantity read straight off the underlying TrackParticle.
TrackGetter trackParticleGetter(float (*extract)(const xAOD::TrackParticle&)) {
  return [extract](const xAOD::TauJet&, const xAOD::TauTrack& track, float& out) {
    const xAOD::TrackParticle* trackParticle = track.track();
    if (!trackParticle) return false;
    out = extract(*trackParticle);
    return true;
  };
}

std::vector<TrackVariable> trackVariables() {
  return {
    {"trk_dEta", [](const xAOD::TauJet& tau, const xAOD::TauTrack& t, float& out) {
      if (!t.track()) return false;
      out = t.track()->eta() - tau.eta();
      return true;
    }},
    {"trk_dPhi", [](const xAOD::TauJet& tau, const xAOD::TauTrack& t, float& out) {
      if (!t.track()) return false;
      out = deltaPhi(t.track()->phi(), tau.phi());
      return true;
    }},
    {"trk_pT",     trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.pt()); }),
                   {Transform::Log}},
    {"trk_E",      trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.e()); }),
                   {Transform::Log}},
    {"trk_Eta",    trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.eta()); })},
    {"trk_Phi",    trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.phi()); })},
    {"trk_charge", trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.charge()); })},
    {"trk_qOverP", trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.qOverP()); })},
    {"trk_d0",     trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.d0()); })},
    {"trk_z0",     trackParticleGetter([](const xAOD::TrackParticle& t) { return static_cast<float>(t.z0()); })},
    {"trk_z0sintheta", [](const xAOD::TauJet&, const xAOD::TauTrack& t, float& out) {
      out = t.z0sinthetaTJVA();
      return true;
    }},
    {"trk_nTRTHits",              summaryGetter(xAOD::numberOfTRTHits)},
    {"trk_nTRTHighThresholdHits", summaryGetter(xAOD::numberOfTRTHighThresholdHits)},
    {"trk_nSCTHits",              summaryGetter(xAOD::numberOfSCTHits)},
    {"trk_nPixelHits",            summaryGetter(xAOD::numberOfPixelHits)},
    {"trk_nBLayerHits",           summaryGetter(xAOD::numberOfInnermostPixelLayerHits)},
    {"trk_eProbNN", [](const xAOD::TauJet&, const xAOD::TauTrack& t, float& out) {
      static const SG::ConstAccessor<float> acc("eProbabilityNN");
      const xAOD::TrackParticle* trackParticle = t.track();
      if (!trackParticle || !acc.isAvailable(*trackParticle)) return false;
      out = acc(*trackParticle);
      return true;
    }},
  };
}

// --------------------------------------------------------------------------
// MARK: Vertex variables
// --------------------------------------------------------------------------

std::vector<VertexVariable> vertexVariables() {
  return {
    // A padded slot stays all zero; the model counts a vertex slot real if any of
    // its four features is nonzero
    {"Vertex_sumPt2", [](const xAOD::TauJet&, const xAOD::Vertex& v, float& out) {
      static const SG::ConstAccessor<float> acc("sumPt2");
      if (!acc.isAvailable(v)) return false;
      out = acc(v);
      return true; }},
    {"reco_tau_Vertex_x", [](const xAOD::TauJet&, const xAOD::Vertex& v, float& out) {
      out = v.x(); return true; }},
    {"reco_tau_Vertex_y", [](const xAOD::TauJet&, const xAOD::Vertex& v, float& out) {
      out = v.y(); return true; }},
    {"reco_tau_Vertex_z", [](const xAOD::TauJet&, const xAOD::Vertex& v, float& out) {
      out = v.z(); return true; }},
  };
}

// --------------------------------------------------------------------------
// MARK: Jet-level scalars
// --------------------------------------------------------------------------

std::vector<ScalarVariable> scalarVariables() {
  return {
    {"JetSeed_pT",  TauScalarVars::ptJetSeed},
    {"JetSeed_eta", TauScalarVars::etaJetSeed},
    {"JetSeed_phi", TauScalarVars::phiJetSeed},
    {"JetSeed_M",   TauScalarVars::mJetSeed},
  };
}

} // anonymous namespace

namespace TausRUsModel {

float Normalisation::apply(float raw) const {
  // Floor for the logarithms, so that a zero or negative raw value cannot
  // produce an inf or NaN that would silently poison the whole tensor.
  constexpr float floorValue = 1e-6f;
  float value = raw;
  switch (transform) {
    case Transform::Identity: break;
    case Transform::Log:    value = std::log(std::max(raw, floorValue)); break;
    case Transform::Log10:  value = std::log10(std::max(raw, floorValue)); break;
    case Transform::Log1p:  value = std::log1p(std::max(raw, -1.f + floorValue)); break;
    case Transform::Abs:    value = std::abs(raw); break;
    case Transform::AbsLog: value = std::log(std::max(std::abs(raw), floorValue)); break;
  }
  return scale * (value + offset);
}

// --------------------------------------------------------------------------
// The model's input and output nodes
// --------------------------------------------------------------------------

std::vector<Input> inputs() {
  std::vector<Input> result;

  Input clusters;
  clusters.name = "x";
  clusters.source = Source::Cluster;
  clusters.maxConstituents = 20;
  clusters.clusterVariables = clusterVariables();
  result.push_back(std::move(clusters));

  Input tracks;
  tracks.name = "tracks";
  tracks.source = Source::TauTrack;
  tracks.maxConstituents = 20;
  tracks.trackVariables = trackVariables();
  result.push_back(std::move(tracks));

  Input vertices;
  vertices.name = "vertex_collection";
  vertices.source = Source::Vertex;
  vertices.maxConstituents = 20;
  vertices.vertexVariables = vertexVariables();
  result.push_back(std::move(vertices));

  Input scalars;
  scalars.name = "seedjet_input";
  scalars.source = Source::Scalar;
  scalars.scalarVariables = scalarVariables();
  result.push_back(std::move(scalars));

  return result;
}

std::vector<Output> outputs() {
  std::vector<Output> result = {
    {"primary",               {1, 3}},
    {"decay_mode",            {1, 5}},
    {"tau_eta",               {1, 1}},
    {"charged_pion_eta",      {1, 1}},
    {"neutral_pion_eta",      {1, 1}},
    {"tau_phi",               {1, 2}},
    {"charged_pion_phi",      {1, 2}},
    {"neutral_pion_phi",      {1, 2}},
    {"tes",                   {1, 5}},
    {"charged_pion_pt",       {1, 5}},
    {"neutral_pion_pt",       {1, 5}},
    {"vertex_classification", {1, 20}},
    {"tautrack_class",        {1, 20, 4}},
  };
  for (Output& output : result) {
    output.size = std::accumulate(output.dims.begin(), output.dims.end(), size_t{1},
                                  std::multiplies<>());
  }
  return result;
}

const Output* findOutput(const std::vector<Output>& outputs, const std::string& name) {
  auto itr = std::find_if(outputs.begin(), outputs.end(),
                          [&name](const Output& output) { return output.name == name; });
  return itr == outputs.end() ? nullptr : &*itr;
}

} // namespace TausRUsModel
