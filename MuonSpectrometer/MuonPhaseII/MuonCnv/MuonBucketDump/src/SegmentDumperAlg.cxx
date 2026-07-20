/*
   Copyright (C) 2002-2025 CERN
   for the benefit of the ATLAS collaboration
*/

#include "SegmentDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuonPrepData/UtilFunctions.h"  // getTruthMatchedParticle
#include "xAODMuonSimHit/MuonSimHit.h"
#include "xAODTruth/TruthParticle.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "TruthUtils/HepMCHelpers.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iterator>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace {
static const SG::AuxElement::ConstAccessor<int> g4TrackIdAcc{"MuonSim_G4TrkId"};

constexpr int32_t noTruthLabel() { return -1; }
constexpr int32_t noTruthSource() { return 0; }
constexpr int32_t g4TruthSource() { return 2; }
constexpr int32_t truthParticleSource() { return 3; }
constexpr int32_t firstG4PseudoLabel() { return 2000000; }

struct G4Key {
  // type = 0: HepMC/genParticleLink id, type = 1: MuonSim_G4TrkId decoration.
  int type{0};
  int id{0};
};

bool operator<(const G4Key& a, const G4Key& b) {
  if (a.type != b.type) return a.type < b.type;
  return a.id < b.id;
}

using G4KeyCounts_t = std::map<G4Key, unsigned int>;
using LabelSupport_t = std::map<int32_t, unsigned int>;

struct SegmentTruthLabel {
  int32_t id{noTruthLabel()};
  int32_t source{noTruthSource()};
  int32_t pdgId{-1};
  float pt{std::numeric_limits<float>::quiet_NaN()};
  float eta{std::numeric_limits<float>::quiet_NaN()};
  float phi{std::numeric_limits<float>::quiet_NaN()};
  uint8_t ambiguous{0};
  float weight{0.f};
  std::vector<int32_t> altIds{};
};

int simHitHepMcId(const xAOD::MuonSimHit& hit) {
  const HepMcParticleLink& link = hit.genParticleLink();
  if (link.isValid()) {
    const auto genParticle = link.cptr();
    if (genParticle) {
      const int uid = HepMC::uniqueID(genParticle);
      if (uid > 0) {
        return uid;
      }
    }
  }
  return link.id() > 0 ? link.id() : 0;
}

G4KeyCounts_t collectG4Keys(const xAOD::MuonSegment& segment) {
  G4KeyCounts_t g4Keys{};
  for (const xAOD::MuonSimHit* hit : MuonR4::getMatchingSimHits(segment)) {
    const xAOD::MuonSimHit& simHit{*hit};
    if (std::abs(simHit.pdgId()) != 13) {
      continue;
    }

    const int hepMcId = simHitHepMcId(simHit);
    if (hepMcId > 0) {
      ++g4Keys[G4Key{0, hepMcId}];
    }

    if (g4TrackIdAcc.isAvailable(simHit)) {
      const int g4TrackId = g4TrackIdAcc(simHit);
      if (g4TrackId > 0) {
        ++g4Keys[G4Key{1, g4TrackId}];
      }
    }
  }
  return g4Keys;
}

SegmentTruthLabel labelFromSupport(const LabelSupport_t& support,
                                   const unsigned int totalSupport,
                                   const int32_t source) {
  SegmentTruthLabel label{};
  if (support.empty()) {
    return label;
  }

  std::vector<std::pair<int32_t, unsigned int>> ranked{support.begin(), support.end()};
  std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
    if (a.second != b.second) return a.second > b.second;
    return a.first < b.first;
  });

  label.id = ranked.front().first;
  label.source = source;
  label.pdgId = 13;
  label.weight = totalSupport > 0
                     ? static_cast<float>(ranked.front().second) / static_cast<float>(totalSupport)
                     : 0.f;
  label.ambiguous = ranked.size() > 1 && ranked[0].second == ranked[1].second ? 1 : 0;
  label.altIds.reserve(ranked.size() > 0 ? ranked.size() - 1 : 0);
  std::transform(std::next(ranked.begin()), ranked.end(), std::back_inserter(label.altIds),
                 [](const auto& entry) { return entry.first; });
  return label;
}

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

