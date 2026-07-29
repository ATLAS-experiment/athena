/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelNNMonitorAlg.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/Surface.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHit.h"
#include "InDetIdentifier/PixelID.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "TruthUtils/MagicNumbers.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <span>
#include <unordered_map>
#include <vector>

namespace {
  // Reproduce the on-track split decision applied to a number-net output: the
  // NnPixelClusterSplitProbTool posterior with its default uniform prior,
  // which reduces to probs[i]/sum, then the ambiguity score-processor
  // threshold (m_sharedProbCut2 = 0.3) that sets isSplit, which is what the
  // on-track cluster tool actually uses. Returns 1/2/3; invalid input
  // (a negative probability or non-positive sum) returns 1.
  int splitDecision(std::span<const double, 3> probs) {
    if (probs[0] < 0. || probs[1] < 0. || probs[2] < 0.) { return 1; }
    const double sum = probs[0] + probs[1] + probs[2];
    if (sum <= 0.) { return 1; }
    constexpr double cut = 0.3;                // m_sharedProbCut / m_sharedProbCut2
    if (probs[2] / sum >= cut) { return 3; }   // splitProbability(3)
    if (probs[1] / sum >= cut) { return 2; }   // splitProbability(2)
    return 1;
  }
}

namespace InDet {

PixelNNMonitorAlg::PixelNNMonitorAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode PixelNNMonitorAlg::initialize() {
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_pixelClusterKey.initialize());
  ATH_CHECK(m_trackCollectionKey.initialize());
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_siHitKey.initialize(m_doTruth));
  ATH_CHECK(m_splitProbKey.initialize(!m_splitProbKey.key().empty()));
  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_CHECK(m_nnFactory.retrieve());
  ATH_CHECK(m_lorentzTool.retrieve());
  return StatusCode::SUCCESS;
}


std::vector<Amg::Vector2D> PixelNNMonitorAlg::truthPositions(
    const InDet::PixelCluster& cluster,
    const InDetDD::SiDetectorElement& element,
    const std::vector<std::vector<const SiHit*>>& siHitsByHash) const {

  std::vector<Amg::Vector2D> result;
  const IdentifierHash hash = element.identifyHash();
  if (hash >= siHitsByHash.size()) { return result; }

  // One truth position per particle: average the mid-plane crossings of its
  // geometry-matched SiHits (within +/-1 pixel of a cluster cell).
  std::map<int, std::pair<Amg::Vector2D, int>> byParticle;
  for (const SiHit* sh : siHitsByHash[hash]) {
    HepGeom::Point3D<double> avg = sh->localStartPosition() + sh->localEndPosition();
    avg *= 0.5;
    Amg::Vector2D p = element.hitLocalToLocal(avg.z(), avg.y());
    InDetDD::SiCellId diode = element.cellIdOfPosition(p);
    if (!diode.isValid()) { continue; }
    bool match = false;
    for (const auto& rid : cluster.rdoList()) {
      if (std::abs(static_cast<int>(diode.etaIndex()) - m_pixelID->eta_index(rid)) <= 1 &&
          std::abs(static_cast<int>(diode.phiIndex()) - m_pixelID->phi_index(rid)) <= 1) {
        match = true; break;
      }
    }
    if (!match) { continue; }
    const int id = HepMC::uniqueID(sh->particleLink());
    // try_emplace with a zeroed vector: Amg::Vector2D (Eigen) is NOT
    // zero-initialised by default, so byParticle[id] would accumulate garbage.
    auto& e = byParticle.try_emplace(id, Amg::Vector2D::Zero(), 0).first->second;
    e.first += p; e.second += 1;
  }
  for (const auto& [id, e] : byParticle) {
    result.emplace_back(e.first / static_cast<double>(e.second));
  }
  return result;
}

