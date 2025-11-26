
/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GraphSPFilterTool.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace MuonML{
    StatusCode GraphSPFilterTool::runGraphInference(const EventContext& ctx,
                                                        GraphRawData& graphData) const {

        const MuonR4::SpacePointContainer* inputSpacePoints{nullptr};
        ATH_CHECK(SG::get(inputSpacePoints, m_readKey, ctx));

        SG::WriteHandle<MuonR4::SpacePointContainer> filteredSpacePoints{m_writeKey, ctx};
        ATH_CHECK(filteredSpacePoints.record(std::make_unique<MuonR4::SpacePointContainer>()));

        if (inputSpacePoints->empty()) {
            ATH_MSG_DEBUG("No input space points found.");
            return StatusCode::SUCCESS;
        }
        ATH_CHECK(buildGraph(ctx, graphData));

        ATH_CHECK(runInference(graphData));
    
        size_t predictionIndex = 0;
        auto outputPredictions = graphData.graph->dataTensor[2].GetTensorMutableData<float>();

        ATH_MSG_DEBUG("Cut value: " << m_filterCut);
    
        for (const MuonR4::SpacePointBucket* bucket : *inputSpacePoints) {
            std::unique_ptr<MuonR4::SpacePointBucket> filteredBucket = std::make_unique<MuonR4::SpacePointBucket>(*bucket);
            filteredBucket->clear();
            LayerSpBucket mlBucket{*bucket};  // We are ordering twice... this must be optimized! 

            for (size_t i = 0; i < bucket->size(); ++i, ++predictionIndex) {
                ATH_MSG_DEBUG("Prediction[" << predictionIndex << "]: " << outputPredictions[predictionIndex]);
                if (outputPredictions[predictionIndex] > m_filterCut) {
                    filteredBucket->push_back(std::make_unique<MuonR4::SpacePoint>(*(mlBucket)[i]));
                }
            }

            ATH_MSG_DEBUG("Bucket size before filtering: " << mlBucket.size() 
                            << ", after filtering: " << filteredBucket->size() );
            if (filteredBucket->size()) {
                filteredSpacePoints->push_back(std::move(filteredBucket));
            }
        }

        return StatusCode::SUCCESS;
    }

    StatusCode GraphSPFilterTool::initialize() {
        ATH_CHECK(setupModel());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
}
