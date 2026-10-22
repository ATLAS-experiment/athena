/** emacs: this is -*- c++ -*-
 **
 ** @file TrackRoiSelector.h
 **
 **       Actually this class combines *fetching* the tracks in the Roi 
 **       and *selecting* the tracks in the Roi when it should just do the 
 **       selection - the fetching is more properly done in the 
 **       TrackAnalysisCollections class which fetches everything else
 **
 ** 
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_TRACKROISELECTOR_H
#define IDTPM2_TRACKROISELECTOR_H

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include <string>
#include <vector>
#include <optional>

class TrigRoiDescriptorCollection;

namespace IDTPM2 {

/// doesn't actually need to be an algtool as it class variables, 
/// so requires no python configuration
  
class TrackRoiSelector { 

public:

    /// Constructor - self evidently
    TrackRoiSelector() { } 

    /// Destructor
    virtual ~TrackRoiSelector() = default;

    /// geometric RoI filters - for non-trigger tracks (e.g. offline, truth, etc.)
    template<class T>
    bool accept( const T& t, const TrigRoiDescriptor* r ) const;

    /// track getter function (for offline tracks || truth particles)
    template<class T>
    std::vector<const T*> getTracks( const std::vector<const T*>& tracks,
				     const TrigRoiDescriptor* roi ) const;

private:

  struct exit_point {
    exit_point(double z_=0, double r_=0 ) : z(z_), r(r_) { }  
    double z;
    double r;
  };

  
private:

  /// why are there these "exit point functions ? should we not use the
  /// versions in the RoiDescriptor ?

#if 1
  /// this is actually a largely copy of a TrigRoiDescriptor function
    /// Geometric utility methods for track-RoI association
    exit_point  getExitPoint( float tz0, float teta ) const;
#endif
  
  std::optional<double>   getOuterPhi( float pt, float phi, float r=1000. ) const;

}; // class TrackRoiSelector

} // namespace IDTPM2


template<class T>
std::vector<const T*> IDTPM2::TrackRoiSelector::getTracks( const std::vector<const T*>& tracks,
                                                           const TrigRoiDescriptor* roi ) const {

  //  std::cout << "\n\n\tSUTT roi filter" << tvec.size() << "\t" << *roi << "\n" << std::endl;

  std::vector<const T*> selectedTracks;

  for( size_t it=0 ; it<tracks.size() ; it++ ) {

    const T* track = tracks.at(it);

    /// no point looking at tracks that never make it into the detector ...
    if ( track->pt()==0 ) continue;
    
    bool keep = accept<T>( *track, roi );

    //    if ( keep ) std::cout <<  it << "\t" << *tvec[it] << "\taccept: " << keep << std::endl; 
    
    if( keep ) selectedTracks.push_back( track );
  }

  //  std::cout << "\tSUTT: " << selectedTracks.size() << std::endl; 
  
  return selectedTracks;
}




#endif // > !IDTPM2_TRACKROISELECTOR_H
