/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/TrackTimeAccTool.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date April, 2022
 * @brief
 */

#include "TrackTimeAccTool.h"

#include <algorithm>
#include <numeric>

using namespace HGTD;

TrackTimeAccTool::TrackTimeAccTool(const std::string& t, const std::string& n,
                                   const IInterface* p)
    // : AthAlgTool(t, n, p),
    : base_class(t, n, p),
      // m_do_last_hit(true),
      // m_do_time_cons(true),
      // m_deltat_cut(2.0),
      // m_chi2_threshold(1.5),
      // m_do_min_nhits(true),
      // m_min_eta(3.5),
      // m_max_eta(3.9),
      // m_do_smearing(false),
      m_name(n) {
  // declareProperty("UseLastHitCut", m_do_last_hit);
  // declareProperty("UseTimeConsistency", m_do_time_cons);
  // declareProperty("DeltaTCut", m_deltat_cut);
  // declareProperty("TimeChi2Cut", m_chi2_threshold);
  // declareProperty("UseMinNHits", m_do_min_nhits);
  // declareProperty("MinEta", m_min_eta);
  // declareProperty("MaxEta", m_max_eta);
  // declareProperty("DoSmearing", m_do_smearing);
  std::replace(m_name.begin(), m_name.end(), '.', '_');
}

StatusCode TrackTimeAccTool::initialize() {
  StatusCode sc = AthAlgTool::initialize();

  m_dec_perLayer_hasCluster =
      std::make_unique<SG::AuxElement::Accessor<std::vector<bool>>>(
          "HGTD_has_extension");
  m_dec_perLayer_clusterChi2 =
      std::make_unique<SG::AuxElement::Accessor<std::vector<float>>>(
          "HGTD_extension_chi2");
  m_dec_perLayer_clusterDeltaT =
      std::make_unique<SG::AuxElement::Accessor<std::vector<float>>>(
          "HGTD_cluster_time");
  m_dec_perLayer_clusterTruthClassification =
      std::make_unique<SG::AuxElement::Accessor<std::vector<int>>>(
          "HGTD_cluster_truth_class");
  m_dec_perLayer_expectCluster =
      std::make_unique<SG::AuxElement::Accessor<std::vector<bool>>>(
          "HGTD_primary_expected");

  m_dec_isset =
      std::make_unique<SG::AuxElement::Decorator<bool>>(m_name + "_isset");
  m_dec_hastime =
      std::make_unique<SG::AuxElement::Decorator<bool>>(m_name + "_hastime");
  m_dec_time =
      std::make_unique<SG::AuxElement::Decorator<float>>(m_name + "_time");
  m_dec_nhits =
      std::make_unique<SG::AuxElement::Decorator<int>>(m_name + "_nhits");
  m_dec_nprimehits =
      std::make_unique<SG::AuxElement::Decorator<int>>(m_name + "_nprimehits");
  m_dec_resolution = std::make_unique<SG::AuxElement::Decorator<float>>(
      m_name + "_resolution");

  m_acc_isset =
      std::make_unique<SG::AuxElement::Accessor<bool>>(m_name + "_isset");
  m_acc_hastime =
      std::make_unique<SG::AuxElement::Accessor<bool>>(m_name + "_hastime");
  m_acc_time =
      std::make_unique<SG::AuxElement::Accessor<float>>(m_name + "_time");
  m_acc_nhits =
      std::make_unique<SG::AuxElement::Accessor<int>>(m_name + "_nhits");
  m_acc_nprimehits =
      std::make_unique<SG::AuxElement::Accessor<int>>(m_name + "_nprimehits");
  m_acc_resolution =
      std::make_unique<SG::AuxElement::Accessor<float>>(m_name + "_resolution");

  return sc;
}

bool TrackTimeAccTool::hasTime(const xAOD::TrackParticle& track_particle) {
  // if the track has been used before, access the decoration instead of
  // recalculating
  if (m_acc_hastime->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_hastime->operator()(track_particle);
  }

  // check if the last measurement is close to HGTD
  if (m_do_last_hit and not lastHitIsOnLastSurface(track_particle)) {
    m_dec_isset->set(track_particle, true);
    m_dec_hastime->set(track_particle, false);
    m_dec_time->set(track_particle, -999.);
    m_dec_nhits->set(track_particle, 0);
    m_dec_nprimehits->set(track_particle, 0);
    m_dec_resolution->set(track_particle, -999.);
    return false;
  }

  HitVec_t used_hits;
  if (m_do_time_cons) {
    // retrieve the hits that survive the preset cuts
    used_hits = getTimeCompatibleHits(track_particle);
  } else {
    // retrieve all hits, no time compatibility check was required
    used_hits = getValidHits(track_particle);
  }

  if (m_do_min_nhits and used_hits.size() == 1) {
    // if a 2 hit minimum is required, reject the case of a single associated
    // hit if the track falls into the defined eta region
    float fabs_eta = std::abs(track_particle.eta());
    if (fabs_eta > m_min_eta and fabs_eta < m_max_eta) {
      m_dec_isset->set(track_particle, true);
      m_dec_hastime->set(track_particle, false);
      m_dec_time->set(track_particle, -999.);
      m_dec_nhits->set(track_particle, 0);
      m_dec_nprimehits->set(track_particle, 0);
      m_dec_resolution->set(track_particle, -999.);
      return false;
    }
  }

  m_dec_isset->set(track_particle, true);
  m_dec_hastime->set(track_particle, used_hits.size() > 0);
  m_dec_time->set(track_particle, calculateMean(used_hits));
  m_dec_nhits->set(track_particle, used_hits.size());
  m_dec_nprimehits->set(track_particle, numberOfPrimaryHits(used_hits));
  m_dec_resolution->set(track_particle, calculateTrackResolution(used_hits));

  return used_hits.size() > 0;
}

