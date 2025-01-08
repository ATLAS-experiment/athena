/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include <vector>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "MSVertexUtils/Tracklet.h"
//

namespace Muon {

    /** @brief The IMSVertexTrackletTool is a pure virtual interface */
    class IMSVertexTrackletTool : virtual public IAlgTool {
    public:
        /** access to tool interface */
        DeclareInterfaceID(Muon::IMSVertexTrackletTool, 1, 0);

        virtual StatusCode findTracklets(std::vector<Tracklet>& traklets, const EventContext& ctx) const = 0;
    };

}  // namespace Muon
