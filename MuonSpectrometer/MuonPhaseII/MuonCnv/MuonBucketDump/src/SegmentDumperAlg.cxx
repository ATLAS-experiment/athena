/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "TruthUtils/MagicNumbers.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace {
static const SG::ConstAccessor<int> g4TrackIdAcc{"MuonSim_G4TrkId"};

constexpr int32_t noTruthLabel() { return -1; }
constexpr int32_t noTruthSource() { return 0; }
constexpr int32_t g4TruthSource() { return 2; }
constexpr int32_t truthParticleSource() { return 3; }
constexpr int32_t firstG4PseudoLabel() { return 2000000; }

struct G4Key {
  // type = 0: HepMC/genParticleLink id, type = 1: MuonSim_G4TrkId decoration.
  int type{0};
  int id{0};
  // Both ids are only unique within one GenEvent.
  HepMcParticleLink::index_type event{0};
  // When pile-up truth is not stored, the event index is the same for every hit
  // and only the id is left, which repeats across pile-up collisions. The charge
  // and a geometric group index (see splitG4KeysByGeometry) separate those muons.
  int pdgId{0};
  int group{0};
};

bool operator<(const G4Key& a, const G4Key& b) {
  return std::tie(a.type, a.event, a.id, a.pdgId, a.group) <
         std::tie(b.type, b.event, b.id, b.pdgId, b.group);
}

bool operator==(const G4Key& a, const G4Key& b) {
  return std::tie(a.type, a.event, a.id, a.pdgId, a.group) ==
         std::tie(b.type, b.event, b.id, b.pdgId, b.group);
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
    if (!MC::isMuon(hit)) {
      continue;
    }

    const HepMcParticleLink::index_type event = simHit.genParticleLink().eventIndex();
    const int hepMcId = simHitHepMcId(simHit);
    if (hepMcId > 0) {
      ++g4Keys[G4Key{0, hepMcId, event, simHit.pdgId()}];
    }

    if (g4TrackIdAcc.isAvailable(simHit)) {
      const int g4TrackId = g4TrackIdAcc(simHit);
      if (g4TrackId > 0) {
        ++g4Keys[G4Key{1, g4TrackId, event, simHit.pdgId()}];
      }
    }
  }
  return g4Keys;
}

int sectorDistance(int a, int b) {
  constexpr int nSectors = 16;
  const int d = std::abs(a - b);
  return std::min(d, nSectors - d);
}

struct SegmentKinematics {
  unsigned int nHits{0};
  double ekin{0.};
  double time{0.};
  double beta{0.};
};

/// Mean kinetic energy, time and beta of the muon sim hits of a segment carrying the key
SegmentKinematics segmentKinematics(const xAOD::MuonSegment& seg, const G4Key& key) {
  SegmentKinematics kin{};
  for (const xAOD::MuonSimHit* hit : MuonR4::getMatchingSimHits(seg)) {
    if (hit->pdgId() != key.pdgId || hit->genParticleLink().eventIndex() != key.event) {
      continue;
    }
    const int id = key.type == 0 ? simHitHepMcId(*hit)
                                 : (g4TrackIdAcc.isAvailable(*hit) ? g4TrackIdAcc(*hit) : 0);
    if (id != key.id) {
      continue;
    }
    ++kin.nHits;
    kin.ekin += hit->kineticEnergy();
    kin.time += hit->globalTime();
    kin.beta += hit->beta();
  }
  if (kin.nHits > 0) {
    kin.ekin /= kin.nHits;
    kin.time /= kin.nHits;
    kin.beta /= kin.nHits;
  }
  return kin;
}

/// A muon only loses energy along its path: the later of two segments (by mean
/// sim-hit time) may not carry more than maxEnergyGain above the earlier one.
/// A negative maxEnergyGain disables the check.
bool energyConsistent(const SegmentKinematics& a, const SegmentKinematics& b, double maxEnergyGain) {
  if (maxEnergyGain < 0. || a.nHits == 0 || b.nHits == 0) {
    return true;
  }
  const SegmentKinematics& early = a.time <= b.time ? a : b;
  const SegmentKinematics& late = a.time <= b.time ? b : a;
  return late.ekin - early.ekin <= maxEnergyGain;
}

