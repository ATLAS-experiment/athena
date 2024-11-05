/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_IMUONSEGMENTCONVERTERTOOL_H
#define XAOD_IMUONSEGMENTCONVERTERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonSegment/MuonSegment.h"
#include "AthLinks/ElementLink.h"
#include "TrkSegment/SegmentCollection.h"



namespace xAODMaker {
  /** Interface for tools calculating hit count summaries for track */
  class IMuonSegmentConverterTool : virtual public IAlgTool {      
  public:
    virtual ~IMuonSegmentConverterTool() = default;
    /** IAlgTool interface */
    DeclareInterfaceID(xAODMaker::IMuonSegmentConverterTool, 1, 0);

    /** @brief convert a ElementLink to a Trk::Segment (should be of type MuonSegment) to a xAOD::MuonSegment, the segment is add to the container if provided 
        so the caller should not delete it */
    virtual xAOD::MuonSegment* convert( const ElementLink< ::Trk::SegmentCollection >& segLink, 
					                              xAOD::MuonSegmentContainer* container = nullptr ) const = 0;

    /** @brief convert a Muon::MuonSegment to a xAOD::MuonSegment, the segment is add to the container if provided 
        so the caller should not delete it */
    virtual xAOD::MuonSegment* convert( const Muon::MuonSegment& seg, 
					                              xAOD::MuonSegmentContainer* container = nullptr ) const = 0;
  };
  

} // end of name space

#endif // IMuonSegmentConverterTool_H

