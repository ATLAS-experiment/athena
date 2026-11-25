/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// InDetSplittedTracksCreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDET_INDETSPLITTEDTRACKSCREATOR_H
#define INDET_INDETSPLITTEDTRACKSCREATOR_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkTrack/TrackCollection.h"
#include "InDetTrackSplitterTool/IInDetTrackSplitterTool.h"

namespace InDet {

/** @class InDetSplittedTracksCreator
 
   Validation Algorithm to retrieve, a track collection from cosmics events, split it in upper half and lower half tracks
   and record it to storegate

   @author Manuel.Kayl@cern.ch
*/

class InDetSplittedTracksCreator : public AthReentrantAlgorithm {
public:
    /** Standard Athena-Algorithm Constructor */
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    /** Default Destructor */
    ~InDetSplittedTracksCreator();

    /** standard Athena-Algorithm method */
    StatusCode          initialize();
    /** standard Athena-Algorithm method */
    StatusCode execute(const EventContext& ctx) const;
    /** standard Athena-Algorithm method */
    StatusCode          finalize();

private:
    /**  holds the name of the track coll to be used*/
    SG::ReadHandleKey<TrackCollection>     m_TrackCol{this, "TrackCollection", "Tracks"};
    /**  holds the names the output track collection written to store gate*/
    SG::WriteHandleKey<TrackCollection>    m_OutputTrackCol{this, "OutputTrackCollection", ""};
    /** flag for using TRT standalone tracks **/
    //bool                                   m_makeTRTStandaloneTracks;
    /** flag for using si only tracks **/
    BooleanProperty                        m_makeSiOnlyTracks{this, "makeSiOnlyTracks", false};
    /** flag to accept the upper segment **/
    BooleanProperty                        m_takeUpperSegment{this, "takeUpperSegment", true};
    /** flag to accept the lower segment **/
    BooleanProperty                        m_takeLowerSegment{this, "takeLowerSegment", true};
    /** instances of IInDet */
    PublicToolHandle< InDet::IInDetTrackSplitterTool >  m_trackSplitterTool{this, "TrackSplitterTool", ""};

};

} // close of namespace

#endif // INDET_INDETSPLITTEDTRACKSCREATOR_H