StatusCode SegmentDumperAlg::execute(const EventContext& ctx) {
  
  using SegmentsPerBucket_t =
      std::unordered_map<const SpacePointBucket*,
                         std::set<const xAOD::MuonSegment*, LocalSegSorter>>;

  const ActsTrk::GeometryContext* gctx{nullptr};
  ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

  const xAOD::MuonSegmentContainer* segContainer{nullptr};
  ATH_CHECK(SG::get(segContainer, m_segmentKeys, ctx));

  std::map<const xAOD::MuonSegment*, const xAOD::TruthParticle*> directTruth{};
  std::map<const xAOD::MuonSegment*, G4KeyCounts_t> segmentG4Keys{};
  std::map<G4Key, unsigned int> g4SegmentMultiplicity{};
  std::map<G4Key, LabelSupport_t> g4TruthSupport{};
  std::map<G4Key, int32_t> g4PseudoLabels{};
  int32_t nextG4PseudoLabel = firstG4PseudoLabel();

  for (const xAOD::MuonSegment* seg : *segContainer) {
    const xAOD::TruthParticle* tp = getTruthMatchedParticle(*seg);
    directTruth.emplace(seg, tp);

    if (!m_includeG4TrackTruth.value()) {
      continue;
    }

    G4KeyCounts_t g4Keys = collectG4Keys(*seg);
    for (const auto& keyCount : g4Keys) {
      ++g4SegmentMultiplicity[keyCount.first];
    }
    if (tp) {
      const int32_t truthIdx = static_cast<int32_t>(tp->index());
      for (const auto& [key, count] : g4Keys) {
        g4TruthSupport[key][truthIdx] += count;
      }
    }
    segmentG4Keys.emplace(seg, std::move(g4Keys));
  }

  auto hasEnoughG4SegmentSupport = [&](const G4Key& key) {
    const unsigned int minSegments = m_minG4TrackTruthSegments.value();
    if (minSegments <= 1) {
      return true;
    }
    const auto multItr = g4SegmentMultiplicity.find(key);
    return multItr != g4SegmentMultiplicity.end() && multItr->second >= minSegments;
  };

  auto g4PseudoLabel = [&](const G4Key& key) {
    auto [itr, inserted] = g4PseudoLabels.try_emplace(key, nextG4PseudoLabel);
    if (inserted) {
      ++nextG4PseudoLabel;
    }
    return itr->second;
  };

  auto g4Label = [&](const xAOD::MuonSegment& segment) {
    SegmentTruthLabel label{};
    const auto keyItr = segmentG4Keys.find(&segment);
    if (keyItr == segmentG4Keys.end() || keyItr->second.empty()) {
      return label;
    }

    const G4KeyCounts_t& g4Keys = keyItr->second;
    LabelSupport_t propagatedSupport{};
    unsigned int totalPropagatedSupport{0};
    for (const auto& [key, count] : g4Keys) {
      if (!hasEnoughG4SegmentSupport(key)) {
        continue;
      }
      const auto supportItr = g4TruthSupport.find(key);
      if (supportItr == g4TruthSupport.end()) {
        continue;
      }
      for (const auto& [truthIdx, truthCount] : supportItr->second) {
        const unsigned int support = std::max(count, truthCount);
        propagatedSupport[truthIdx] += support;
        totalPropagatedSupport += support;
      }
    }
    if (!propagatedSupport.empty()) {
      return labelFromSupport(propagatedSupport, totalPropagatedSupport, g4TruthSource());
    }

    G4KeyCounts_t eligibleG4Keys{};
    for (const auto& [key, count] : g4Keys) {
      if (hasEnoughG4SegmentSupport(key)) {
        eligibleG4Keys[key] = count;
      }
    }
    const auto bestKey = std::max_element(eligibleG4Keys.begin(), eligibleG4Keys.end(),
                                          [](const auto& a, const auto& b) {
      if (a.second != b.second) return a.second < b.second;
      return b.first < a.first;
    });
    if (bestKey == eligibleG4Keys.end()) {
      return label;
    }

    unsigned int totalKeys{0};
    for (const auto& [key, count] : eligibleG4Keys) {
      totalKeys += count;
    }

    label.id = g4PseudoLabel(bestKey->first);
    label.source = g4TruthSource();
    label.pdgId = 13;
    label.weight = totalKeys > 0
                       ? static_cast<float>(bestKey->second) / static_cast<float>(totalKeys)
                       : 0.f;
    label.ambiguous = eligibleG4Keys.size() > 1 ? 1 : 0;
    for (const auto& keyCount : eligibleG4Keys) {
      const G4Key& key = keyCount.first;
      if (key.type == bestKey->first.type && key.id == bestKey->first.id) {
        continue;
      }
      label.altIds.push_back(g4PseudoLabel(key));
    }
    return label;
  };

  for (unsigned iKey = 0; iKey < m_spacePointKeys.size(); ++iKey) {
    const auto& spKey = m_spacePointKeys[iKey];

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
      m_bucket_chamberIdx = static_cast<uint8_t>(bucket->msSector()->chamberIndex());
      m_bucket_sector = static_cast<uint8_t>(bucket->msSector()->sector());
      m_bucket_layers = countLayersInBucket(*bucket);

      const auto it = segPerBucket.find(bucket);
      m_bucket_segments = (it != segPerBucket.end()) ? static_cast<uint16_t>(it->second.size()) : 0;
      // Flattened ragged array for alternative truth labels. The offsets vector
      // has length nSegments+1 for each dumped bucket.
      m_segmentTruthAltPartOffsets.push_back(0);
      if (it != segPerBucket.end()) {
        for (const xAOD::MuonSegment* seg : it->second) {
          // Reco-level outputs (always aligned to segments)
          m_segmentPos.push_back(seg->position());
          m_segmentDir.push_back(seg->direction());
          m_segment_chiSquared.push_back(seg->chiSquared());
          m_segment_numberDoF.push_back(seg->numberDoF());

          // --- Reco eta/phi (GLOBAL): direction is already in global coordinates
          const Amg::Vector3D& globDir = seg->direction();
          const float segEta = static_cast<float>(globDir.eta());
          const float segPhi = static_cast<float>(globDir.phi());
          m_segmentRecoEta.push_back(segEta);
          m_segmentRecoPhi.push_back(segPhi);

          SegmentTruthLabel label{};
          const auto truthItr = directTruth.find(seg);
          const xAOD::TruthParticle* tp =
              truthItr != directTruth.end() ? truthItr->second : getTruthMatchedParticle(*seg);
          if (tp) {
            label.id = static_cast<int32_t>(tp->index());
            label.source = truthParticleSource();
            label.pdgId = static_cast<int32_t>(tp->pdgId());
            label.pt = static_cast<float>(tp->pt());
            label.eta = static_cast<float>(tp->eta());
            label.phi = static_cast<float>(tp->phi());
            label.weight = 1.f;
          } else if (m_includeG4TrackTruth.value()) {
            label = g4Label(*seg);
            if (label.id >= 0) {
              label.eta = segEta;
              label.phi = segPhi;
            }
          }

          m_segmentHasTruth.push_back(label.id >= 0 ? 1 : 0);
          m_segmentTruthIdx.push_back(label.id);
          m_segmentTruthSource.push_back(label.source);
          m_segmentTruthAmbiguous.push_back(label.ambiguous);
          m_segmentTruthLabelWeight.push_back(label.weight);
          for (const int32_t altId : label.altIds) {
            m_segmentTruthAltParts.push_back(altId);
          }
          m_segmentTruthAltPartOffsets.push_back(
              static_cast<int32_t>(m_segmentTruthAltParts.size()));
          m_segmentTruthPDGId.push_back(label.pdgId);
          m_segmentTruthPt.push_back(label.pt);
          m_segmentTruthEta.push_back(label.eta);
          m_segmentTruthPhi.push_back(label.phi);
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
