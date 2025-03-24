/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERFACES_IGRAPHINFERENCETOOL_H
#define MUONINFERENCEINTERFACES_IGRAPHINFERENCETOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
namespace MuonML {
    struct GraphRawData;
    class IGraphInferenceTool: virtual public IAlgTool {
        public:
            /** @brief Empty desctructor */
            virtual ~IGraphInferenceTool() = default;
            /** @brief Declaration of the interface */
            DeclareInterfaceID(IGraphInferenceTool, 1, 0);

            virtual StatusCode runGraphInference(const EventContext& ctx,
                                                 GraphRawData& graph) const = 0;
    };
}
#endif