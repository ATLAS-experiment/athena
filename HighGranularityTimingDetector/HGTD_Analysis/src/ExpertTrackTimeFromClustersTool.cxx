/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/ExpertTrackTimeFromClustersTool.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date April, 2022
 * @brief
 */

#include "ExpertTrackTimeFromClustersTool.h"

#include <algorithm>

using namespace HGTD;

ExpertTrackTimeFromClustersTool::ExpertTrackTimeFromClustersTool(
    const std::string& t, const std::string& n, const IInterface* p)
    : base_class(t, n, p), m_dec_prefix(n) {
  // aux variable names may not contain a '.', but the instance name of a
  // private tool does
  std::replace(m_dec_prefix.begin(), m_dec_prefix.end(), '.', '_');
}

StatusCode ExpertTrackTimeFromClustersTool::initialize() {
  ATH_CHECK(AthAlgTool::initialize());

  m_dec_isset = std::make_unique<SG::AuxElement::Decorator<bool>>(
      m_dec_prefix + "_isset");
  m_dec_hastime = std::make_unique<SG::AuxElement::Decorator<bool>>(
      m_dec_prefix + "_hastime");
  m_dec_time = std::make_unique<SG::AuxElement::Decorator<float>>(
      m_dec_prefix + "_time");
  m_dec_nhits = std::make_unique<SG::AuxElement::Decorator<int>>(
      m_dec_prefix + "_nhits");
  m_dec_nprimehits = std::make_unique<SG::AuxElement::Decorator<int>>(
      m_dec_prefix + "_nprimehits");
  m_dec_resolution = std::make_unique<SG::AuxElement::Decorator<float>>(
      m_dec_prefix + "_resolution");

  m_acc_isset =
      std::make_unique<SG::ConstAccessor<bool>>(m_dec_prefix + "_isset");
  m_acc_hastime =
      std::make_unique<SG::ConstAccessor<bool>>(m_dec_prefix + "_hastime");
  m_acc_time =
      std::make_unique<SG::ConstAccessor<float>>(m_dec_prefix + "_time");
  m_acc_nhits =
      std::make_unique<SG::ConstAccessor<int>>(m_dec_prefix + "_nhits");
  m_acc_nprimehits =
      std::make_unique<SG::ConstAccessor<int>>(m_dec_prefix + "_nprimehits");
  m_acc_resolution =
      std::make_unique<SG::ConstAccessor<float>>(m_dec_prefix + "_resolution");

  return StatusCode::SUCCESS;
}

