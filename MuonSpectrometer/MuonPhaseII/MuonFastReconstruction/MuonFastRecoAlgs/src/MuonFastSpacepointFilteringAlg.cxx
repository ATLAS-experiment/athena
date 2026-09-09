/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastSpacepointFilteringAlg.h"

#include "xAODMuonPrepData/UtilFunctions.h"

namespace MuonR4 {
using namespace Muon::MuonStationIndex;

StatusCode MuonFastSpacepointFilteringAlg::initialize() {
    ATH_CHECK(m_outSpacePoints.initialize());
    ATH_CHECK(m_outNswSpacePoints.initialize(!m_outNswSpacePoints.empty()));
    ATH_CHECK(m_inPatterns.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MuonFastSpacepointFilteringAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE(__func__<<"() Start filtering space points...");

    const GlobalPatternContainer* inPatterns{nullptr};
    ATH_CHECK(SG::get(inPatterns, m_inPatterns, ctx));

    SG::WriteHandle outSpacePoints{m_outSpacePoints, ctx};
    ATH_CHECK(outSpacePoints.record(std::make_unique<SpacePointContainer>()));
    
    SG::WriteHandle outNswSpacePoints{m_outNswSpacePoints, ctx};
    if (!m_outNswSpacePoints.empty()) {
        ATH_CHECK(outNswSpacePoints.record(std::make_unique<SpacePointContainer>()));
    }

    for (const GlobalPattern* pat : *inPatterns) {
        const std::vector<const SpacePointBucket*>& buckets{pat->getParentBuckets()};
        ATH_MSG_DEBUG(__func__<<"() Filtering " << buckets.size() << " space point buckets from pattern " << *pat);

        for (const SpacePointBucket* bucket : buckets) {

            if (isNSW(bucket->front()->type())) {
                if (m_outNswSpacePoints.empty()) {
                    ATH_MSG_WARNING(__func__<<"() Found NSW space points but no output container is defined - skip.");
                    continue;
                }
                outNswSpacePoints->push_back(std::make_unique<SpacePointBucket>(*bucket));
            } else {
                outSpacePoints->push_back(std::make_unique<SpacePointBucket>(*bucket));
            }
        }
    }

    ATH_MSG_DEBUG(__func__<<"() Wrote " << outSpacePoints->size() << " non-NSW and "
        << outNswSpacePoints->size() << " NSW space point buckets into StoreGate.");
    return StatusCode::SUCCESS;
}
}