/// Split every G4 key into groups of segments chained (single linkage) by sector
/// distance, position-eta proximity and energy consistency, and give each group
/// its own key. Segments of one muon stay within such a chain, while muons from
/// different pile-up collisions that share an id are separated.
void splitG4KeysByGeometry(std::map<const xAOD::MuonSegment*, G4KeyCounts_t>& segmentG4Keys,
                           double maxDeltaEta, int maxDeltaSector, double maxEnergyGain) {
  std::map<G4Key, std::vector<const xAOD::MuonSegment*>> segmentsPerKey{};
  for (const auto& [seg, keys] : segmentG4Keys) {
    for (const auto& keyCount : keys) {
      segmentsPerKey[keyCount.first].push_back(seg);
    }
  }
  // segmentG4Keys is ordered by address: use the container order so that the
  // group numbers, which break label ties, are reproducible
  for (auto& [key, segs] : segmentsPerKey) {
    std::ranges::sort(segs, {}, [](const xAOD::MuonSegment* s) { return s->index(); });
  }

  std::map<std::pair<G4Key, const xAOD::MuonSegment*>, int> groupOf{};
  for (const auto& [key, segs] : segmentsPerKey) {
    std::vector<std::size_t> parent(segs.size());
    std::iota(parent.begin(), parent.end(), 0);
    auto root = [&parent](std::size_t i) {
      while (parent[i] != i) {
        i = parent[i] = parent[parent[i]];
      }
      return i;
    };
    std::vector<SegmentKinematics> kinematics{};
    if (maxEnergyGain >= 0.) {
      kinematics.reserve(segs.size());
      for (const xAOD::MuonSegment* seg : segs) {
        kinematics.push_back(segmentKinematics(*seg, key));
      }
    }
    for (std::size_t i = 0; i < segs.size(); ++i) {
      for (std::size_t j = i + 1; j < segs.size(); ++j) {
        if (sectorDistance(segs[i]->sector(), segs[j]->sector()) <= maxDeltaSector &&
            std::abs(segs[i]->position().eta() - segs[j]->position().eta()) <= maxDeltaEta &&
            (kinematics.empty() || energyConsistent(kinematics[i], kinematics[j], maxEnergyGain))) {
          parent[root(i)] = root(j);
        }
      }
    }
    std::map<std::size_t, int> groupIndex{};
    for (std::size_t i = 0; i < segs.size(); ++i) {
      const int nGroups = static_cast<int>(groupIndex.size());
      groupOf[{key, segs[i]}] = groupIndex.try_emplace(root(i), nGroups).first->second;
    }
  }

  for (auto& [seg, keys] : segmentG4Keys) {
    G4KeyCounts_t splitKeys{};
    for (const auto& [key, count] : keys) {
      G4Key groupKey{key};
      groupKey.group = groupOf.at({key, seg});
      splitKeys[groupKey] += count;
    }
    keys = std::move(splitKeys);
  }
}

/// DEBUG diagnostic: a G4 key whose segments lie on both detector sides cannot
/// belong to a single muon. Print the link content of every muon sim hit behind
/// such keys, to see which identifier collides between different particles.
void printCrossSideG4Keys(MsgStream& msg, const EventContext& ctx,
                          const std::map<const xAOD::MuonSegment*, G4KeyCounts_t>& segmentG4Keys) {
  constexpr unsigned int maxPrintedKeys = 5;
  std::map<G4Key, std::vector<const xAOD::MuonSegment*>> segmentsPerKey{};
  for (const auto& [seg, keys] : segmentG4Keys) {
    for (const auto& keyCount : keys) {
      segmentsPerKey[keyCount.first].push_back(seg);
    }
  }

  const auto eventNumber = ctx.eventID().event_number();
  unsigned int nCrossSide{0};
  for (const auto& [key, segs] : segmentsPerKey) {
    const bool sideA = std::ranges::any_of(segs, [](const xAOD::MuonSegment* s) { return s->position().z() > 0.; });
    const bool sideC = std::ranges::any_of(segs, [](const xAOD::MuonSegment* s) { return s->position().z() < 0.; });
    if (!sideA || !sideC || ++nCrossSide > maxPrintedKeys) {
      continue;
    }
    msg << MSG::DEBUG << "Cross-side G4 key in event " << eventNumber << ": type=" << key.type
        << " eventIndex=" << key.event << " id=" << key.id << " pdgId=" << key.pdgId
        << " group=" << key.group << " on " << segs.size() << " segments" << endmsg;
    for (const xAOD::MuonSegment* seg : segs) {
      msg << MSG::DEBUG << "  segment z=" << seg->position().z() << " sector=" << seg->sector()
          << " chamberIndex=" << static_cast<int>(seg->chamberIndex()) << endmsg;
      for (const xAOD::MuonSimHit* hit : MuonR4::getMatchingSimHits(*seg)) {
        if (!MC::isMuon(hit)) {
          continue;
        }
        const HepMcParticleLink& link = hit->genParticleLink();
        msg << MSG::DEBUG << "    hit pdg=" << hit->pdgId() << " linkValid=" << link.isValid()
            << " hasGenParticle=" << (link.isValid() && link.cptr() != nullptr)
            << " eventIndex=" << link.eventIndex() << " linkId=" << link.id()
            << " uniqueID=" << simHitHepMcId(*hit)
            << " g4TrkId=" << (g4TrackIdAcc.isAvailable(*hit) ? g4TrackIdAcc(*hit) : -1)
            << " t=" << hit->globalTime() << " Ekin=" << hit->kineticEnergy() << endmsg;
      }
    }
  }
  msg << MSG::DEBUG << "Event " << eventNumber << ": " << segmentsPerKey.size() << " G4 keys, "
      << nCrossSide << " with segments on both detector sides" << endmsg;
}

