/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONMDTTOOLS_ISEGMENTFINDERTOOL_H
#define L1MUONMDTTOOLS_ISEGMENTFINDERTOOL_H


#include "GaudiKernel/IAlgTool.h"
#include "L1MuonMDTTools/Segment.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

namespace L1Muon::L1MDT {

  /**
  * @class ISegmentFinderTool
  * @brief Interface for the implementation of L1Muon::L1MDT Segment Finder*/

  class ISegmentFinderTool: virtual public IAlgTool {

  public:
    DeclareInterfaceID(ISegmentFinderTool, 1 ,0);
    virtual ~ISegmentFinderTool() = default;
    virtual StatusCode findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles, const ActsTrk::GeometryContext& gctx,
                                    float m, float b,std::vector<Segment>& segments) const = 0;
  };
}
#endif
