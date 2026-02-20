/*
   Copyright (C) 2002-2025 CERN
   for the benefit of the ATLAS collaboration
*/

#include "SegmentDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODMuonPrepData/UtilFunctions.h"  // getTruthMatchedParticle

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include <set>
#include <unordered_map>
#include <limits>

namespace {
struct LocalSegSorter {
  bool operator()(const xAOD::MuonSegment* a,
                  const xAOD::MuonSegment* b) const {
    if (a == b) return false;
    if (a->chamberIndex() != b->chamberIndex())
      return a->chamberIndex() < b->chamberIndex();
    if (a->sector() != b->sector())
      return a->sector() < b->sector();
    if (a->etaIndex() != b->etaIndex())
      return a->etaIndex() < b->etaIndex();
    using namespace MuonR4::SegmentFit;
    auto la = localSegmentPars(*a);
    auto lb = localSegmentPars(*b);
    return la < lb;
  }
};
}  // namespace

namespace MuonR4 {

StatusCode SegmentDumperAlg::initialize() {
  ATH_MSG_INFO("Initializing SegmentDumperAlg (MC)");

  ATH_CHECK(m_spacePointKeys.initialize());
  ATH_CHECK(m_segmentKeys.initialize());

  // Truth decoration on MC
  m_truthDecorKeys.emplace_back(m_segmentKeys, "truthParticleLink");
  ATH_CHECK(m_truthDecorKeys.initialize());

  ATH_CHECK(m_geoCtxKey.initialize());
  m_tree.addBranch(std::make_shared<MuonVal::EventHashBranch>(m_tree.tree()));
  ATH_CHECK(m_tree.init(this));
  return StatusCode::SUCCESS;
}

StatusCode SegmentDumperAlg::finalize() {
  ATH_CHECK(m_tree.write());
  return StatusCode::SUCCESS;
}

StatusCode SegmentDumperAlg::execute() {
  const EventContext& ctx{Gaudi::Hive::currentContext()};

  using SegmentsPerBucket_t =
      std::unordered_map<const SpacePointBucket*,
                         std::set<const xAOD::MuonSegment*, LocalSegSorter>>;

  const ActsTrk::GeometryContext* gctx{nullptr};
  ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

  const xAOD::MuonSegmentContainer* segContainer{nullptr};
  ATH_CHECK(SG::get(segContainer, m_segmentKeys, ctx));

  for (unsigned iKey = 0; iKey < m_spacePointKeys.size(); ++iKey) {
    const auto& spKey  = m_spacePointKeys[iKey];

    const SpacePointContainer* spContainer{nullptr};
    ATH_CHECK(SG::get(spContainer, spKey, ctx));

    SegmentsPerBucket_t segPerBucket{};
    if (segContainer) {
      for (const xAOD::MuonSegment* seg : *segContainer) {
        const auto* detSeg = MuonR4::detailedSegment(*seg);
        segPerBucket[detSeg->parent()->parentBucket()].insert(seg);
      }
    }

    for (const SpacePointBucket* bucket : *spContainer) {
      m_bucket_spacePoints = static_cast<uint16_t>(bucket->size());
      m_bucket_chamberIdx  = static_cast<uint8_t>(bucket->msSector()->chamberIndex());
      m_bucket_sector      = static_cast<uint8_t>(bucket->msSector()->sector());
      m_bucket_layers      = countLayersInBucket(*bucket);

      const auto it = segPerBucket.find(bucket);
      m_bucket_segments = (it != segPerBucket.end()) ? static_cast<uint16_t>(it->second.size()) : 0;
      if (it != segPerBucket.end()) {
        for (const xAOD::MuonSegment* seg : it->second) {
          // Reco-level outputs (always aligned to segments)
          m_segmentPos.push_back(seg->position());
          m_segmentDir.push_back(seg->direction());
          m_segment_chiSquared.push_back(seg->chiSquared());
          m_segment_numberDoF.push_back(seg->numberDoF());

          // --- Reco η/φ (GLOBAL): direction is already in global coordinates
          const Amg::Vector3D& globDir = seg->direction();
          float segEta = static_cast<float>(globDir.eta());
          float segPhi = static_cast<float>(globDir.phi());
          m_segmentRecoEta.push_back(segEta);
          m_segmentRecoPhi.push_back(segPhi);

          // Truth association
          int32_t truthIdx = -1;
          int32_t truthPdg = -1;
          const xAOD::TruthParticle* tp = getTruthMatchedParticle(*seg);
          if (tp) {
            truthIdx = static_cast<int32_t>(tp->index());
            truthPdg = static_cast<int32_t>(tp->pdgId());
          }

          // Per-segment truth flags/ids remain aligned to segments
          m_segmentHasTruth.push_back(truthIdx >= 0 ? 1 : 0);

          // Fill truth kinematics ONLY when a truth match exists (sparse truth arrays)
          if (truthIdx >= 0) {
            m_segmentTruthIdx.push_back(truthIdx);
            m_segmentTruthPDGId.push_back(truthPdg);
            m_segmentTruthPt.push_back(static_cast<float>(tp->pt()));   // MeV
            m_segmentTruthEta.push_back(static_cast<float>(tp->eta()));
            m_segmentTruthPhi.push_back(static_cast<float>(tp->phi()));
          }
        }
      }

      if (!m_tree.fill(ctx)) {
        ATH_MSG_ERROR("Failed to fill output tree");
        return StatusCode::FAILURE;
      }
    }
  }

  return StatusCode::SUCCESS;
}

uint16_t SegmentDumperAlg::countLayersInBucket(const SpacePointBucket& bucket) const {
  SpacePointPerLayerSorter sorter{};
  std::vector<unsigned int> uniqueLayers;
  uniqueLayers.reserve(bucket.size());

  for (const SpacePointBucket::value_type& sp : bucket) {
    const unsigned layNum = sorter.sectorLayerNum(*sp);
    if (std::find(uniqueLayers.begin(), uniqueLayers.end(), layNum) == uniqueLayers.end()) {
      uniqueLayers.push_back(layNum);
    }
  }
  return static_cast<uint16_t>(uniqueLayers.size());
}

}  // namespace MuonR4
