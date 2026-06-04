/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITRKAMBIGUITYSCOREPROCESSORTOOL_H
#define ITRKAMBIGUITYSCOREPROCESSORTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkTrack/TrackCollection.h" // typedef
#include "TrkToolInterfaces/ITrackAmbiguityProcessorTool.h"


namespace Trk {

/** @brief Interface for resolving hit association ambiguities in a given track collection.

The TrkAmbiguityScoreProcessor attempts to improve the 'score' of an event, where the score of an event is the summed scores of all the tracks it contains. 

*/
class ITrackAmbiguityScoreProcessorTool : virtual public IAlgTool
{
	public:

	DeclareInterfaceID(ITrackAmbiguityScoreProcessorTool, 1, 0);
	/** (in concrete object) Returns a processed TrackCollection from the passed 'tracks'
	@param tracks collection of tracks which will have ambiguities resolved. Will not be modified.
	@return  map of score and track. Ownership is passed on 
	(i.e. client handles deletion)*/
  virtual void process(const EventContext& ctx, const TrackCollection & tracks, TracksScores* scoredTracks) const = 0;
  //Print statistics at the end of the processing.
  virtual void statistics() = 0;

};

} //end ns

#endif // TrackAmbiguityScoreProcessorTool_H