StatusCode PixelNNMonitorAlg::fillHistograms(const EventContext& ctx) const {
  using namespace Monitored;

  auto numberGroup = getGroup("PixelNNNumber");
  auto posSummary  = getGroup("PixelNNPosSummary");
  auto splitGroup  = getGroup("PixelNNSplitFrac");

  SG::ReadHandle<InDet::PixelClusterContainer> pixelClusters(m_pixelClusterKey, ctx);
  if (!pixelClusters.isValid()) {
    ATH_MSG_WARNING("Could not retrieve PixelClusterContainer");
    return StatusCode::SUCCESS;
  }

  // SiHits indexed by wafer hash, for truth positions.
  std::vector<std::vector<const SiHit*>> siHitsByHash;
  if (m_doTruth && m_pixelID) {
    siHitsByHash.resize(m_pixelID->wafer_hash_max());
    SG::ReadHandle<SiHitCollection> siHits(m_siHitKey, ctx);
    if (siHits.isValid()) {
      for (const SiHit& h : *siHits) {
        if (!h.isPixel()) { continue; }
        Identifier wid = m_pixelID->wafer_id(h.getBarrelEndcap(), h.getLayerDisk(),
                                             h.getPhiModule(), h.getEtaModule());
        IdentifierHash wh = m_pixelID->wafer_hash(wid);
        if (wh < m_pixelID->wafer_hash_max()) siHitsByHash[wh].push_back(&h);
      }
    } else {
      ATH_MSG_DEBUG("SiHit collection not available; truth plots disabled this event");
    }
  }

  // On-track clusters -> track parameters + surface (POSITION-net input only).
  // A cluster may carry several tracks (one per particle in a merged cluster);
  // each track is evaluated separately with its own incidence angle.
  struct OnTrackInfo { const Trk::TrackParameters* params; const Trk::Surface* surface; };
  std::unordered_map<Identifier::value_type, std::vector<OnTrackInfo>> onTrackClusters;
  SG::ReadHandle<TrackCollection> tracks(m_trackCollectionKey, ctx);
  if (tracks.isValid()) {
    for (const Trk::Track* track : *tracks) {
      if (!track) { continue; }
      for (const auto* tsos : *track->trackStateOnSurfaces()) {
        if (!tsos || !tsos->type(Trk::TrackStateOnSurface::Measurement)) { continue; }
        const auto* rio = dynamic_cast<const Trk::RIO_OnTrack*>(tsos->measurementOnTrack());
        if (!rio || !rio->prepRawData()) { continue; }
        const auto* pixClus = dynamic_cast<const InDet::PixelCluster*>(rio->prepRawData());
        if (!pixClus || !tsos->trackParameters()) { continue; }
        onTrackClusters[pixClus->identify().get_compact()].push_back(
          {tsos->trackParameters(), &rio->associatedSurface()});
      }
    }
  }

  // Final reco split decision (isSplit, set by the ambiguity solver), for the
  // reco curve of the split-fraction profiles.
  const Trk::ClusterSplitProbabilityContainer* splitProbs = nullptr;
  if (!m_splitProbKey.key().empty()) {
    SG::ReadHandle<Trk::ClusterSplitProbabilityContainer> h(m_splitProbKey, ctx);
    if (h.isValid()) splitProbs = h.cptr();
  }

  int nClusters = 0;

  // Per-event extremes of the position-net outputs (filled for every evaluated
  // on-track cluster, independent of truth, so they also monitor data).
  double evtMinErrX = std::numeric_limits<double>::max();
  double evtMaxErrX = 0.;
  double evtMinErrY = std::numeric_limits<double>::max();
  double evtMaxErrY = 0.;
  double evtMaxAbsDeltaX = 0.;
  double evtMaxAbsDeltaY = 0.;
  double evtMaxProb2 = 0.;
  bool evtHasPos = false;

  for (const auto* coll : *pixelClusters) {
    if (!coll) { continue; }
    for (const auto* cluster : *coll) {
      if (!cluster) { continue; }
      const InDetDD::SiDetectorElement* element = cluster->detectorElement();
      if (!element) { continue; }
      ++nClusters;

      const double eta = cluster->globalPosition().eta();
      const int nCell  = static_cast<int>(cluster->rdoList().size());

      // True particle positions and multiplicity.
      std::vector<Amg::Vector2D> truths;
      if (m_doTruth && !siHitsByHash.empty()) {
        truths = truthPositions(*cluster, *element, siHitsByHash);
      }
      const int trueN = static_cast<int>(truths.size());
      const bool haveTruth = (trueN >= 1 && trueN <= 3);

      // The with-track NN is only exercised for clusters on a track: the
      // ambiguity solver re-evaluates the split prob with the track, and the
      // on-track tool makes the position measurement. Off-track clusters are
      // not part of this comparison.
      auto it = onTrackClusters.find(cluster->identify().get_compact());
      if (it == onTrackClusters.end() || it->second.empty()) { continue; }

      // ---- Number net WITH track (as in the ambiguity solver). The predicted
      //      multiplicity is the reco split decision (with-track split prob +
      //      ambiguity map) = numberOfSubclusters, i.e. exactly what the
      //      on-track position call uses. ----
      const Trk::Surface* surf0      = it->second.front().surface;
      const Trk::TrackParameters* tp0 = it->second.front().params;
      std::vector<double> probs =
        m_nnFactory->estimateNumberOfParticles(*cluster, *surf0, *tp0);
      if (probs.size() < 3) { continue; }

      // Predicted multiplicity = the reco split decision applied to THIS
      // (ONNX) number net's output, so the whole chain is the model under test.
      const int predN = splitDecision(std::span<const double, 3>(probs.data(), 3));

      // Reconstructed TRACK incidence angles fed to the with-track NN
      // (reproduces addTrackInfoToInput). Written to the per-cluster track-angle
      // dump so the offline eval can use the real track angle.
      // Reference path length used by NnClusterizationFactory::addTrackInfoToInput.
      constexpr double refPathLength = 0.250;
      const Amg::Vector3D particleDir = tp0->momentum().unit();
      Amg::Vector3D localIntersection = surf0->transform().inverse().linear() * particleDir;
      const double cosTheta = std::cos(localIntersection.theta());
      // Direction in the module plane: no well-defined incidence angle.
      if (std::abs(cosTheta) < 1e-6) { continue; }
      localIntersection *= refPathLength / cosTheta;
      const double trkTheta = std::atan2(localIntersection.y(), refPathLength);
      double trkPhi = std::atan2(localIntersection.x(), refPathLength);
      const double tanl = m_lorentzTool->getTanLorentzAngle(element->identifyHash(), ctx);
      trkPhi = std::atan(std::tan(trkPhi) - tanl);

      evtMaxProb2 = std::max(evtMaxProb2, probs[1]);
      auto monPredN = Scalar<int>  ("predN", predN);
      auto monProb1 = Scalar<float>("prob1", probs[0]);
      auto monProb2 = Scalar<float>("prob2", probs[1]);
      auto monProb3 = Scalar<float>("prob3", probs[2]);
      fill(numberGroup, monPredN, monProb1, monProb2, monProb3);

      // ---- Split fraction vs track kinematics: NN (network argmax >= 2) and
      //      reco (ambiguity-solver isSplit) on every on-track cluster, so
      //      these also fill on data; truth (trueN >= 2) on MC only. ----
      double leadPt = 0.;
      for (const OnTrackInfo& t : it->second)
        if (t.params) leadPt = std::max(leadPt, t.params->momentum().perp());
      const int nnArgmax = 1 + static_cast<int>(
        std::max_element(probs.begin(), probs.begin() + 3) - probs.begin());

      auto monPt      = Scalar<float>("trackPt",    leadPt / Gaudi::Units::GeV);
      auto monPhi     = Scalar<float>("trkPhi",     trkPhi);
      auto monTheta   = Scalar<float>("trkTheta",   trkTheta);
      auto monClusEta = Scalar<float>("clusEta",    eta);
      auto monNn      = Scalar<int>  ("nnSplit",    nnArgmax >= 2 ? 1 : 0);
      fill(splitGroup, monPt, monPhi, monTheta, monClusEta, monNn);
      if (splitProbs) {
        const auto& sp = splitProbs->splitProbability(cluster);
        auto monReco = Scalar<int>("recoSplit", sp.isSplit() ? 1 : 0);
        fill(splitGroup, monPt, monPhi, monTheta, monClusEta, monReco);
      }
      if (haveTruth) {
        auto monTruth = Scalar<int>("truthSplit", trueN >= 2 ? 1 : 0);
        fill(splitGroup, monPt, monPhi, monTheta, monClusEta, monTruth);
      }

      if (haveTruth) {
        auto monTrueN     = Scalar<int>  ("trueN",     trueN);
        auto monPredNc    = Scalar<int>  ("predNconf", predN);
        auto monIsCorrect = Scalar<int>  ("isCorrect", trueN == predN ? 1 : 0);
        auto monEta       = Scalar<float>("eta",       eta);
        auto monNCell     = Scalar<int>  ("nCell",     nCell);
        auto monProbMulti = Scalar<float>("probMulti", probs[1] + probs[2]);
        fill(numberGroup, monTrueN, monPredNc, monIsCorrect, monEta, monNCell, monProbMulti);
      }


      // ---- Position net WITH track at the predicted multiplicity. The
      //      wide-range and extreme-value monitoring fills for every evaluated
      //      cluster (truth-free, so also on data); the residuals and pulls
      //      additionally require the multiplicity to be predicted correctly
      //      (predN == trueN) on simulation. ----
      const int numberOfSubclusters = predN;

      auto posDQ = getGroup("PixelNNPosDQ");
      auto posGroup = getGroup("PixelNNPosN" + std::to_string(numberOfSubclusters));
      for (const OnTrackInfo& trk : it->second) {
        if (!trk.params || !trk.surface) { continue; }
        std::vector<Amg::MatrixX> errors;
        std::vector<Amg::Vector2D> positions = m_nnFactory->estimatePositions(
          *cluster, *trk.surface, *trk.params, errors, numberOfSubclusters);
        if (static_cast<int>(positions.size()) != numberOfSubclusters ||
            static_cast<int>(errors.size()) != numberOfSubclusters) { continue; }

        // Wide-range and precision monitoring of every sub-cluster prediction,
        // with the offset taken against the cluster position (no truth needed).
        for (int i = 0; i < numberOfSubclusters; ++i) {
          if (errors[i].rows() < 2) { continue; }
          const double sigX = std::sqrt(errors[i](0, 0));
          const double sigY = std::sqrt(errors[i](1, 1));
          if (sigX <= 0 || sigY <= 0) { continue; }
          const double dX = positions[i][Trk::locX] - cluster->localPosition()[Trk::locX];
          const double dY = positions[i][Trk::locY] - cluster->localPosition()[Trk::locY];
          auto monDeltaX   = Scalar<float>("posDeltaXWide", dX);
          auto monDeltaY   = Scalar<float>("posDeltaYWide", dY);
          auto monErrXWide = Scalar<float>("posErrXWide",   sigX);
          auto monErrYWide = Scalar<float>("posErrYWide",   sigY);
          auto monPrecX    = Scalar<float>("posPrecX",      1.0 / (sigX * sigX));
          auto monPrecY    = Scalar<float>("posPrecY",      1.0 / (sigY * sigY));
          fill(posDQ, monDeltaX, monDeltaY, monErrXWide, monErrYWide, monPrecX, monPrecY);
          evtMinErrX = std::min(evtMinErrX, sigX);
          evtMaxErrX = std::max(evtMaxErrX, sigX);
          evtMinErrY = std::min(evtMinErrY, sigY);
          evtMaxErrY = std::max(evtMaxErrY, sigY);
          evtMaxAbsDeltaX = std::max(evtMaxAbsDeltaX, std::abs(dX));
          evtMaxAbsDeltaY = std::max(evtMaxAbsDeltaY, std::abs(dY));
          evtHasPos = true;
        }

        if (!haveTruth || predN != trueN) { continue; }

        // Pick the sub-cluster the track uses (tool logic: chi2 distance to the
        // track local position, weighted by the track covariance).
        const Amg::Vector2D trkPos = trk.params->localPosition();
        // Fallback local uncertainties when the track has no covariance [mm].
        constexpr double fallbackErrX = 0.01;
        constexpr double fallbackErrY = 0.05;
        Amg::Vector2D trkErr(fallbackErrX, fallbackErrY);
        if (trk.params->covariance()) {
          trkErr = Amg::Vector2D(std::sqrt((*trk.params->covariance())(0, 0)),
                                 std::sqrt((*trk.params->covariance())(1, 1)));
        }
        int sub = 0;
        double best = std::numeric_limits<double>::max();
        for (int i = 0; i < numberOfSubclusters; ++i) {
          double d = std::pow(trkPos[0] - positions[i][0], 2) / trkErr[0]
                   + std::pow(trkPos[1] - positions[i][1], 2) / trkErr[1];
          if (d < best) { best = d; sub = i; }
        }
        if (errors[sub].rows() < 2) { continue; }
        const double errX = std::sqrt(errors[sub](0, 0));
        const double errY = std::sqrt(errors[sub](1, 1));
        if (errX <= 0 || errY <= 0) { continue; }

        // Assign the N sub-clusters to the N truths globally (min total squared
        // distance), then the track's sub-cluster gets ITS matched truth. This
        // avoids two sub-clusters grabbing the same truth in multi-particle
        // clusters. (trueN == numberOfSubclusters here, since predN==trueN.)
        std::vector<int> perm(numberOfSubclusters);
        std::iota(perm.begin(), perm.end(), 0);
        std::vector<int> bestPerm = perm;
        double bestDist = std::numeric_limits<double>::max();
        do {
          double dsum = 0.;
          for (int i = 0; i < numberOfSubclusters; ++i)
            dsum += (positions[i] - truths[perm[i]]).squaredNorm();
          if (dsum < bestDist) { bestDist = dsum; bestPerm = perm; }
        } while (std::next_permutation(perm.begin(), perm.end()));
        const int tr = bestPerm[sub];

        const double resX = positions[sub][Trk::locX] - truths[tr][Trk::locX];
        const double resY = positions[sub][Trk::locY] - truths[tr][Trk::locY];

        auto monResX  = Scalar<float>("resX",  resX / Gaudi::Units::micrometer);
        auto monResY  = Scalar<float>("resY",  resY / Gaudi::Units::micrometer);
        auto monPullX = Scalar<float>("pullX", resX / errX);
        auto monPullY = Scalar<float>("pullY", resY / errY);
        auto monErrX  = Scalar<float>("errX",  errX / Gaudi::Units::micrometer);
        auto monErrY  = Scalar<float>("errY",  errY / Gaudi::Units::micrometer);
        auto monEta   = Scalar<float>("eta",   eta);
        auto monNCell = Scalar<int>  ("nCell", nCell);
        fill(posGroup, monResX, monResY, monPullX, monPullY, monErrX, monErrY, monEta, monNCell);

        auto monPosN = Scalar<int>("posN", numberOfSubclusters);
        fill(posSummary, monPosN, monPullX, monPullY, monErrX, monErrY);
      }
    }
  }

  auto monNClusters = Scalar<int>("nClusters", nClusters);
  fill(numberGroup, monNClusters);

  if (evtHasPos) {
    auto extremes = getGroup("PixelNNExtremes");
    auto monMinErrX = Scalar<float>("evtMinErrX", evtMinErrX);
    auto monMaxErrX = Scalar<float>("evtMaxErrX", evtMaxErrX);
    auto monMinErrY = Scalar<float>("evtMinErrY", evtMinErrY);
    auto monMaxErrY = Scalar<float>("evtMaxErrY", evtMaxErrY);
    auto monMaxDX   = Scalar<float>("evtMaxAbsDeltaX", evtMaxAbsDeltaX);
    auto monMaxDY   = Scalar<float>("evtMaxAbsDeltaY", evtMaxAbsDeltaY);
    auto monMaxP2   = Scalar<float>("evtMaxProb2", evtMaxProb2);
    fill(extremes, monMinErrX, monMaxErrX, monMinErrY, monMaxErrY, monMaxDX, monMaxDY, monMaxP2);
  }
  return StatusCode::SUCCESS;
}

} // namespace InDet
