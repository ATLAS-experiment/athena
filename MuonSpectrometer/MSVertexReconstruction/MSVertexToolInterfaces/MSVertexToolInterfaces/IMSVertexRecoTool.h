/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include <vector>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "MSVertexUtils/MSVertex.h"
#include "MSVertexUtils/Tracklet.h"
//

namespace Muon {

    /** @brief The IMSVertexRecoTool is a pure virtual interface */
    class IMSVertexRecoTool : virtual public IAlgTool {
    public:
        /** access to tool interface */
        DeclareInterfaceID(Muon::IMSVertexRecoTool, 1, 0);

        virtual StatusCode findMSvertices(const std::vector<Tracklet>& tracklets, std::vector<std::unique_ptr<MSVertex>> &vertices,
                                          const EventContext& ctx) const = 0;
    };

}  // namespace Muon
