#include "SegmentEdgeInferenceAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include <algorithm>
#include <limits>

namespace MuonML {

StatusCode SegmentEdgeInferenceAlg::initialize() {
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_candidateDecorKey.initialize());
  ATH_CHECK(m_edgeClassifier.retrieve());
  ATH_CHECK(m_candidateBuilder.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode SegmentEdgeInferenceAlg::execute(const EventContext& ctx) const {
  const xAOD::MuonSegmentContainer* segments{};
  ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": input segments in '" << m_segmentKey.key() << "' = " << segments->size());

  SegmentEdgeGraph graph{};
  std::vector<SegmentEdgeScore> scores{};
  std::vector<std::vector<unsigned>> ids{};
  ATH_CHECK(m_edgeClassifier->buildGraph(ctx, *segments, graph));
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": built graph with nodes=" << graph.nNodes << ", edges=" << graph.nEdges);

  ATH_CHECK(m_edgeClassifier->classifyEdges(ctx, graph, scores));
  if (!scores.empty()) {
    float minProb = std::numeric_limits<float>::max();
    float maxProb = std::numeric_limits<float>::lowest();
    for (const SegmentEdgeScore& s : scores) {
      minProb = std::min(minProb, s.probability);
      maxProb = std::max(maxProb, s.probability);
    }
    ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                  << ": edge scores=" << scores.size()
                  << ", prob range=[" << minProb << ", " << maxProb << "]");
  } else {
    ATH_MSG_DEBUG("Event " << ctx.eventID().event_number() << ": no edge scores produced");
  }

  ATH_CHECK(m_candidateBuilder->buildCandidates(ctx, graph, scores, ids));
  std::size_t nonEmptyCandidates = 0;
  std::size_t maxCandidateList = 0;
  for (const auto& segIds : ids) {
    if (!segIds.empty()) ++nonEmptyCandidates;
    maxCandidateList = std::max(maxCandidateList, segIds.size());
  }
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": decorated segments=" << ids.size()
                << ", non-empty candidate lists=" << nonEmptyCandidates
                << ", max candidates/segment=" << maxCandidateList);

  SG::WriteDecorHandle<xAOD::MuonSegmentContainer, std::vector<unsigned>> decor{m_candidateDecorKey, ctx};
  // Initializing on one element creates the aux decoration column; remaining
  // entries are default-constructed by the EDM unless overwritten below.
  if (!segments->empty()) {
    decor(*(*segments)[0]) = {};
  }
  for (std::size_t i = 0; i < graph.segments.size(); ++i) {
    decor(*graph.segments[i]) = ids[i];
  }
  return StatusCode::SUCCESS;
}
} // namespace MuonML
