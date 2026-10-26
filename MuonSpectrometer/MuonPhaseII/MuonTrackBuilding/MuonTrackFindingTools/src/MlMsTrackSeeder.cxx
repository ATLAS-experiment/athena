/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MlMsTrackSeeder.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Definitions/Units.hpp"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <set>
#include <optional>
#include <span>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MuonR4 {

namespace {

struct MlComponent {
  std::vector<const xAOD::MuonSegment*> segments;
  /// (anchor rank, anchor); rank 0 is the best anchor
  std::vector<std::pair<unsigned int, const xAOD::MuonSegment*>> anchors;
};

using LayerIndex = Muon::MuonStationIndex::LayerIndex;

LayerIndex layerOf(const xAOD::MuonSegment* seg) {
  return Muon::MuonStationIndex::toLayerIndex(seg->chamberIndex());
}
bool isInner(const xAOD::MuonSegment* seg) { return layerOf(seg) == LayerIndex::Inner; }
/// A seed keeps one segment per seed layer: the station layer, with barrel and endcap chambers
/// kept apart so that a muon in the barrel-endcap overlap keeps e.g. both its BM and EM segments.
using SeedLayer = std::pair<LayerIndex, bool>;
SeedLayer seedLayer(const xAOD::MuonSegment* seg) {
  return {layerOf(seg), Muon::MuonStationIndex::isBarrel(seg->chamberIndex())};
}
using LayerMap_t = std::map<SeedLayer, const xAOD::MuonSegment*>;
/// Distinct station layers of a seed made of `centre` and `members` (the MinLayers count)
std::size_t nStationLayers(const xAOD::MuonSegment* centre, const LayerMap_t& members) {
  std::set<LayerIndex> layers{layerOf(centre)};
  for (const auto& [layer, segment] : members) layers.insert(layer.first);
  return layers.size();
}

/// Truth classes of a seed (diagnostics only): 0 = pure, 1 = partial, 2 = junk.
/// `owner` is the particle with most segments on the seed if it has at least two.
std::size_t classifySeed(const xAOD::MuonSegment* anchor, const LayerMap_t& members,
                         const xAOD::TruthParticle*& owner) {
  std::unordered_map<const xAOD::TruthParticle*, unsigned int> counts;
  const xAOD::TruthParticle* anchorPart = MuonR4::getTruthMatchedParticle(*anchor);
  if (anchorPart) ++counts[anchorPart];
  for (const auto& [layer, segment] : members) {
    if (const xAOD::TruthParticle* part = MuonR4::getTruthMatchedParticle(*segment)) ++counts[part];
  }
  owner = nullptr;
  unsigned int best = 1;
  for (const auto& [part, count] : counts) {
    if (count > best || (count == best && owner && part == anchorPart)) {
      best = count;
      owner = part;
    }
  }
  if (anchorPart && counts[anchorPart] == members.size() + 1) return 0;
  return owner ? 1 : 2;
}

/// Same segment ranking as SegmentEdgeInferenceAlg
const MuonR4::SegmentQualitySorter betterQuality{};

/// Four-class truth label of a set of segments (diagnostics only): 0 = pure (all from
/// one particle), 1 = partial (>= 2 share a particle), 2 = junk without any truth-matched
/// segment, 3 = junk mixing different particles.
std::uint8_t classifySegments(std::span<const xAOD::MuonSegment* const> segments) {
  std::unordered_map<const xAOD::TruthParticle*, unsigned int> counts;
  for (const xAOD::MuonSegment* segment : segments) {
    if (const xAOD::TruthParticle* part = MuonR4::getTruthMatchedParticle(*segment)) ++counts[part];
  }
  unsigned int best = 0;
  for (const auto& [part, count] : counts) best = std::max(best, count);
  if (best == segments.size()) return 0;
  if (best >= 2) return 1;
  return counts.empty() ? 2 : 3;
}

/// Spread of the segments' polar directions in degrees
double thetaSpreadDeg(std::span<const xAOD::MuonSegment* const> segments) {
  double minTheta = M_PI, maxTheta = 0.;
  for (const xAOD::MuonSegment* segment : segments) {
    minTheta = std::min(minTheta, segment->direction().theta());
    maxTheta = std::max(maxTheta, segment->direction().theta());
  }
  return (maxTheta - minTheta) / Gaudi::Units::deg;
}

}  // namespace

StatusCode MlMsTrackSeeder::initialize() {
  ATH_CHECK(m_seedParameterEstimator.retrieve());
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_candidateDecorKey.initialize());
  ATH_CHECK(m_componentScoreKey.initialize());
  if (m_truthDiag.value() && m_segSelector.empty()) {
    ATH_MSG_ERROR("EnableTruthDiagnostics needs a SegmentSelectionTool");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_segSelector.retrieve(EnableTool{!m_segSelector.empty()}));
  ATH_CHECK(m_ctxProvider.initialize());
  return StatusCode::SUCCESS;
}

