/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IL0MDTSEGMENTFINDERTOOL_H
#define IL0MDTSEGMENTFINDERTOOL_H


#include "GaudiKernel/IAlgTool.h"
#include "L1MuonMDTTools/L0MDTSegment.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

namespace L1Muon::L1MDT {

  /**
  * @class IL0MDTSegmentFinderTool
  * @brief Interface for the implementation of L1Muon::L1MDT Segment Finder*/
  
  class IL0MDTSegmentFinderTool: virtual public IAlgTool {

  public:
    DeclareInterfaceID(IL0MDTSegmentFinderTool, 1 ,0);
    virtual ~IL0MDTSegmentFinderTool() = default;
    virtual StatusCode findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles, const ActsTrk::GeometryContext& gctx,
                                    float m, float b,std::vector<L1Muon::L1MDT::Segment>& segments) const = 0;
  };
}
#endif
