/** emacs: this is -*- c++ -*- 
 **
 ** @file    ITrackSelector.h
 ** @brief   header file for interface for all the various
 **          track selection tools in this package
 **
 ** Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef IDTPM2_ITRACKSELECTOR_H
#define IDTPM2_ITRACKSELECTOR_H


/// Athena includes
#include "AsgTools/IAsgTool.h"

#include "TrackView.h"
#include "TagNProbe.h"


namespace IDTPM2 {

class ITrackSelector : virtual public asg::IAsgTool {

public:

  ASG_TOOL_INTERFACE( IDTPM2::ITrackSelector )

  virtual std::vector<const TrackView*> selectTracks( const std::vector<const TrackView*>& tv, TagNProbe* tnp=0 ) const { 
      std::vector<const TrackView*> out;
      if ( !highestPTonly() ) {
	for ( size_t i=0 ; i<tv.size() ; i++ ) {
	  if ( tnp && !tnp->select(tv[i]) ) continue;
	  if ( selectTrack(*tv[i]) ) out.push_back(tv[i]);
	}
      }
      else {
	/// if we want the highest PT track only, we could a) sort and then return the first,
	/// or b) just go through looking for the highest PT. Method a) is O(nlogn) at best,
	/// whereas b) is O(n) so ...
	const TrackView* best = 0;
	for ( size_t i=0 ; i<tv.size() ; i++ ) {
	  if ( tnp && !tnp->select(tv[i]) ) continue;
	  if ( !selectTrack(*tv[i]) ) continue;
	  if ( !best || std::fabs(tv[i]->pt())>std::fabs(best->pt()) ) best = tv[i];
	}
	if ( best ) out.push_back(best);
      }
      return out;
  }

  virtual bool selectTrack( const TrackView& t ) const = 0;

  /// select the highest PT track only 
  virtual bool highestPTonly() const { return false; }

};

} // namespace IDTPM2

#endif // > ! IDTPM2_ITRACKSELECTOR_H