StatusCode MlMsTrackSeeder::findTrackSeeds(
    const EventContext& ctx,
    std::vector<MsTrackSeed>& outSeeds) const {
  // Diagnostics cost nothing unless the tool runs at DEBUG level.
  const bool diag = msgLvl(MSG::DEBUG);
  const auto start = diag ? std::chrono::steady_clock::now()
                          : std::chrono::steady_clock::time_point{};
  const xAOD::MuonSegmentContainer* segments{nullptr};
  ATH_CHECK(SG::get(segments, m_segmentKey, ctx));

  // The decoration payload is [componentId, anchorRank + 1 (0 = no anchor)].
  SG::ReadDecorHandle<xAOD::MuonSegmentContainer, std::vector<unsigned int>>
      componentAcc{m_candidateDecorKey, ctx};
  if (!componentAcc.isAvailable()) {
    ATH_MSG_DEBUG("No ML candidate decoration found");
    return StatusCode::SUCCESS;
  }

  const std::size_t outSeedsBegin = outSeeds.size();
  // projected distance between the two segments of 2-segment window seeds, by seed index
  std::unordered_map<std::size_t, float> twoSegDist;
  std::map<unsigned int, MlComponent> components;

  for (const xAOD::MuonSegment* segment : *segments) {
    if (!segment) continue;
    const std::vector<unsigned int>& payload = componentAcc(*segment);
    if (payload.size() != 2 || payload[0] == 0) continue;
    MlComponent& component = components[payload[0]];
    component.segments.push_back(segment);
    if (payload[1] != 0) component.anchors.emplace_back(payload[1] - 1, segment);
  }

  const bool truthDiag = diag && m_truthDiag.value();
  std::unordered_set<const xAOD::TruthParticle*> windowBefore, windowAfter;
  const bool useWindow = m_pointingWindow.value() > 0.;
  std::optional<Acts::GeometryContext> tgContext{};
  if (useWindow) tgContext.emplace(m_ctxProvider.getGeometryContext(ctx));
  std::size_t windowCentreNotAnchor = 0, windowCentreOutOfBounds = 0;
  std::size_t windowDropped = 0, windowOutOfBounds = 0;
  std::size_t twoSegLowScore = 0, twoSegNoScore = 0;
  // 2-segment rule: a 2-segment seed needs a strong ML cross-layer association in its
  // component. Returns false if the seed is dropped.
  std::optional<SG::ReadDecorHandle<xAOD::MuonSegmentContainer, std::vector<float>>> twoSegScoreAcc{};
  if (m_twoSegMinScore.value() > 0.) {
    twoSegScoreAcc.emplace(m_componentScoreKey, ctx);
    if (!twoSegScoreAcc->isAvailable()) {
      ATH_MSG_WARNING("ML component score decoration " << m_componentScoreKey.key()
                      << " not available: TwoSegmentMinComponentScore is not applied");
      twoSegScoreAcc.reset();
    }
  }
  const auto passesTwoSegmentRule = [&](const xAOD::MuonSegment* centre, const xAOD::MuonSegment* member) {
    if (!twoSegScoreAcc) return true;
    const std::vector<float>& score = (*twoSegScoreAcc)(*centre);
    if (score.empty()) {
      ++twoSegNoScore;
      return true;
    }
    if (score[0] >= m_twoSegMinScore.value()) return true;
    ++twoSegLowScore;
    if (truthDiag) {
      const std::array<const xAOD::MuonSegment*, 2> segs{centre, member};
      ++m_anatomy.twoSegDropped[std::min<std::size_t>(classifySegments(segs), 2)];
    }
    return false;
  };
  // momentum estimate of the start parameters and the pure seeds without start parameters.
  const auto recordStartParameters = [&](const MsTrackSeed& seed) {
    if (!truthDiag) return;
    const auto pars = m_seedParameterEstimator->estimateStartParameters(ctx, seed);
    const std::size_t cls = classifySegments(seed.segments());
    if (!pars.ok()) {
      if (cls == 0) {
        ++m_anatomy.pureNoStartPars;
        const bool anyQuality = std::ranges::any_of(seed.segments(), [&](const xAOD::MuonSegment* seg) {
          return m_segSelector->passSeedingQuality(ctx, *seg);
        });
        const bool hasNsw = std::ranges::any_of(seed.segments(), [](const xAOD::MuonSegment* seg) {
          const auto tech = seg->technology();
          return tech == Muon::MuonStationIndex::TechnologyIndex::MM ||
                 tech == Muon::MuonStationIndex::TechnologyIndex::STGC;
        });
        ++m_anatomy.pureNoStartParsQuality[anyQuality];
        ++m_anatomy.pureNoStartParsNsw[hasNsw];
      }
      return;
    }
    const double p = (*pars).absoluteMomentum() / Acts::UnitConstants::GeV;
    const std::size_t pBin = p < 3. ? 0 : p < 6. ? 1 : p < 10. ? 2 : p < 20. ? 3 : 4;
    ++m_anatomy.seedsByP[cls * SeedAnatomy::kPBins + pBin];
  };

  // D12: furthest seeding stage reached by each selected truth muon
  using LayerSet = std::set<LayerIndex>;
  struct MuonTrace {
    LayerSet layers{};    //!< layers with a segment of the muon
    LayerSet mlLayers{};  //!< ... of which with a segment in an ML component
    std::size_t stage{0};
    std::size_t defects{0};  //!< missing layers + fakes of the recorded stage-7 seed
    std::array<unsigned int, EtaFunnel::kMissReasons> missing{};
    std::array<unsigned int, 2> fakes{};
  };
  std::unordered_map<const xAOD::TruthParticle*, MuonTrace> traces;
  if (truthDiag) {
    for (const xAOD::MuonSegment* segment : *segments) {
      if (!segment) continue;
      const xAOD::TruthParticle* part = MuonR4::getTruthMatchedParticle(*segment);
      if (!part || std::abs(part->pdgId()) != 13 || part->pt() < 20. * Gaudi::Units::GeV ||
          std::abs(part->eta()) >= 2.5) {
        continue;
      }
      MuonTrace& trace = traces[part];
      trace.layers.insert(layerOf(segment));
      const std::vector<unsigned int>& payload = componentAcc(*segment);
      if (payload.size() == 2 && payload[0] != 0) trace.mlLayers.insert(layerOf(segment));
    }
    for (auto& [part, trace] : traces) trace.stage = trace.mlLayers.size() >= 2 ? 1 : 0;
  }
  // Layers of each selected truth muon among `segs`, only for muons with >= 2 layers
  const auto muonLayers = [&](std::span<const xAOD::MuonSegment* const> segs) {
    std::unordered_map<const xAOD::TruthParticle*, LayerSet> out;
    for (const xAOD::MuonSegment* seg : segs) {
      const xAOD::TruthParticle* part = MuonR4::getTruthMatchedParticle(*seg);
      if (part && traces.contains(part)) out[part].insert(layerOf(seg));
    }
    std::erase_if(out, [](const auto& entry) { return entry.second.size() < 2; });
    return out;
  };
  const auto traceRejected = [&](const MlComponent& component) {
    if (!truthDiag) return;
    for (const auto& [part, layers] : muonLayers(component.segments)) {
      MuonTrace& trace = traces[part];
      trace.stage = std::max<std::size_t>(trace.stage, 2);
    }
  };
  // Stage reached, for one anchor of the component, by each muon with >= 2 layers in it
  const auto traceSeed = [&](const MlComponent& component,
                             const std::vector<const xAOD::MuonSegment*>& pool,
                             const std::vector<const xAOD::MuonSegment*>& inWindow,
                             const xAOD::MuonSegment* centre, const LayerMap_t& windowed,
                             bool droppedTwoSeg) {
    std::vector<const xAOD::MuonSegment*> seedSegs{centre};
    for (const auto& [layer, segment] : windowed) seedSegs.push_back(segment);
    const auto poolMu = muonLayers(pool);
    const auto windowMu = muonLayers(inWindow);
    const auto has = [](const auto& map, const xAOD::TruthParticle* part, LayerIndex layer) {
      const auto it = map.find(part);
      return it != map.end() && it->second.contains(layer);
    };
    for (const auto& [part, compLayers] : muonLayers(component.segments)) {
      MuonTrace& trace = traces[part];
      LayerSet seedLayers;
      std::array<unsigned int, 2> fakes{};
      for (const xAOD::MuonSegment* seg : seedSegs) {
        const xAOD::TruthParticle* segPart = MuonR4::getTruthMatchedParticle(*seg);
        if (segPart == part) seedLayers.insert(layerOf(seg));
        else ++fakes[segPart ? 1 : 0];
      }
      std::size_t stage = !poolMu.contains(part) ? 3 : !windowMu.contains(part) ? 4
                        : seedLayers.size() < 2 ? 5 : droppedTwoSeg ? 6 : 7;
      std::array<unsigned int, EtaFunnel::kMissReasons> missing{};
      std::size_t defects = fakes[0] + fakes[1];
      if (stage == 7) {
        for (const LayerIndex layer : trace.layers) {
          if (seedLayers.contains(layer)) continue;
          ++defects;
          ++missing[!compLayers.contains(layer) ? 0 : !has(poolMu, part, layer) ? 1
                    : !has(windowMu, part, layer) ? 2 : layer == layerOf(centre) ? 3 : 4];
        }
        if (defects == 0) stage = 8;
      }
      if (stage > trace.stage || (stage == 7 && trace.stage == 7 && defects < trace.defects)) {
        trace.stage = stage;
        trace.defects = defects;
        trace.missing = missing;
        trace.fakes = fakes;
      }
    }
  };

  // ExtraClusterSeeds: an extra cluster needs a segment whose own strongest selected
  // cross-layer ML edge reaches ExtraClusterMinScore.
  std::optional<SG::ReadDecorHandle<xAOD::MuonSegmentContainer, std::vector<float>>> segScoreAcc{};
  if (m_extraClusterSeeds.value() > 0 || m_variantSeeds.value()) {
    segScoreAcc.emplace(m_componentScoreKey, ctx);
    if (!segScoreAcc->isAvailable()) {
      ATH_MSG_WARNING("ML component score decoration " << m_componentScoreKey.key()
                      << " not available: ExtraClusterSeeds and VariantSeeds are not applied");
      segScoreAcc.reset();
    }
  }
  const auto strongSegment = [&](const xAOD::MuonSegment* seg) {
    if (!segScoreAcc) return false;
    const std::vector<float>& score = (*segScoreAcc)(*seg);
    return score.size() == 2 && score[1] >= m_extraClusterMinScore.value();
  };
  std::size_t extraSeeds = 0, extraNoStrong = 0;

  // IP pointing of a segment: |theta(direction) - theta(position)| in degrees
  const auto pointingDeg = [](const xAOD::MuonSegment* seg) {
    return std::abs(seg->direction().theta() - seg->position().theta()) / Gaudi::Units::deg;
  };
  // InnerPointingMaxDeg: endcap inner segments that do not point back to the IP
  const auto failsPointing = [&](const xAOD::MuonSegment* seg) {
    return m_innerPointingMaxDeg.value() > 0. && isInner(seg) &&
           !Muon::MuonStationIndex::isBarrel(seg->chamberIndex()) &&
           pointingDeg(seg) > m_innerPointingMaxDeg.value();
  };
  std::size_t pointingRemoved = 0, pointingCentreSkipped = 0, innerRescued = 0;

  // VariantSeeds: only seeds likely to come from a muon get a second seed
  const auto passesVariantGate = [&](const xAOD::MuonSegment* centre, const LayerMap_t& members,
                                     MsTrackSeed::Location loc) {
    if (nStationLayers(centre, members) < m_variantMinLayers.value()) return false;
    if (loc == MsTrackSeed::Location::Endcap) {
      // the inner segment must point back to the IP
      const xAOD::MuonSegment* inner = isInner(centre) ? centre : nullptr;
      for (const auto& [layer, segment] : members) {
        if (isInner(segment)) inner = segment;
      }
      if (!inner || pointingDeg(inner) > m_variantMaxPointingDeg.value()) return false;
    }
    if (m_variantMinScore.value() > 0.) {
      if (!segScoreAcc) return false;
      const std::vector<float>& score = (*segScoreAcc)(*centre);
      if (score.size() != 2 || score[0] < m_variantMinScore.value()) return false;
    }
    return true;
  };
  // The variant: every seed layer takes its runner-up from the window. Without any alternative,
  // the weakest non-inner member of a seed with >= 3 station layers is dropped instead.
  // Returns an empty map if no different seed with >= 2 station layers can be built.
  const auto buildVariant = [&](const xAOD::MuonSegment* centre, const LayerMap_t& members,
                                const std::vector<const xAOD::MuonSegment*>& window) {
    LayerMap_t variant = members;
    variant[seedLayer(centre)] = centre;
    bool changed = false;
    for (auto& [layer, chosen] : variant) {
      const xAOD::MuonSegment* alternative{nullptr};
      for (const xAOD::MuonSegment* candidate : window) {
        if (candidate == chosen || seedLayer(candidate) != layer) continue;
        if (!alternative || betterQuality(candidate, alternative)) alternative = candidate;
      }
      if (alternative) {
        chosen = alternative;
        changed = true;
      }
    }
    const auto stationLayers = [](const LayerMap_t& layers) {
      std::set<LayerIndex> distinct;
      for (const auto& [layer, segment] : layers) distinct.insert(layer.first);
      return distinct.size();
    };
    if (!changed && stationLayers(variant) >= 3) {
      auto weakest = variant.end();
      for (auto it = variant.begin(); it != variant.end(); ++it) {
        if (it->first.first == LayerIndex::Inner) continue;
        if (weakest == variant.end() || betterQuality(weakest->second, it->second)) weakest = it;
      }
      if (weakest != variant.end()) {
        variant.erase(weakest);
        changed = true;
      }
    }
    if (!changed || stationLayers(variant) < 2) variant.clear();
    return variant;
  };
  std::size_t variantSeeds = 0, variantNone = 0;

  std::size_t acceptedComponents = 0;
  std::size_t constructedSeeds = 0;
  std::size_t tooSmall = 0, tooLarge = 0, noAnchor = 0, tooFewLayers = 0;
  std::array<std::size_t, 4> seedSegments{};
  for (auto& [componentId, component] : components) {
    if (component.segments.size() < m_minSegmentsPerCandidate.value()) {
      ++tooSmall;
      traceRejected(component);
      continue;
    }
    if (m_maxSegmentsPerCandidate.value() != 0 &&
        component.segments.size() > m_maxSegmentsPerCandidate.value()) {
      ++tooLarge;
      traceRejected(component);
      continue;
    }
    if (component.anchors.empty()) {
      ++noAnchor;
      traceRejected(component);
      continue;
    }
    ++acceptedComponents;

    // Best anchor first.
    std::ranges::sort(component.anchors);

    for (const auto& [rank, anchor] : component.anchors) {
      if (useWindow) {
        // ExtraClusterSeeds: segments of the component not yet covered by a window of this anchor
        std::vector<const xAOD::MuonSegment*> remaining = component.segments;
        std::vector<const xAOD::MuonSegment*> lastWindow{};
        for (unsigned int cluster = 0; cluster <= m_extraClusterSeeds.value(); ++cluster) {
          const bool extra = cluster > 0;
          if (extra) {
            std::erase_if(remaining, [&](const xAOD::MuonSegment* seg) {
              return std::ranges::find(lastWindow, seg) != lastWindow.end();
            });
            if (remaining.size() < 2) break;
          }
          // Pool: the anchor and the component segments direction-compatible with it; for an
          // extra cluster, the remaining segments (filtered against the cluster centre below).
          std::vector<const xAOD::MuonSegment*> pool{};
          if (!extra) {
            pool.push_back(anchor);
            for (const xAOD::MuonSegment* segment : component.segments) {
              if (segment == anchor) continue;
              if (anchor->direction().dot(segment->direction()) < m_minCosConsistency.value()) continue;
              pool.push_back(segment);
            }
          } else {
            pool = remaining;
          }
          if (m_innerPointingMaxDeg.value() > 0.) {
            std::erase_if(pool, [&](const xAOD::MuonSegment* seg) {
              if (!failsPointing(seg)) return false;
              if (!extra) {
                ++pointingRemoved;
                if (truthDiag) ++m_rescue.pointingRemoved[MuonR4::getTruthMatchedParticle(*seg) ? 0 : 1];
              }
              return true;
            });
          }
          // Projection of the pool onto the standard seeder's cylinder around a centre:
          // z in the barrel, R in the endcap.
          struct Projection {
            MsTrackSeed::Location loc{MsTrackSeed::Location::Undefined};
            ExpandedSector sector{static_cast<std::int8_t>(0)};
            bool centreInBounds{false};
            std::vector<char> inBounds{};
            std::vector<double> dist{};
          };
          const auto projectAround = [&](const xAOD::MuonSegment* centre) {
            Projection pr{};
            pr.loc = Muon::MuonStationIndex::isBarrel(centre->chamberIndex())
                         ? MsTrackSeed::Location::Barrel : MsTrackSeed::Location::Endcap;
            pr.sector = ExpandedSector{centre->position().phi()};
            const std::size_t coord = pr.loc == MsTrackSeed::Location::Barrel ? 1 : 0;
            const Amg::Vector2D centreProj =
                m_seedParameterEstimator->expressOnCylinder(*tgContext, *centre, pr.loc, pr.sector);
            pr.centreInBounds = m_seedParameterEstimator->withinBounds(centreProj, pr.loc);
            pr.inBounds.assign(pool.size(), 0);
            pr.dist.assign(pool.size(), 0.);
            for (std::size_t i = 0; i < pool.size(); ++i) {
              if (pool[i] == centre) {
                pr.inBounds[i] = 1;
                continue;
              }
              const Amg::Vector2D proj =
                  m_seedParameterEstimator->expressOnCylinder(*tgContext, *pool[i], pr.loc, pr.sector);
              pr.inBounds[i] = m_seedParameterEstimator->withinBounds(proj, pr.loc);
              pr.dist[i] = std::abs(proj[coord] - centreProj[coord]);
            }
            return pr;
          };
          const auto inWindowOf = [&](const Projection& pr, std::size_t i, const xAOD::MuonSegment* centre) {
            return pool[i] == centre || !pr.centreInBounds ||
                   (pr.inBounds[i] && pr.dist[i] <= m_pointingWindow.value());
          };
          // One segment per layer by quality; the centre always represents its own layer.
          const auto perLayer = [&](const xAOD::MuonSegment* centre,
                                    const std::vector<const xAOD::MuonSegment*>& segs) {
            LayerMap_t layers;
            const SeedLayer centreLayer = seedLayer(centre);
            for (const xAOD::MuonSegment* segment : segs) {
              const SeedLayer layer = seedLayer(segment);
              if (layer == centreLayer) continue;
              const xAOD::MuonSegment*& best = layers[layer];
              if (!best || betterQuality(segment, best)) best = segment;
            }
            return layers;
          };
          // Cluster centre: the segment whose window covers the most distinct layers
          // (ties: segment quality).
          const auto clusterCentre = [&]() {
            const xAOD::MuonSegment* best{nullptr};
            std::size_t bestLayers{0};
            for (std::size_t c = 0; c < pool.size(); ++c) {
              const Projection pr = projectAround(pool[c]);
              if (!pr.centreInBounds) continue;
              std::set<LayerIndex> layers;
              for (std::size_t i = 0; i < pool.size(); ++i) {
                if (inWindowOf(pr, i, pool[c])) layers.insert(layerOf(pool[i]));
              }
              if (layers.size() > bestLayers ||
                  (layers.size() == bestLayers && best && betterQuality(pool[c], best))) {
                best = pool[c];
                bestLayers = layers.size();
              }
            }
            return best;
          };

          // InnerRescueWindow: fill a missing Inner layer with the closest IP-pointing,
          // direction-compatible inner segment of the whole container (any ML component or none)
          // projecting within InnerRescueWindow of the centre. It never replaces a seed member.
          const auto rescueInner = [&](const xAOD::MuonSegment* centre, const Projection& pr,
                                       LayerMap_t& layers) {
            const std::size_t coord = pr.loc == MsTrackSeed::Location::Barrel ? 1 : 0;
            const Amg::Vector2D centreProj =
                m_seedParameterEstimator->expressOnCylinder(*tgContext, *centre, pr.loc, pr.sector);
            const xAOD::MuonSegment* best{nullptr};
            double bestDist = m_innerRescueWindow.value();
            for (const xAOD::MuonSegment* seg : *segments) {
              if (!seg || !isInner(seg) || pointingDeg(seg) > m_innerRescueMaxDeg.value()) continue;
              if (centre->direction().dot(seg->direction()) < m_minCosConsistency.value()) continue;
              const Amg::Vector2D proj =
                  m_seedParameterEstimator->expressOnCylinder(*tgContext, *seg, pr.loc, pr.sector);
              if (!m_seedParameterEstimator->withinBounds(proj, pr.loc)) continue;
              const double dist = std::abs(proj[coord] - centreProj[coord]);
              if (dist <= bestDist) {
                best = seg;
                bestDist = dist;
              }
            }
            if (!best) return;
            if (truthDiag) {
              const xAOD::TruthParticle* owner{nullptr};
              classifySeed(centre, layers, owner);
              ++m_rescue.filled[!owner ? 2 : MuonR4::getTruthMatchedParticle(*best) == owner ? 0 : 1];
            }
            layers.emplace(seedLayer(best), best);
            ++innerRescued;
          };

          // No centre inside the cylinder: the window is not applied.
          const xAOD::MuonSegment* clustered =
              (m_clusterWindowCentre.value() || extra) ? clusterCentre() : nullptr;
          if (extra) {
            // An extra cluster needs a centre inside the cylinder; keep its direction-compatible segments
            if (!clustered) break;
            std::erase_if(pool, [&](const xAOD::MuonSegment* seg) {
              return seg != clustered &&
                     clustered->direction().dot(seg->direction()) < m_minCosConsistency.value();
            });
          }
          const xAOD::MuonSegment* centre = clustered ? clustered : anchor;
          if (failsPointing(centre)) {
            // no IP-pointing cluster left: the anchor itself is a non-pointing inner segment
            ++pointingCentreSkipped;
            lastWindow = {centre};
            continue;
          }
          if (!extra && centre != anchor) ++windowCentreNotAnchor;
          const Projection pr = projectAround(centre);
          if (!extra && !pr.centreInBounds) ++windowCentreOutOfBounds;
          const xAOD::TruthParticle* centrePart = truthDiag ? MuonR4::getTruthMatchedParticle(*centre) : nullptr;

          std::vector<const xAOD::MuonSegment*> inWindow{};
          for (std::size_t i = 0; i < pool.size(); ++i) {
            const bool keep = inWindowOf(pr, i, centre);
            if (!extra && pool[i] != centre && pr.centreInBounds) {
              if (!pr.inBounds[i]) ++windowOutOfBounds;
              else if (!keep) ++windowDropped;
              if (truthDiag) {
                // 0 = same particle as the centre, 1 = another particle or none (also if the centre has none)
                const std::size_t same =
                    !(centrePart && MuonR4::getTruthMatchedParticle(*pool[i]) == centrePart);
                const double dist = pr.dist[i];
                const std::size_t bin = !pr.inBounds[i] ? 5 : dist < 100. ? 0 : dist < 250. ? 1
                                      : dist < 500. ? 2 : dist < 1000. ? 3 : 4;
                ++m_window.distance[same * WindowCounters::kDistBins + bin];
                if (!keep) ++m_window.dropped[same];
              }
            }
            if (keep) inWindow.push_back(pool[i]);
          }
          lastWindow = inWindow;
          LayerMap_t windowed = perLayer(centre, inWindow);
          const bool enoughLayers = nStationLayers(centre, windowed) >= m_minLayers.value();

          if (truthDiag && !extra) {
            const LayerMap_t unwindowed = perLayer(centre, pool);
            if (nStationLayers(centre, unwindowed) >= m_minLayers.value()) {
              const xAOD::TruthParticle* owner{nullptr};
              const std::size_t cls = classifySeed(centre, unwindowed, owner);
              ++m_window.seedsWithout[cls];
              if (owner) windowBefore.insert(owner);
              if (!enoughLayers) {
                ++m_window.seedsLost[cls];
              } else {
                const xAOD::TruthParticle* windowedOwner{nullptr};
                if (classifySeed(centre, windowed, windowedOwner) == 0) ++m_window.seedsPureAfter[cls];
                if (windowedOwner) windowAfter.insert(windowedOwner);
              }
              // Could a window centred on the owner's own segment have kept it?
              if (owner) {
                ++m_window.ownerSeeds;
                if (centrePart == owner) ++m_window.centreOnOwner;
                const xAOD::MuonSegment* ownerCentre{nullptr};
                for (const xAOD::MuonSegment* segment : pool) {
                  if (MuonR4::getTruthMatchedParticle(*segment) != owner) continue;
                  if (!ownerCentre || betterQuality(segment, ownerCentre)) ownerCentre = segment;
                }
                const Projection opr = projectAround(ownerCentre);
                std::set<LayerIndex> ownerLayers;
                for (std::size_t i = 0; i < pool.size(); ++i) {
                  if (MuonR4::getTruthMatchedParticle(*pool[i]) == owner && inWindowOf(opr, i, ownerCentre)) {
                    ownerLayers.insert(layerOf(pool[i]));
                  }
                }
                if (ownerLayers.size() >= m_minLayers.value()) ++m_window.oracleKeeps;
                if (enoughLayers) {
                  std::size_t nOwner = centrePart == owner;
                  for (const auto& [layer, segment] : windowed) {
                    nOwner += MuonR4::getTruthMatchedParticle(*segment) == owner;
                  }
                  if (nOwner >= 2) ++m_window.actualKeeps;
                }
              }
            }
          }
          if (!enoughLayers) {
            ++tooFewLayers;
            if (truthDiag) traceSeed(component, pool, inWindow, centre, windowed, false);
            continue;
          }
          if (extra && !(strongSegment(centre) || std::ranges::any_of(windowed, [&](const auto& entry) {
                return strongSegment(entry.second);
              }))) {
            ++extraNoStrong;
            continue;
          }
          if (windowed.size() == 1) {
            if (!passesTwoSegmentRule(centre, windowed.begin()->second)) {
              if (truthDiag) traceSeed(component, pool, inWindow, centre, windowed, true);
              continue;
            }
            if (truthDiag && pr.centreInBounds) {
              const auto member = std::ranges::find(pool, windowed.begin()->second);
              twoSegDist[outSeeds.size()] = pr.dist[member - pool.begin()];
            }
          }
          // Applied after the MinLayers check and the 2-segment rule: it never creates a seed.
          if (m_innerRescueWindow.value() > 0. && pr.centreInBounds && !isInner(centre) &&
              std::ranges::none_of(windowed, [](const auto& entry) { return entry.first.first == LayerIndex::Inner; })) {
            rescueInner(centre, pr, windowed);
          }
          MsTrackSeed windowSeed{pr.loc, pr.sector};
          windowSeed.addSegment(centre);
          for (const auto& [layer, segment] : windowed) windowSeed.addSegment(segment);
          windowSeed.setPosition(centre->position());
          if (truthDiag) traceSeed(component, pool, inWindow, centre, windowed, false);
          recordStartParameters(windowSeed);
          if (extra) {
            ++extraSeeds;
            if (truthDiag) ++m_anatomy.extraSeedClass[std::min<std::size_t>(classifySegments(windowSeed.segments()), 2)];
          }
          ++seedSegments[std::clamp<std::size_t>(windowSeed.segments().size(), 2, 5) - 2];
          const std::size_t mainClass =
              truthDiag ? std::min<std::size_t>(classifySegments(windowSeed.segments()), 2) : 0;
          outSeeds.push_back(std::move(windowSeed));
          ++constructedSeeds;

          // VariantSeeds: a second, different seed as a fallback in case the fit of this one fails
          if (m_variantSeeds.value() && passesVariantGate(centre, windowed, pr.loc)) {
            LayerMap_t variant = buildVariant(centre, windowed, inWindow);
            if (variant.empty()) {
              ++variantNone;
            } else {
              MsTrackSeed variantSeed{pr.loc, pr.sector};
              for (const auto& [layer, segment] : variant) variantSeed.addSegment(segment);
              variantSeed.setPosition(centre->position());
              if (truthDiag) {
                const xAOD::MuonSegment* variantCentre = variant.begin()->second;
                variant.erase(variant.begin());
                traceSeed(component, pool, inWindow, variantCentre, variant, false);
                const std::size_t cls = std::min<std::size_t>(classifySegments(variantSeed.segments()), 2);
                ++m_variant.seedClass[cls];
                if (cls == 0 && mainClass != 0) ++m_variant.pureMainNotPure;
              }
              recordStartParameters(variantSeed);
              ++seedSegments[std::clamp<std::size_t>(variantSeed.segments().size(), 2, 5) - 2];
              outSeeds.push_back(std::move(variantSeed));
              ++constructedSeeds;
              ++variantSeeds;
            }
          }
        }
        continue;
      }

      // Without PointingWindow: the anchor and one direction-compatible segment per other layer.
      const MsTrackSeed::Location location =
          Muon::MuonStationIndex::isBarrel(anchor->chamberIndex())
              ? MsTrackSeed::Location::Barrel
              : MsTrackSeed::Location::Endcap;
      MsTrackSeed seed{location, ExpandedSector{anchor->position().phi()}};
      const SeedLayer anchorLayer = seedLayer(anchor);
      LayerMap_t bestPerLayer;
      for (const xAOD::MuonSegment* segment : component.segments) {
        if (segment == anchor) continue;
        const SeedLayer layer = seedLayer(segment);
        if (layer == anchorLayer) continue;
        if (anchor->direction().dot(segment->direction()) < m_minCosConsistency.value()) continue;
        const xAOD::MuonSegment*& best = bestPerLayer[layer];
        if (!best || betterQuality(segment, best)) best = segment;
      }
      if (nStationLayers(anchor, bestPerLayer) < m_minLayers.value()) {
        ++tooFewLayers;
        continue;
      }
      seed.addSegment(anchor);
      for (const auto& [layer, segment] : bestPerLayer) seed.addSegment(segment);
      seed.setPosition(anchor->position());
      if (bestPerLayer.size() == 1 && !passesTwoSegmentRule(anchor, bestPerLayer.begin()->second)) continue;
      recordStartParameters(seed);
      ++seedSegments[std::clamp<std::size_t>(seed.segments().size(), 2, 5) - 2];
      outSeeds.push_back(std::move(seed));
      ++constructedSeeds;
    }
    ATH_MSG_VERBOSE("Constructed " << component.anchors.size()
                    << " seed(s) from ML component " << componentId
                    << " with " << component.segments.size()
                    << " segments");
  }

  ATH_MSG_DEBUG("Constructed " << constructedSeeds << " seed(s) from "
                << acceptedComponents << "/" << components.size()
                << " decorated ML component(s)");
  if (diag) {
    ++m_counters.events;
    m_counters.components += components.size();
    m_counters.rejectedTooSmall += tooSmall;
    m_counters.rejectedTooLarge += tooLarge;
    m_counters.rejectedNoAnchor += noAnchor;
    m_counters.acceptedComponents += acceptedComponents;
    m_counters.anchorsTooFewLayers += tooFewLayers;
    m_counters.windowCentreNotAnchor += windowCentreNotAnchor;
    m_counters.windowCentreOutOfBounds += windowCentreOutOfBounds;
    m_counters.windowDropped += windowDropped;
    m_counters.windowOutOfBounds += windowOutOfBounds;
    m_counters.twoSegLowScore += twoSegLowScore;
    m_counters.twoSegNoScore += twoSegNoScore;
    m_counters.extraSeeds += extraSeeds;
    m_counters.extraNoStrong += extraNoStrong;
    m_counters.pointingRemoved += pointingRemoved;
    m_counters.pointingCentreSkipped += pointingCentreSkipped;
    m_counters.innerRescued += innerRescued;
    m_counters.variantSeeds += variantSeeds;
    m_counters.variantNone += variantNone;
    if (truthDiag) {
      // anatomy of the seeds handed to MsTrackFindingAlg
      std::vector<std::uint8_t> classes(outSeeds.size(), 255);
      for (std::size_t i = outSeedsBegin; i < outSeeds.size(); ++i) {
        classes[i] = classifySegments(outSeeds[i].segments());
      }
      for (std::size_t i = outSeedsBegin; i < outSeeds.size(); ++i) {
        const auto segs = outSeeds[i].segments();
        const std::size_t cls = classes[i];
        ++m_anatomy.nSegments[cls * 3 + std::min<std::size_t>(segs.size(), 4) - 2];
        const double spread = thetaSpreadDeg(segs);
        ++m_anatomy.thetaSpread[cls * 4 + (spread < 2. ? 0 : spread < 4. ? 1 : spread < 8. ? 2 : 3)];
      }
      // Descriptors of every seed and for the pure 2-segment seeds
      SG::ReadDecorHandle<xAOD::MuonSegmentContainer, std::vector<float>> scoreAcc{m_componentScoreKey, ctx};
      const bool haveScores = scoreAcc.isAvailable();
      const auto scoreBin = [](float score) -> std::uint8_t {
        return score < 0.f ? 4 : score < 0.05f ? 0 : score < 0.3f ? 1 : score < 0.8f ? 2 : 3;
      };
      std::vector<std::array<std::uint8_t, 8>> desc(outSeeds.size());
      std::unordered_set<const xAOD::TruthParticle*> ownersWithLongSeed;
      for (std::size_t i = outSeedsBegin; i < outSeeds.size(); ++i) {
        const auto segs = outSeeds[i].segments();
        std::array<std::uint8_t, 8>& d = desc[i];
        d.fill(0);
        {
          // IP pointing of the seed's inner segment: |theta(direction) - theta(position)|
          using SA = SeedAnatomy;
          const auto inner = std::ranges::find_if(segs, isInner);
          std::size_t bin = SA::kPointBins - 1;
          if (inner != segs.end()) {
            const double dTheta =
                std::abs((*inner)->direction().theta() - (*inner)->position().theta()) / Gaudi::Units::deg;
            bin = dTheta < 1. ? 0 : dTheta < 2. ? 1 : dTheta < 5. ? 2 : dTheta < 10. ? 3 : 4;
          }
          const std::size_t cat = (segs.size() == 2 ? 0 : 2) +
                                  (outSeeds[i].location() == MsTrackSeed::Location::Endcap ? 1 : 0);
          d[7] = cat * SA::kPointBins + bin;
        }
        const std::size_t nMeas = outSeeds[i].nMeasurements();
        d[6] = nMeas < 10 ? 0 : nMeas < 15 ? 1 : nMeas < 20 ? 2 : nMeas < 30 ? 3 : 4;
        if (segs.size() >= 3 && classes[i] <= 1) {
          std::unordered_map<const xAOD::TruthParticle*, unsigned int> counts;
          for (const xAOD::MuonSegment* seg : segs) {
            if (const xAOD::TruthParticle* part = MuonR4::getTruthMatchedParticle(*seg)) ++counts[part];
          }
          for (const auto& [part, count] : counts) {
            if (count >= 2) ownersWithLongSeed.insert(part);
          }
        }
        if (segs.size() != 2) {
          d[0] = 255;
          continue;
        }
        const auto distIt = twoSegDist.find(i);
        const float dist = distIt != twoSegDist.end() ? distIt->second : -1.f;
        d[0] = dist < 0.f ? 3 : dist < 50.f ? 0 : dist < 100.f ? 1 : dist < 250.f ? 2 : 3;
        d[1] = m_segSelector->passSeedingQuality(ctx, *segs[0]) + m_segSelector->passSeedingQuality(ctx, *segs[1]);
        float compScore = -1.f, pairScore = -1.f;
        if (haveScores) {
          const std::vector<float>& s0 = scoreAcc(*segs[0]);
          const std::vector<float>& s1 = scoreAcc(*segs[1]);
          if (s0.size() == 2 && s1.size() == 2) {
            compScore = std::max(s0[0], s1[0]);
            pairScore = std::min(s0[1], s1[1]);
          }
        }
        d[2] = scoreBin(compScore);
        d[3] = scoreBin(pairScore);
        using enum Muon::MuonStationIndex::LayerIndex;
        const LayerIndex l0 = layerOf(segs[0]);
        const LayerIndex l1 = layerOf(segs[1]);
        const auto has = [&](LayerIndex l) { return l0 == l || l1 == l; };
        d[4] = (has(Extended) || has(BarrelExtended)) ? 3
             : (has(Inner) && has(Middle)) ? 0 : (has(Middle) && has(Outer)) ? 1
             : (has(Inner) && has(Outer)) ? 2 : 4;
        d[5] = outSeeds[i].location() == MsTrackSeed::Location::Endcap;
      }
      // Does the owner of a pure 2-segment seed also have a longer seed?
      for (std::size_t i = outSeedsBegin; i < outSeeds.size(); ++i) {
        if (classes[i] != 0 || outSeeds[i].segments().size() != 2) continue;
        const xAOD::TruthParticle* owner = MuonR4::getTruthMatchedParticle(*outSeeds[i].segments()[0]);
        ++m_anatomy.pureTwoSegOwnerHasLonger[ownersWithLongSeed.contains(owner) ? 0 : 1];
      }
      // Remember this event's seeds to time their fits from estimateStartParameters
      std::lock_guard lock{m_slotMutex};
      SlotState& state = m_slots[ctx.slot()];
      if (state.lastIndex >= 0) ++m_anatomy.untimedLastSeeds;
      state.base = outSeeds.data();
      state.seedClass = std::move(classes);
      state.startOk.assign(outSeeds.size(), 0);
      state.pBin.assign(outSeeds.size(), 0);
      state.desc = std::move(desc);
      state.lastIndex = -1;
    }
    if (truthDiag && useWindow) {
      using EF = EtaFunnel;
      for (const auto& [part, trace] : traces) {
        const double absEta = std::abs(part->eta());
        const std::size_t region = absEta < 1.05 ? 0 : absEta < 1.3 ? 1 : absEta < 2.0 ? 2 : 3;
        if (trace.layers.size() < 2) {
          ++m_etaFunnel.notSeedable[region];
          continue;
        }
        ++m_etaFunnel.stage[region * EF::kStages + trace.stage];
        if (trace.stage != 7) continue;
        bool anyMissing = false;
        for (std::size_t r = 0; r < EF::kMissReasons; ++r) {
          m_etaFunnel.missingLayers[region * EF::kMissReasons + r] += trace.missing[r];
          anyMissing |= trace.missing[r] > 0;
        }
        if (anyMissing) ++m_etaFunnel.withMissing[region];
        m_etaFunnel.fakes[region * 2] += trace.fakes[0];
        m_etaFunnel.fakes[region * 2 + 1] += trace.fakes[1];
        if (trace.fakes[0] + trace.fakes[1] > 0) ++m_etaFunnel.withFake[region];
      }
      m_window.particlesSeeded += windowBefore.size();
      m_window.particlesLost += std::ranges::count_if(windowBefore,
          [&](const xAOD::TruthParticle* part) { return !windowAfter.contains(part); });
    }
    m_counters.seeds += constructedSeeds;
    for (std::size_t i = 0; i < seedSegments.size(); ++i) m_counters.seedSegments[i] += seedSegments[i];
    m_counters.findSeedsNs += std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - start).count();
  }
  return StatusCode::SUCCESS;
}

