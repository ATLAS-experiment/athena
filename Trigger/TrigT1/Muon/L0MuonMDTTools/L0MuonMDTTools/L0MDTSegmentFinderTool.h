/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L0MuonMDTTools_L0MDTSEGMENTFINDERTOOL_H
#define L0MuonMDTTools_L0MDTSEGMENTFINDERTOOL_H
#include "AthenaBaseComps/AthAlgTool.h"
//local includes
#include "xAODMuonPrepData/MdtDriftCircleContainer.h" 
#include "L0MuonMDTTools/IL0MDTSegmentFinderTool.h"
#include "L0MuonMDTTools/L0MDTSegment.h"

// namespace for the L0MDTS related classes
namespace L0MDT {


/**
 * @class L0MDTSegmentFinderTool
 * @brief Athena tool to reconstruct L0MDT segments.*/
 
     class L0MDTSegmentFinderTool : public extends<AthAlgTool, IL0MDTSegmentFinderTool> {

  public:
    using base_class::base_class;
    virtual ~L0MDTSegmentFinderTool() override = default;

    virtual StatusCode initialize() override;
    
    
    virtual StatusCode findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles,
                                const EventContext& ctx,
                                std::vector<L0MDT::Segment>& segments) const override;

    private:
    };  
} // end of namespace
#endif