/// DEBUG measurement for a kinematic consistency check of G4 labels: print the sim-hit
/// kinematics of segment pairs (in different chambers) that are
///   truth: matched to the same truth particle (known to be one muon),
///   split: sharing an id and charge but separated by splitG4KeysByGeometry,
///   kept:  sharing an id and charge and kept in one geometric group.
void printKinematicPairs(MsgStream& msg, const EventContext& ctx,
                         const std::map<const xAOD::MuonSegment*, G4KeyCounts_t>& unsplitG4Keys,
                         const std::map<const xAOD::MuonSegment*, G4KeyCounts_t>& splitG4Keys,
                         const std::map<const xAOD::MuonSegment*, const xAOD::TruthParticle*>& directTruth) {
  const auto eventNumber = ctx.eventID().event_number();
  auto printPair = [&](const char* category, const G4Key& key, const xAOD::MuonSegment* a,
                       const xAOD::MuonSegment* b) {
    if (a->chamberIndex() == b->chamberIndex()) {
      return;
    }
    const SegmentKinematics ka = segmentKinematics(*a, key);
    const SegmentKinematics kb = segmentKinematics(*b, key);
    if (ka.nHits == 0 || kb.nHits == 0) {
      return;
    }
    msg << MSG::DEBUG << "KinPair cat=" << category << " evt=" << eventNumber
        << " tpA=" << (directTruth.at(a) != nullptr) << " tpB=" << (directTruth.at(b) != nullptr)
        << " zA=" << a->position().z() << " zB=" << b->position().z()
        << " etaA=" << a->position().eta() << " etaB=" << b->position().eta()
        << " nA=" << ka.nHits << " nB=" << kb.nHits << " eA=" << ka.ekin << " eB=" << kb.ekin
        << " tA=" << ka.time << " tB=" << kb.time << " bA=" << ka.beta << " bB=" << kb.beta
        << " dist=" << (a->position() - b->position()).mag() << endmsg;
  };

  // Group index of each (unsplit key, segment) after the geometric split
  std::map<std::pair<G4Key, const xAOD::MuonSegment*>, int> groupOf{};
  for (const auto& [seg, keys] : splitG4Keys) {
    for (const auto& keyCount : keys) {
      G4Key unsplit{keyCount.first};
      unsplit.group = 0;
      groupOf[{unsplit, seg}] = keyCount.first.group;
    }
  }
  std::map<G4Key, std::vector<const xAOD::MuonSegment*>> segmentsPerKey{};
  for (const auto& [seg, keys] : unsplitG4Keys) {
    for (const auto& keyCount : keys) {
      segmentsPerKey[keyCount.first].push_back(seg);
    }
  }
  for (const auto& [key, segs] : segmentsPerKey) {
    for (std::size_t i = 0; i < segs.size(); ++i) {
      for (std::size_t j = i + 1; j < segs.size(); ++j) {
        const bool sameGroup = groupOf.at({key, segs[i]}) == groupOf.at({key, segs[j]});
        printPair(sameGroup ? "kept" : "split", key, segs[i], segs[j]);
      }
    }
  }

  // Known single muons: use the dominant sim-hit key of the first segment
  std::map<const xAOD::TruthParticle*, std::vector<const xAOD::MuonSegment*>> segmentsPerTruth{};
  for (const auto& [seg, tp] : directTruth) {
    if (tp) {
      segmentsPerTruth[tp].push_back(seg);
    }
  }
  for (auto& [tp, segs] : segmentsPerTruth) {
    std::ranges::sort(segs, {}, [](const xAOD::MuonSegment* s) { return s->index(); });
  }
  for (const auto& [tp, segs] : segmentsPerTruth) {
    for (std::size_t i = 0; i < segs.size(); ++i) {
      const G4KeyCounts_t& keys = unsplitG4Keys.at(segs[i]);
      if (keys.empty()) {
        continue;
      }
      const G4Key& dominant = std::max_element(keys.begin(), keys.end(), [](const auto& x, const auto& y) {
        return x.second < y.second;
      })->first;
      for (std::size_t j = i + 1; j < segs.size(); ++j) {
        printPair("truth", dominant, segs[i], segs[j]);
      }
    }
  }
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

/// Key used by SegmentEdgeClassifierTool to count the bucket_segments node feature
using StationKey_t = std::tuple<int, int, int>;  // sector, chamberIndex, etaIndex
StationKey_t stationKey(const xAOD::MuonSegment& seg) {
  return {seg.sector(), static_cast<int>(seg.chamberIndex()), seg.etaIndex()};
}
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
  // Counted over the full container, without deduplication, as the inference does
  std::map<StationKey_t, uint16_t> segmentsPerStation{};

  for (const xAOD::MuonSegment* seg : *segContainer) {
    ++segmentsPerStation[stationKey(*seg)];
    directTruth.emplace(seg, getTruthMatchedParticle(*seg));
    if (m_includeG4TrackTruth.value()) {
      segmentG4Keys.emplace(seg, collectG4Keys(*seg));
    }
  }

  if (m_includeG4TrackTruth.value()) {
    // Multiplicity and truth support are counted on the split keys, so a
    // truth index only propagates within one geometric group.
    std::map<const xAOD::MuonSegment*, G4KeyCounts_t> unsplitG4Keys{};
    if (msgLvl(MSG::DEBUG)) {
      unsplitG4Keys = segmentG4Keys;
    }
    splitG4KeysByGeometry(segmentG4Keys, m_g4LabelMaxDeltaEta.value(), m_g4LabelMaxDeltaSector.value(),
                          m_g4LabelMaxEnergyGain.value());
    for (const auto& [seg, g4Keys] : segmentG4Keys) {
      for (const auto& keyCount : g4Keys) {
        ++g4SegmentMultiplicity[keyCount.first];
      }
      if (const xAOD::TruthParticle* tp = directTruth.at(seg)) {
        const int32_t truthIdx = static_cast<int32_t>(tp->index());
        for (const auto& [key, count] : g4Keys) {
          g4TruthSupport[key][truthIdx] += count;
        }
      }
    }
    if (msgLvl(MSG::DEBUG)) {
      printCrossSideG4Keys(msg(), ctx, segmentG4Keys);
      printKinematicPairs(msg(), ctx, unsplitG4Keys, segmentG4Keys, directTruth);
    }
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
      if (key == bestKey->first) {
        continue;
      }
      label.altIds.push_back(g4PseudoLabel(key));
    }
    return label;
  };

  auto segmentLabel = [&](const xAOD::MuonSegment& seg) {
    SegmentTruthLabel label{};
    if (const xAOD::TruthParticle* tp = directTruth.at(&seg)) {
      label.id = static_cast<int32_t>(tp->index());
      label.source = truthParticleSource();
      label.pdgId = static_cast<int32_t>(tp->pdgId());
      label.pt = static_cast<float>(tp->pt());
      label.eta = static_cast<float>(tp->eta());
      label.phi = static_cast<float>(tp->phi());
      label.weight = 1.f;
    } else if (m_includeG4TrackTruth.value()) {
      label = g4Label(seg);
      if (label.id >= 0) {
        label.eta = static_cast<float>(seg.direction().eta());
        label.phi = static_cast<float>(seg.direction().phi());
      }
    }
    return label;
  };

  // Labels are decided per event before filling: a muon whose segments all sit
  // in one chamberIndex (single or duplicate segments) has no edge the model can
  // learn, since same-chamber pairs are dropped, so it is treated as background.
  std::unordered_map<const xAOD::MuonSegment*, SegmentTruthLabel> segmentLabels{};
  std::map<int32_t, std::set<int>> chambersPerLabel{};
  for (const xAOD::MuonSegment* seg : *segContainer) {
    const SegmentTruthLabel& label = segmentLabels.emplace(seg, segmentLabel(*seg)).first->second;
    if (label.id >= 0) {
      chambersPerLabel[label.id].insert(static_cast<int>(seg->chamberIndex()));
    }
  }
  for (auto& [seg, label] : segmentLabels) {
    if (label.id >= 0 && chambersPerLabel.at(label.id).size() < m_minLabelChambers.value()) {
      label = SegmentTruthLabel{};
    }
  }

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
          m_segment_bucketMultiplicity.push_back(segmentsPerStation.at(stationKey(*seg)));

          // --- Reco eta/phi (GLOBAL): direction is already in global coordinates
          const Amg::Vector3D& globDir = seg->direction();
          const float segEta = static_cast<float>(globDir.eta());
          const float segPhi = static_cast<float>(globDir.phi());
          m_segmentRecoEta.push_back(segEta);
          m_segmentRecoPhi.push_back(segPhi);

          const SegmentTruthLabel& label = segmentLabels.at(seg);

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