Acts::Result<Acts::BoundTrackParameters>
MlMsTrackSeeder::estimateStartParameters(
    const EventContext& ctx,
    const MsTrackSeed& seed) const {
  if (!msgLvl(MSG::DEBUG)) {
    return m_seedParameterEstimator->estimateStartParameters(ctx, seed);
  }
  const auto start = std::chrono::steady_clock::now();
  // The time since the previous call is the fit of the previous seed in MsTrackFindingAlg
  const bool timeFits = m_truthDiag.value();
  if (timeFits) {
    std::lock_guard lock{m_slotMutex};
    SlotState& state = m_slots[ctx.slot()];
    if (state.lastIndex >= 0) {
      const std::size_t slot = state.seedClass[state.lastIndex] * 2 + state.startOk[state.lastIndex];
      const std::size_t cls = state.seedClass[state.lastIndex];
      if (cls < SeedAnatomy::kClasses) {
        const std::uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            start - state.lastEnd).count();
        m_anatomy.fitNs[slot] += ns;
        ++m_anatomy.fitCount[slot];
        // fitted seeds (valid start parameters) by momentum estimate and size
        if (state.startOk[state.lastIndex]) {
          const std::size_t pSlot = cls * SeedAnatomy::kPBins + state.pBin[state.lastIndex];
          m_anatomy.fitNsByP[pSlot] += ns;
          ++m_anatomy.fitCountByP[pSlot];
          if (cls == 0 || cls >= 2) {
            const std::size_t sSlot = (cls == 0 ? 0 : 2) + (state.lastNSeg >= 3);
            m_anatomy.fitNsBySegs[sSlot] += ns;
            ++m_anatomy.fitCountBySegs[sSlot];
          }
          if (static_cast<std::size_t>(state.lastIndex) < state.desc.size()) {
            using SA = SeedAnatomy;
            const auto& d = state.desc[state.lastIndex];
            const std::size_t g = std::min<std::size_t>(cls, 2);
            m_anatomy.measN[g * SA::kMeasBins + d[6]] += 1;
            m_anatomy.measNs[g * SA::kMeasBins + d[6]] += ns;
            m_anatomy.pointN[g * SA::kPointCats * SA::kPointBins + d[7]] += 1;
            m_anatomy.pointNs[g * SA::kPointCats * SA::kPointBins + d[7]] += ns;
            if (d[0] != 255) {
              ++m_anatomy.twoSegDistN[g * SA::kDistBins + d[0]];
              m_anatomy.twoSegDistNs[g * SA::kDistBins + d[0]] += ns;
              ++m_anatomy.twoSegQualN[g * SA::kQualBins + d[1]];
              m_anatomy.twoSegQualNs[g * SA::kQualBins + d[1]] += ns;
              ++m_anatomy.twoSegCompScoreN[g * SA::kScoreBins + d[2]];
              m_anatomy.twoSegCompScoreNs[g * SA::kScoreBins + d[2]] += ns;
              ++m_anatomy.twoSegPairScoreN[g * SA::kScoreBins + d[3]];
              m_anatomy.twoSegPairScoreNs[g * SA::kScoreBins + d[3]] += ns;
              ++m_anatomy.twoSegLayersN[g * SA::kPairBins + d[4]];
              m_anatomy.twoSegLayersNs[g * SA::kPairBins + d[4]] += ns;
              ++m_anatomy.twoSegRegionN[g * 2 + d[5]];
              m_anatomy.twoSegRegionNs[g * 2 + d[5]] += ns;
              const std::size_t joint = (g * SA::kDistBins + d[0]) * SA::kScoreBins + d[2];
              ++m_anatomy.twoSegJointN[joint];
              m_anatomy.twoSegJointNs[joint] += ns;
            }
          }
        }
      }
      state.lastIndex = -1;
    }
  }
  auto pars = m_seedParameterEstimator->estimateStartParameters(ctx, seed);
  m_counters.startParNs += std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now() - start).count();
  ++m_counters.startParCalls;
  m_counters.startParFailures += !pars.ok();
  if (timeFits) {
    std::lock_guard lock{m_slotMutex};
    SlotState& state = m_slots[ctx.slot()];
    const long index = state.base ? static_cast<long>(&seed - state.base) : -1;
    if (index >= 0 && index < static_cast<long>(state.seedClass.size())) {
      state.startOk[index] = pars.ok();
      if (pars.ok()) {
        const double p = (*pars).absoluteMomentum() / Acts::UnitConstants::GeV;
        state.pBin[index] = p < 3. ? 0 : p < 6. ? 1 : p < 10. ? 2 : p < 20. ? 3 : 4;
      }
      state.lastNSeg = seed.segments().size();
      state.lastIndex = index;
      state.lastEnd = std::chrono::steady_clock::now();
    }
  }
  return pars;
}

