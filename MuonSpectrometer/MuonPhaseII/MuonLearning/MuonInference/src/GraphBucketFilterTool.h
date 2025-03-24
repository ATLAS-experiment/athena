#ifndef MUONINFERENCE_GRAPHBUCKETFILTERTOOL_H
#define MUONINFERENCE_GRAPHBUCKETFILTERTOOL_H

#include "GraphInferenceToolBase.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonSpacePoint/SpacePointContainer.h"

namespace MuonML{
    class GraphBucketFilterTool : public GraphInferenceToolBase {
        public:
            using GraphInferenceToolBase::GraphInferenceToolBase;

            virtual StatusCode runGraphInference(const EventContext& ctx,
                                                 GraphRawData& graphData) const override final;


            virtual StatusCode initialize() override final;
        private:
            SG::WriteHandleKey<MuonR4::SpacePointContainer> m_writeKey{this, "WriteSpacePointKey", "FilteredMlSpacePoints"};
            Gaudi::Property<double>                         m_filterCut{this,"MLFilterCut", -7};

    };

}

#endif