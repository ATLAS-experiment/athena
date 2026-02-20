/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONINFERENCE_GRAPHSPFILTERTOOL_H
#define MUONINFERENCE_GRAPHSPFILTERTOOL_H

#include "SPInferenceToolBase.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonSpacePoint/SpacePointContainer.h"

namespace MuonML{
    class GraphSPFilterTool : public SPInferenceToolBase {
        public:
            using SPInferenceToolBase::SPInferenceToolBase;

            virtual StatusCode runGraphInference(   const EventContext& ctx,
                                                    GraphRawData& graphData) const override final;

            virtual StatusCode initialize() override final;
        private:
            SG::ReadHandleKey<MuonR4::SpacePointContainer> m_readKey{this, "ReadSpacePointKey", "MuonSpacePoints"};
            SG::WriteHandleKey<MuonR4::SpacePointContainer> m_writeKey{this, "WriteSpacePointKey", "FilteredMlSpacePoints"};
            Gaudi::Property<double>                         m_filterCut{this,"MLFilterCut", -2.8};

    };

}

#endif