float TrackTimeAccTool::time(const xAOD::TrackParticle& track_particle) {
  if (m_acc_time->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_time->operator()(track_particle);
  } else {
    throw std::runtime_error("[TrackTimeAccTool::time] ERROR, always call "
                             "hasTime on a track fist!");
  }
}

float TrackTimeAccTool::timeRes(const xAOD::TrackParticle& track_particle) {
  if (m_acc_resolution->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_resolution->operator()(track_particle);
  } else {
    throw std::runtime_error("[TrackTimeAccTool::timeRes] ERROR, always call "
                             "hasTime on a track fist!");
  }
}

int TrackTimeAccTool::nHits(const xAOD::TrackParticle& track_particle) {
  if (m_acc_nhits->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_nhits->operator()(track_particle);
  } else {
    throw std::runtime_error("[TrackTimeAccTool::nHits] ERROR, always call "
                             "hasTime on a track fist!");
  }
}

std::vector<TrackTimeAccTool::Hit> TrackTimeAccTool::getTimeCompatibleHits(
    const xAOD::TrackParticle& track_particle) {

  // get all available hits in a first step
  TrackTimeAccTool::HitVec_t valid_hits = getValidHits(track_particle);

  size_t vts = valid_hits.size();

  // if there is only one hit, no time consistency check can be done
  // to improve efficiency, accept it
  if (vts <= 1) {
    return valid_hits;
  }

  // in case of two hits, check for time compatibility
  if (vts == 2) {
    if (passesDeltaT(valid_hits)) {
      return valid_hits;
    } else {
      // if times are too far away from each other, don't accept time
      return {};
    }
  }
  // if there are 3 or 4 hits, perform chi2 outlier removal
  // calculate the chi2 value of the available hits in a first step
  float chi2 = calculateChi2(valid_hits);

  // if the chi2 value doesn't surpass the set threshold, the hits are accepted
  // as compatible in time
  if (chi2 < m_chi2_threshold) {
    return valid_hits;
  }

  HitVec_t time_candidates_copy = valid_hits; // TODO do I need this copy?
  bool searching = true;
  while (searching) {
    // calculate chi2 contribution of each value
    FloatVec_t chi2_contributions(time_candidates_copy.size(), 0.0);
    for (size_t i = 0; i < time_candidates_copy.size(); i++) {
      HitVec_t buff = time_candidates_copy;
      buff.erase(buff.begin() + i);

      // calculate the chi2 value we would get when removing the i-th hit
      double local_chi2 = calculateChi2(buff);

      chi2_contributions.at(i) = local_chi2;
    }
    // if removing one of the hits gives a much smaller chi2, it should be
    // removed, so find the position where the "local chi2" is the smallest, and
    // this is the hit that should be removed (since it gave a big
    // contribution)]

    // find minimum local chi2
    int position = std::distance(
        chi2_contributions.begin(),
        std::min_element(chi2_contributions.begin(), chi2_contributions.end()));

    // and remove it from the hits
    time_candidates_copy.erase(time_candidates_copy.begin() + position);

    // recompute chi2 value
    chi2 = calculateChi2(time_candidates_copy);

    // check for accepted chi2
    if (chi2 < m_chi2_threshold) {
      // if the threshold is now fulfilled, break out of the while loop
      searching = false;
    }
    // if everything except 2 values has been removed, check again for
    // consistency
    if (time_candidates_copy.size() == 2) {

      if (passesDeltaT(time_candidates_copy)) {
        return time_candidates_copy;
      } else {
        // if times are too far away, don't accept any TODO maybe accept one,
        // can the spatial chi2 be used?
        return {};
      }
    }
  }

  return time_candidates_copy;
}

bool TrackTimeAccTool::passesDeltaT(const HitVec_t& hits) {
  // WARNING I don't check it here, but the vector has to be of size 2!!!
  // pass if the distance in units of the resolution passes the cut
  if (std::abs(hits.at(0).time - hits.at(1).time) <
      m_deltat_cut * std::hypot(hits.at(0).resolution, hits.at(1).resolution)) {
    return true;
  }
  return false;
}

float TrackTimeAccTool::calculateChi2(const TrackTimeAccTool::HitVec_t& hits) {
  float mean = calculateMean(hits);

  float chi2 = 0.;
  for (size_t i = 0; i < hits.size(); i++) {

    chi2 += (hits.at(i).time - mean) * (hits.at(i).time - mean) /
            (hits.at(i).resolution * hits.at(i).resolution);
  }
  // TODO should I better use chi2/ndof, where ndof = hits.size() - 1 (due to
  // mean)?
  return chi2;
}

