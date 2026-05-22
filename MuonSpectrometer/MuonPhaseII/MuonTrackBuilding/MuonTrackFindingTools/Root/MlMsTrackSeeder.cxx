#include "MuonTrackFindingTools/MlMsTrackSeeder.h"
#include <AthContainers/ConstDataVector.h>
#include "AthContainers/AuxElement.h"
#include "CxxUtils/checker_macros.h"
#include <algorithm>
#include <unordered_map>
#include <set>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

namespace MuonR4 {

MlMsTrackSeeder::MlMsTrackSeeder(const std::string& msgName, Config&& cfg)
  : AthMessaging{msgName + ".MlMsTrackSeeder"},
    m_cfg{std::move(cfg)},
    m_baselineSeeder{msgName + ".BaselineMsTrackSeeder", MsTrackSeeder::Config{m_cfg.baselineSeeder}} {}

std::unique_ptr<MsTrackSeedContainer> MlMsTrackSeeder::findTrackSeeds(const EventContext& ctx,
                                                                      const ActsTrk::GeometryContext& gctx,
                                                                      const xAOD::MuonSegmentContainer& segments) const {
  SG::AuxElement::ConstAccessor<std::vector<unsigned>> acc{m_cfg.candidateDecoration};
  std::unordered_map<unsigned, std::vector<const xAOD::MuonSegment*>> groups;
  groups.reserve(segments.size());

  bool sawDecor = false;
  for (const xAOD::MuonSegment* seg : segments) {
    if (!seg || !acc.isAvailable(*seg)) continue;
    sawDecor = true;
    for (const unsigned id : acc(*seg)) groups[id].push_back(seg);
  }

  if (!sawDecor) {
    ATH_MSG_DEBUG("MlMsTrackSeeder: no ML decoration found on any segment."
                  << (m_cfg.fallbackToBaselineIfUndecorated ? " Falling back to baseline seeder." : " Returning empty seed set."));
    if (m_cfg.fallbackToBaselineIfUndecorated) return m_baselineSeeder.findTrackSeeds(ctx, gctx, segments);
    return std::make_unique<MsTrackSeedContainer>();
  }

  ATH_MSG_DEBUG("MlMsTrackSeeder: " << groups.size() << " ML candidate group(s) from "
                << segments.size() << " segment(s)");

  auto out = std::make_unique<MsTrackSeedContainer>();

  std::vector<unsigned> orderedIds;
  orderedIds.reserve(groups.size());
  for (const auto& [id, _] : groups) orderedIds.push_back(id);
  std::sort(orderedIds.begin(), orderedIds.end());

  struct CandidateResult {
    unsigned id{0};
    std::size_t nSegments{0};
    std::unique_ptr<MsTrackSeedContainer> seeds{};
  };

  std::vector<CandidateResult> results(orderedIds.size());

  auto runOneCandidate = [&](std::size_t idx) {
    const unsigned id = orderedIds[idx];
    auto& segs = groups[id];
    std::sort(segs.begin(), segs.end());
    segs.erase(std::unique(segs.begin(), segs.end()), segs.end());
    results[idx].id = id;
    results[idx].nSegments = segs.size();

    if (segs.size() < m_cfg.minSegmentsPerCandidate) {
      return;
    }

    ConstDataVector<xAOD::MuonSegmentContainer> viewCont{SG::VIEW_ELEMENTS};
    for (const xAOD::MuonSegment* seg : segs) {
      viewCont.push_back(seg);
    }
    results[idx].seeds = m_baselineSeeder.findTrackSeeds(ctx, gctx, *viewCont.asDataVector());
  };

  if (m_cfg.runCandidatesInParallel && orderedIds.size() > 1) {
    tbb::parallel_for(tbb::blocked_range<std::size_t>(0, orderedIds.size()),
                      [&](const tbb::blocked_range<std::size_t>& range) {
                        for (std::size_t idx = range.begin(); idx != range.end(); ++idx) {
                          runOneCandidate(idx);
                        }
                      });
  } else {
    for (std::size_t idx = 0; idx < orderedIds.size(); ++idx) {
      runOneCandidate(idx);
    }
  }

  for (CandidateResult& result : results) {
    const std::size_t nSeeds = result.seeds ? result.seeds->size() : 0;
    ATH_MSG_DEBUG("  candidate " << result.id << ": "
                  << result.nSegments << " segment(s) -> "
                  << nSeeds << " seed(s)");
    if (!result.seeds) continue;
    for (MsTrackSeed& seed : *result.seeds) {
      out->push_back(std::move(seed));
    }
  }

  // Remove exact duplicates induced by overlapping ML candidate IDs.
  // This is intentionally pointer-based: if two seeds contain the same segment set,
  // they will drive the same downstream fit attempt.
  std::set<std::vector<const xAOD::MuonSegment*>> seen;
  MsTrackSeedContainer uniqueSeeds{};
  for (MsTrackSeed& seed : *out) {
    std::vector<const xAOD::MuonSegment*> key = seed.segments();
    std::sort(key.begin(), key.end());
    if (!seen.insert(key).second) continue;
    uniqueSeeds.push_back(std::move(seed));
  }
  *out = std::move(uniqueSeeds);

  ATH_MSG_DEBUG("MlMsTrackSeeder: total seeds produced = " << out->size()
                << (out->empty() && m_cfg.fallbackToBaselineIfNoCandidates ? " — falling back to baseline seeder" : ""));
  if (out->empty() && m_cfg.fallbackToBaselineIfNoCandidates) return m_baselineSeeder.findTrackSeeds(ctx, gctx, segments);
  return out;
}

} // namespace MuonR4
