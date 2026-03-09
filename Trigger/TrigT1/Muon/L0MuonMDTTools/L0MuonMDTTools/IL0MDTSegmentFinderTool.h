/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IL0MDTSEGMENTFINDERTOOL_H
#define IL0MDTSEGMENTFINDERTOOL_H


#include "GaudiKernel/IAlgTool.h"
#include "L0MuonMDTTools/L0MDTSegment.h"


namespace L0MDT {

  /**
  * @class IL0MDTSegmentFinderTool
  * @brief Interface for the implementation of L0MDT Segment Finder*/
  
  class IL0MDTSegmentFinderTool: virtual public IAlgTool {

  public:
    DeclareInterfaceID(IL0MDTSegmentFinderTool, 1 ,0);
    virtual ~IL0MDTSegmentFinderTool() = default;
    virtual StatusCode findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles, const EventContext& ctx,
                                    std::vector<L0MDT::Segment>& segments) const = 0;
  };
}
#endif