float TrackTimeAccTool::calculateMean(const TrackTimeAccTool::HitVec_t& hits) {
  // FIXME improve this
  if (hits.size() == 0) {
    return -999.;
  }
  float sum = 0.;
  for (const TrackTimeAccTool::Hit& hit : hits) {
    sum += hit.time;
  }
  return sum / (float)hits.size();
}

float TrackTimeAccTool::calculateMean(const std::vector<float>& vals) {
  float sum = std::accumulate(vals.begin(), vals.end(), 0.0);
  return sum / (float)vals.size();
}

std::vector<TrackTimeAccTool::Hit>
TrackTimeAccTool::getValidHits(const xAOD::TrackParticle& track_particle) {

  std::vector<float> times =
      m_dec_perLayer_clusterDeltaT->operator()(track_particle);
  std::vector<bool> has_clusters =
      m_dec_perLayer_hasCluster->operator()(track_particle);
  std::vector<int> hit_classification =
      m_dec_perLayer_clusterTruthClassification->operator()(track_particle);

  HitVec_t valid_hits;
  valid_hits.reserve(4);

  for (size_t i = 0; i < has_clusters.size(); i++) {
    if (not has_clusters.at(i)) {
      continue;
    }
    Hit newhit;
    newhit.time = times.at(i);
    newhit.resolution = 0.035; // nano seconds
    newhit.isprime = hit_classification.at(i) == 1;

    valid_hits.push_back(newhit);
  }
  return valid_hits;
}

bool TrackTimeAccTool::lastHitIsOnLastSurface(
    const xAOD::TrackParticle& track) {

  TVector3 last_hit = this->getLastMeasurement(track);
  double radius = std::hypot(last_hit.X(), last_hit.Y());
  double abs_z = std::abs(last_hit.Z());

  // 20.20 numbers
  // bool is_last = abs_z > 2700;
  // is_last = is_last || (radius < 350 and abs_z > 2400);
  // is_last = is_last || (radius > 205 and radius < 350 and abs_z > 2100);
  // is_last = is_last || (radius < 220 and abs_z > 2200);
  // is_last = is_last || (radius < 150 and abs_z > 2000);
  // 21.9 numbers
  bool is_last = abs_z > 2700;
  is_last = is_last || (radius < 350 and abs_z > 2400);
  is_last = is_last || (radius > 205 and radius < 350 and abs_z > 2100);
  is_last = is_last || (radius < 220 and abs_z > 2200);
  is_last = is_last || (radius < 140 and abs_z > 1890);
  return is_last;
}

int TrackTimeAccTool::numberOfPrimaryHits(
    const TrackTimeAccTool::HitVec_t& hits) {
  int n = 0;
  for (const TrackTimeAccTool::Hit& hit : hits) {
    if (hit.isprime) {
      n++;
    }
  }
  return n;
}

int TrackTimeAccTool::nPrimaryHits(const xAOD::TrackParticle& track_particle) {
  if (m_acc_nprimehits->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_nprimehits->operator()(track_particle);
  } else {
    return 0;
  }
}

float TrackTimeAccTool::fracPrimaryHits(
    const xAOD::TrackParticle& track_particle) {
  if (not hasTime(track_particle)) {
    ATH_MSG_WARNING("[TrackTimeAccTool::fracPrimaryHits]"
                    "No available hits, returning -999.");
    return -999.;
  }
  int n_primaries = nPrimaryHits(track_particle);
  int n_assigned = nHits(track_particle);
  return (float)n_primaries / (float)n_assigned;
}

float TrackTimeAccTool::calculateTrackResolution(
    const TrackTimeAccTool::HitVec_t& hits) {
  // should never happen
  if (hits.size() == 0) {
    return -999.;
  }
  float sum = 0;
  for (const TrackTimeAccTool::Hit& hit : hits) {
    sum += 1. / (hit.resolution * hit.resolution);
  }
  return std::sqrt(1. / sum);
}

TVector3
TrackTimeAccTool::getLastMeasurement(const xAOD::TrackParticle& track) {

  unsigned int index = 0;

  if (not track.indexOfParameterAtPosition(index, xAOD::LastMeasurement)) {
    return TVector3(0, 0, 0);
  }

  return TVector3(track.parameterX(index), track.parameterY(index),
                  track.parameterZ(index));
}

int TrackTimeAccTool::numberPotentialPrimaryHits(
    const xAOD::TrackParticle& track_particle) {
  if (not m_dec_perLayer_expectCluster->isAvailable(track_particle)) {
    ATH_MSG_WARNING("[TrackTimeAccTool::numberPotentialPrimaryHits]"
                    "Expected clusters not available, returning 0\n");
    return 0;
  }
  auto expected_hits = m_dec_perLayer_expectCluster->operator()(track_particle);
  return std::count(expected_hits.begin(), expected_hits.end(), true);
}