bool ExpertTrackTimeFromClustersTool::expertHasTime(
    const xAOD::TrackParticle& track_particle) const {
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

float ExpertTrackTimeFromClustersTool::expertTime(
    const xAOD::TrackParticle& track_particle) const {
  if (m_acc_time->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_time->operator()(track_particle);
  } else {
    throw std::runtime_error(
        "[ExpertTrackTimeFromClustersTool::expertTime] ERROR, always call "
        "expertHasTime on a track fist!");
  }
}

float ExpertTrackTimeFromClustersTool::expertTimeRes(
    const xAOD::TrackParticle& track_particle) const {
  if (m_acc_resolution->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_resolution->operator()(track_particle);
  } else {
    throw std::runtime_error(
        "[ExpertTrackTimeFromClustersTool::expertTimeRes] ERROR, always call "
        "expertHasTime on a track fist!");
  }
}

int ExpertTrackTimeFromClustersTool::nHits(
    const xAOD::TrackParticle& track_particle) const {
  if (m_acc_nhits->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_nhits->operator()(track_particle);
  } else {
    throw std::runtime_error(
        "[ExpertTrackTimeFromClustersTool::nHits] ERROR, always call "
        "expertHasTime on a track fist!");
  }
}

std::vector<ExpertTrackTimeFromClustersTool::Hit>
ExpertTrackTimeFromClustersTool::getTimeCompatibleHits(
    const xAOD::TrackParticle& track_particle) const {

  // get all available hits in a first step
  HitVec_t valid_hits = getValidHits(track_particle);

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

  HitVec_t time_candidates_copy = std::move(valid_hits); // TODO do I need this copy?
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

bool ExpertTrackTimeFromClustersTool::passesDeltaT(const HitVec_t& hits) const {
  // WARNING I don't check it here, but the vector has to be of size 2!!!
  // pass if the distance in units of the resolution passes the cut
  if (std::abs(hits.at(0).time - hits.at(1).time) <
      m_deltat_cut * std::hypot(hits.at(0).resolution, hits.at(1).resolution)) {
    return true;
  }
  return false;
}

float ExpertTrackTimeFromClustersTool::calculateChi2(const HitVec_t& hits) const {
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

float ExpertTrackTimeFromClustersTool::calculateMean(const HitVec_t& hits) const {
  // FIXME improve this
  if (hits.size() == 0) {
    return -999.;
  }
  float sum = 0.;
  for (const Hit& hit : hits) {
    sum += hit.time;
  }
  return sum / (float)hits.size();
}

std::vector<ExpertTrackTimeFromClustersTool::Hit>
ExpertTrackTimeFromClustersTool::getValidHits(
    const xAOD::TrackParticle& track_particle) const {

  const std::vector<float>& times = m_acc_perLayer_clusterTime(track_particle);
  const std::vector<bool>& has_clusters =
      m_acc_perLayer_hasCluster(track_particle);
  const std::vector<int>& hit_classification =
      m_acc_perLayer_clusterTruthClass(track_particle);

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

bool ExpertTrackTimeFromClustersTool::lastHitIsOnLastSurface(
    const xAOD::TrackParticle& track) const {

  TVector3 last_hit = this->getLastMeasurement(track);
  double radius = std::hypot(last_hit.X(), last_hit.Y());
  double abs_z = std::abs(last_hit.Z());

  // 21.9 numbers
  bool is_last = abs_z > 2700;
  is_last = is_last || (radius < 350 and abs_z > 2400);
  is_last = is_last || (radius > 205 and radius < 350 and abs_z > 2100);
  is_last = is_last || (radius < 220 and abs_z > 2200);
  is_last = is_last || (radius < 140 and abs_z > 1890);
  return is_last;
}

int ExpertTrackTimeFromClustersTool::numberOfPrimaryHits(
    const HitVec_t& hits) const {
  int n = 0;
  for (const Hit& hit : hits) {
    if (hit.isprime) {
      n++;
    }
  }
  return n;
}

int ExpertTrackTimeFromClustersTool::nPrimaryHits(
    const xAOD::TrackParticle& track_particle) const {
  if (m_acc_nprimehits->isAvailable(track_particle) and
      m_acc_isset->operator()(track_particle)) {
    return m_acc_nprimehits->operator()(track_particle);
  } else {
    return 0;
  }
}

float ExpertTrackTimeFromClustersTool::fracPrimaryHits(
    const xAOD::TrackParticle& track_particle) const {
  if (not expertHasTime(track_particle)) {
    ATH_MSG_WARNING("[ExpertTrackTimeFromClustersTool::fracPrimaryHits]"
                    "No available hits, returning -999.");
    return -999.;
  }
  int n_primaries = nPrimaryHits(track_particle);
  int n_assigned = nHits(track_particle);
  return (float)n_primaries / (float)n_assigned;
}

float ExpertTrackTimeFromClustersTool::calculateTrackResolution(
    const HitVec_t& hits) const {
  // should never happen
  if (hits.size() == 0) {
    return -999.;
  }
  float sum = 0;
  for (const Hit& hit : hits) {
    sum += 1. / (hit.resolution * hit.resolution);
  }
  if (sum == 0.)[[unlikely]]{
    return -999.;
  }
  return std::sqrt(1. / sum);
}

TVector3 ExpertTrackTimeFromClustersTool::getLastMeasurement(
    const xAOD::TrackParticle& track) const {

  unsigned int index = 0;

  if (not track.indexOfParameterAtPosition(index, xAOD::LastMeasurement)) {
    return TVector3(0, 0, 0);
  }

  return TVector3(track.parameterX(index), track.parameterY(index),
                  track.parameterZ(index));
}

int ExpertTrackTimeFromClustersTool::numberPotentialPrimaryHits(
    const xAOD::TrackParticle& track_particle) const {
  const std::vector<bool>& expected_hits =
      m_acc_perLayer_expectCluster(track_particle);
  return std::count(expected_hits.begin(), expected_hits.end(), true);
}