StatusCode MlMsTrackSeeder::finalize() {
  if (m_counters.events == 0) return StatusCode::SUCCESS;
  const double nEvents = m_counters.events.load();
  ATH_MSG_DEBUG("Summary (job-summed over " << m_counters.events.load()
                << " events): components=" << m_counters.components.load()
                << ", rejected: <MinSegmentsPerCandidate(" << m_minSegmentsPerCandidate.value()
                << ")=" << m_counters.rejectedTooSmall.load()
                << " >MaxSegmentsPerCandidate(" << m_maxSegmentsPerCandidate.value()
                << ")=" << m_counters.rejectedTooLarge.load()
                << " noAnchor=" << m_counters.rejectedNoAnchor.load()
                << "; accepted=" << m_counters.acceptedComponents.load()
                << ", anchors with <MinLayers(" << m_minLayers.value() << ")="
                << m_counters.anchorsTooFewLayers.load()
                << ", PointingWindow=" << m_pointingWindow.value() << " mm (0 = off), ClusterWindowCentre="
                << m_clusterWindowCentre.value() << ": centre not the anchor=" << m_counters.windowCentreNotAnchor.load()
                << ", centre out of bounds (no window)=" << m_counters.windowCentreOutOfBounds.load()
                << ", segments outside window=" << m_counters.windowDropped.load()
                << ", segments projecting out of bounds=" << m_counters.windowOutOfBounds.load()
                << ", 2-segment rule (TwoSegmentMinComponentScore=" << m_twoSegMinScore.value()
                << ", 0 = off): dropped=" << m_counters.twoSegLowScore.load()
                << ", kept without score=" << m_counters.twoSegNoScore.load()
                << ", ExtraClusterSeeds=" << m_extraClusterSeeds.value() << " (0 = off, ExtraClusterMinScore="
                << m_extraClusterMinScore.value() << "): extra seeds=" << m_counters.extraSeeds.load()
                << ", extra clusters without a strong segment=" << m_counters.extraNoStrong.load()
                << ", InnerPointingMaxDeg=" << m_innerPointingMaxDeg.value()
                << " (0 = off): non-pointing endcap inner segments removed=" << m_counters.pointingRemoved.load()
                << ", clusters skipped (non-pointing centre)=" << m_counters.pointingCentreSkipped.load()
                << ", InnerRescueWindow=" << m_innerRescueWindow.value() << " mm (0 = off, InnerRescueMaxDeg="
                << m_innerRescueMaxDeg.value() << "): inner layers filled=" << m_counters.innerRescued.load()
                << ", VariantSeeds=" << m_variantSeeds.value() << " (MinLayers=" << m_variantMinLayers.value()
                << ", MaxPointingDeg=" << m_variantMaxPointingDeg.value() << ", MinScore=" << m_variantMinScore.value()
                << "): variant seeds=" << m_counters.variantSeeds.load()
                << ", gated seeds without a different variant=" << m_counters.variantNone.load()
                << ", seeds=" << m_counters.seeds.load()
                << " (" << m_counters.seeds.load() / nEvents << "/event)"
                << ", seed segments 2/3/4/5+: " << m_counters.seedSegments[0].load() << "/"
                << m_counters.seedSegments[1].load() << "/" << m_counters.seedSegments[2].load()
                << "/" << m_counters.seedSegments[3].load());
  ATH_MSG_DEBUG("Timing: findTrackSeeds " << m_counters.findSeedsNs.load() * 1e-6 / nEvents
                << " ms/event; estimateStartParameters calls=" << m_counters.startParCalls.load()
                << " failed=" << m_counters.startParFailures.load()
                << ", " << m_counters.startParNs.load() * 1e-6 / nEvents << " ms/event"
                << ". The track fit itself runs in MSTrackFinderAlg and is not timed here: "
                   "fit cost per seed ~ (MSTrackFinderAlg PerfMon time) / seeds.");
  if (m_truthDiag.value()) {
    const auto row = [](const auto& counters) {
      std::stringstream sstr{};
      sstr << counters[0].load() << "/" << counters[1].load() << "/" << counters[2].load();
      return sstr.str();
    };
    if (m_pointingWindow.value() > 0.) {
      const auto distRow = [&](std::size_t same) {
        std::stringstream sstr{};
        for (std::size_t b = 0; b < WindowCounters::kDistBins; ++b) {
          sstr << (b ? "/" : "") << m_window.distance[same * WindowCounters::kDistBins + b].load();
        }
        return sstr.str();
      };
      ATH_MSG_DEBUG("PointingWindow truth diagnostics (W=" << m_pointingWindow.value()
                    << " mm): projected distance to the window centre <100/100-250/250-500/500-1000/>=1000 mm/out of bounds"
                    << ": same particle as centre=" << distRow(0) << ", other/no particle=" << distRow(1)
                    << "; dropped same/other=" << m_window.dropped[0].load() << "/" << m_window.dropped[1].load()
                    << "; seeds without window pure/partial/junk=" << row(m_window.seedsWithout)
                    << ", of which lost (<MinLayers) with window=" << row(m_window.seedsLost)
                    << ", of which pure with window=" << row(m_window.seedsPureAfter)
                    << "; truth particles with a pure/partial seed without window=" << m_window.particlesSeeded.load()
                    << ", of which none with window=" << m_window.particlesLost.load());
      ATH_MSG_DEBUG("  Window centring (first cluster): seeds with a truth owner=" << m_window.ownerSeeds.load()
                    << ", centre on the owner=" << m_window.centreOnOwner.load()
                    << ", an owner-centred window would keep >= MinLayers owner layers=" << m_window.oracleKeeps.load()
                    << ", the emitted seed holds >= 2 owner segments=" << m_window.actualKeeps.load());

      using EF = EtaFunnel;
      constexpr std::array<const char*, EF::kRegions> regions{
          "barrel |eta|<1.05", "transition 1.05-1.3", "endcap 1.3-2.0", "endcap 2.0-2.5"};
      ATH_MSG_DEBUG("D12 seeding funnel per truth muon (pT>20 GeV, segments in >=2 layers), furthest stage "
                    "over all ML components: 0 not in ML/1 split over components/2 component rejected/"
                    "3 cos cut/4 window/5 layer pick/6 2-seg score/7 seeded imperfect/8 seeded clean");
      for (std::size_t r = 0; r < EF::kRegions; ++r) {
        std::stringstream stages{}, missing{};
        std::size_t total = 0;
        for (std::size_t s = 0; s < EF::kStages; ++s) {
          const std::size_t n = m_etaFunnel.stage[r * EF::kStages + s].load();
          stages << (s ? "/" : "") << n;
          total += n;
        }
        for (std::size_t m = 0; m < EF::kMissReasons; ++m) {
          missing << (m ? "/" : "") << m_etaFunnel.missingLayers[r * EF::kMissReasons + m].load();
        }
        ATH_MSG_DEBUG("  " << regions[r] << ": muons=" << total << " (segments in <2 layers: "
                      << m_etaFunnel.notSeedable[r].load() << "), stages=" << stages.str()
                      << " | stage 7: muons missing a layer=" << m_etaFunnel.withMissing[r].load()
                      << ", missing layers other component or not ML/cos cut/window/centre layer/quality pick="
                      << missing.str() << ", muons with a fake=" << m_etaFunnel.withFake[r].load()
                      << ", fake segments background/other particle=" << m_etaFunnel.fakes[r * 2].load()
                      << "/" << m_etaFunnel.fakes[r * 2 + 1].load());
      }
    }
    {
      constexpr std::array<const char*, SeedAnatomy::kClasses> pClasses{"pure", "partial", "junkNoTruth", "junkMixed"};
      ATH_MSG_DEBUG("momentum estimate of the start parameters (<3/3-6/6-10/10-20/>=20 GeV), "
                    "seeds with valid start parameters; fit time [ms/fit] per bin:");
      for (std::size_t c = 0; c < SeedAnatomy::kClasses; ++c) {
        std::stringstream seeds{}, ms{};
        for (std::size_t b = 0; b < SeedAnatomy::kPBins; ++b) {
          const std::size_t slot = c * SeedAnatomy::kPBins + b;
          const std::size_t n = m_anatomy.fitCountByP[slot].load();
          seeds << (b ? "/" : "") << m_anatomy.seedsByP[slot].load();
          ms << (b ? "/" : "") << (n ? m_anatomy.fitNsByP[slot].load() * 1e-6 / n : 0.);
        }
        ATH_MSG_DEBUG("  " << pClasses[c] << ": seeds=" << seeds.str() << ", fit time=" << ms.str());
      }
      const auto msPer = [&](std::size_t slot) {
        const std::size_t n = m_anatomy.fitCountBySegs[slot].load();
        return n ? m_anatomy.fitNsBySegs[slot].load() * 1e-6 / n : 0.;
      };
      ATH_MSG_DEBUG("  Fit time by seed size [ms/fit] pure 2 / 3+ segments=" << msPer(0) << " / " << msPer(1)
                    << " (n=" << m_anatomy.fitCountBySegs[0].load() << "/" << m_anatomy.fitCountBySegs[1].load()
                    << "), junk 2 / 3+ segments=" << msPer(2) << " / " << msPer(3)
                    << " (n=" << m_anatomy.fitCountBySegs[2].load() << "/" << m_anatomy.fitCountBySegs[3].load() << ")");
      ATH_MSG_DEBUG("  Pure seeds without valid start parameters=" << m_anatomy.pureNoStartPars.load()
                    << ": no / some segment passes the seeding quality=" << m_anatomy.pureNoStartParsQuality[0].load()
                    << " / " << m_anatomy.pureNoStartParsQuality[1].load()
                    << ", without / with an NSW segment=" << m_anatomy.pureNoStartParsNsw[0].load()
                    << " / " << m_anatomy.pureNoStartParsNsw[1].load());
      // fits (ms/fit)" per bin for the groups pure / partial / junk
      const auto table = [](const auto& counts, const auto& ns, std::size_t g, std::size_t nBins) {
        std::stringstream sstr{};
        for (std::size_t b = 0; b < nBins; ++b) {
          const std::size_t n = counts[g * nBins + b].load();
          sstr << (b ? " / " : "") << n << " (" << (n ? ns[g * nBins + b].load() * 1e-6 / n : 0.) << ")";
        }
        return sstr.str();
      };
      constexpr std::array<const char*, SeedAnatomy::kGroups> groups{"pure", "partial", "junk"};
      ATH_MSG_DEBUG("Fitted 2-segment seeds, \"fits (ms/fit)\" per bin");
      for (std::size_t g = 0; g < SeedAnatomy::kGroups; ++g) {
        using SA = SeedAnatomy;
        ATH_MSG_DEBUG("  " << groups[g]
                      << ": projected distance <50/50-100/100-250/>=250 or n.a. mm: "
                      << table(m_anatomy.twoSegDistN, m_anatomy.twoSegDistNs, g, SA::kDistBins)
                      << " | segments passing seeding quality 0/1/2: "
                      << table(m_anatomy.twoSegQualN, m_anatomy.twoSegQualNs, g, SA::kQualBins)
                      << " | component best cross-layer edge <0.05/0.05-0.3/0.3-0.8/>=0.8/n.a.: "
                      << table(m_anatomy.twoSegCompScoreN, m_anatomy.twoSegCompScoreNs, g, SA::kScoreBins)
                      << " | weaker segment's best cross-layer edge (same bins): "
                      << table(m_anatomy.twoSegPairScoreN, m_anatomy.twoSegPairScoreNs, g, SA::kScoreBins)
                      << " | layers I-M/M-O/I-O/with BEE-EE/other: "
                      << table(m_anatomy.twoSegLayersN, m_anatomy.twoSegLayersNs, g, SA::kPairBins)
                      << " | barrel/endcap: "
                      << table(m_anatomy.twoSegRegionN, m_anatomy.twoSegRegionNs, g, 2));
      }
      ATH_MSG_DEBUG("Fitted seeds by number of measurements <10/10-15/15-20/20-30/>=30, "
                    "\"fits (ms/fit)\": pure: " << table(m_anatomy.measN, m_anatomy.measNs, 0, SeedAnatomy::kMeasBins)
                    << " | partial: " << table(m_anatomy.measN, m_anatomy.measNs, 1, SeedAnatomy::kMeasBins)
                    << " | junk: " << table(m_anatomy.measN, m_anatomy.measNs, 2, SeedAnatomy::kMeasBins));
      {
        using SA = SeedAnatomy;
        constexpr std::array<const char*, SA::kDistBins> distNames{"<50", "50-100", "100-250", ">=250/n.a."};
        ATH_MSG_DEBUG("Fitted 2-segment seeds, projected distance [mm] x component best "
                      "cross-layer edge (<0.05/0.05-0.3/0.3-0.8/>=0.8/n.a.), \"fits (ms/fit)\"");
        for (std::size_t g = 0; g < SA::kGroups; ++g) {
          for (std::size_t db = 0; db < SA::kDistBins; ++db) {
            ATH_MSG_DEBUG("  " << groups[g] << " dist " << distNames[db] << ": "
                          << table(m_anatomy.twoSegJointN, m_anatomy.twoSegJointNs, g * SA::kDistBins + db, SA::kScoreBins));
          }
        }
        ATH_MSG_DEBUG("2-segment rule: seeds dropped pure/partial/junk="
                      << m_anatomy.twoSegDropped[0].load() << "/" << m_anatomy.twoSegDropped[1].load() << "/"
                      << m_anatomy.twoSegDropped[2].load());
      }
      {
        using SA = SeedAnatomy;
        constexpr std::array<const char*, SA::kPointCats> cats{"2-seg barrel", "2-seg endcap", ">=3-seg barrel", ">=3-seg endcap"};
        ATH_MSG_DEBUG("Fitted seeds by IP pointing of the inner segment |theta(dir)-theta(pos)| "
                      "<1/1-2/2-5/5-10/>=10 deg/no inner segment, \"fits (ms/fit)\"");
        for (std::size_t g = 0; g < SA::kGroups; ++g) {
          for (std::size_t c = 0; c < SA::kPointCats; ++c) {
            ATH_MSG_DEBUG("  " << groups[g] << " " << cats[c] << ": "
                          << table(m_anatomy.pointN, m_anatomy.pointNs, g * SA::kPointCats + c, SA::kPointBins));
          }
        }
      }
      if (m_extraClusterSeeds.value() > 0) {
        ATH_MSG_DEBUG("ExtraClusterSeeds: extra seeds pure/partial/junk=" << row(m_anatomy.extraSeedClass));
      }
      if (m_innerPointingMaxDeg.value() > 0.) {
        ATH_MSG_DEBUG("InnerPointingMaxDeg: non-pointing endcap inner segments removed from first-cluster pools, "
                      "truth-matched/no truth=" << m_rescue.pointingRemoved[0].load() << "/"
                      << m_rescue.pointingRemoved[1].load());
      }
      if (m_variantSeeds.value()) {
        ATH_MSG_DEBUG("VariantSeeds: variant seeds pure/partial/junk=" << row(m_variant.seedClass)
                      << ", pure variants whose main seed is not pure=" << m_variant.pureMainNotPure.load());
      }
      if (m_innerRescueWindow.value() > 0.) {
        ATH_MSG_DEBUG("InnerRescueWindow: inner layers filled with a segment of the seed's owner/another "
                      "particle or none/seed without owner=" << row(m_rescue.filled));
      }
      ATH_MSG_DEBUG("Pure 2-segment seeds: the owner also has a >=3-segment seed="
                    << m_anatomy.pureTwoSegOwnerHasLonger[0].load()
                    << ", the owner has no >=3-segment seed=" << m_anatomy.pureTwoSegOwnerHasLonger[1].load());
    }
    // Seeds handed to MsTrackFindingAlg, by class pure / partial / junk-noTruth / junk-mixed
    constexpr std::array<const char*, SeedAnatomy::kClasses> classes{"pure", "partial", "junkNoTruth", "junkMixed"};
    const auto bins = [](const auto& counters, std::size_t cls, std::size_t nBins) {
      std::stringstream sstr{};
      for (std::size_t b = 0; b < nBins; ++b) sstr << (b ? "/" : "") << counters[cls * nBins + b].load();
      return sstr.str();
    };
    ATH_MSG_DEBUG("Seed anatomy (fit time = wall time between consecutive "
                  "estimateStartParameters calls, i.e. the previous seed's fit in MSTrackFinderAlg; "
                  "last seed of each event untimed=" << m_anatomy.untimedLastSeeds.load() << ")");
    for (std::size_t c = 0; c < SeedAnatomy::kClasses; ++c) {
      const auto msPerFit = [&](std::size_t ok) {
        const std::size_t n = m_anatomy.fitCount[c * 2 + ok].load();
        return n ? m_anatomy.fitNs[c * 2 + ok].load() * 1e-6 / n : 0.;
      };
      ATH_MSG_DEBUG("  " << classes[c]
                    << ": timed fits startPars ok=" << m_anatomy.fitCount[c * 2 + 1].load()
                    << " (" << msPerFit(1) << " ms/fit, " << m_anatomy.fitNs[c * 2 + 1].load() * 1e-6 / nEvents
                    << " ms/event), startPars failed=" << m_anatomy.fitCount[c * 2].load()
                    << " (" << msPerFit(0) << " ms/fit)"
                    << "; segments 2/3/4+=" << bins(m_anatomy.nSegments, c, 3)
                    << ", theta spread <2/2-4/4-8/>=8 deg=" << bins(m_anatomy.thetaSpread, c, 4));
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace MuonR4
