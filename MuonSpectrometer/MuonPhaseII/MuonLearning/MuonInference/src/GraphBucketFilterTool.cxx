

#include "GraphBucketFilterTool.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace MuonML{
    StatusCode GraphBucketFilterTool::runGraphInference(const EventContext& ctx,
                                                        GraphRawData& graphData) const {

        ATH_CHECK(buildGraph(ctx, graphData));
        ATH_CHECK(runInference(graphData));
        SG::WriteHandle filteredSpacePoints{m_writeKey, ctx};
        ATH_CHECK(filteredSpacePoints.record(std::make_unique<MuonR4::SpacePointContainer>()));
        //// Inference part
    
        SG::ReadHandle inputSpacePoints{m_readKey, ctx};
        ATH_CHECK(inputSpacePoints.isPresent());

        size_t predictionIndex = 0;
        auto outputPredictions = graphData.graph->dataTensor[2].GetTensorMutableData<float>();

        ATH_MSG_DEBUG("Cut value: " << m_filterCut);
    
        for (const MuonR4::SpacePointBucket* bucket : *inputSpacePoints) {
            std::unique_ptr<MuonR4::SpacePointBucket> filteredBucket = std::make_unique<MuonR4::SpacePointBucket>();

            for (size_t i = 0; i < bucket->size(); ++i, ++predictionIndex) {
                ATH_MSG_DEBUG("Prediction[" << predictionIndex << "]: " << outputPredictions[predictionIndex]);
                if (outputPredictions[predictionIndex] > m_filterCut) {
                    filteredBucket->push_back((*bucket)[i]);
                }
            }

            ATH_MSG_DEBUG("Bucket size before filtering: " << bucket->size() 
                            << ", after filtering: " << filteredBucket->size() );
            
            filteredSpacePoints->push_back(std::move(filteredBucket));
        }

        return StatusCode::SUCCESS;
    }

    StatusCode GraphBucketFilterTool::initialize() {
        ATH_CHECK(setupModel());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
}