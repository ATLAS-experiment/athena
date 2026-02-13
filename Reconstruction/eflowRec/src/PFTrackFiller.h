#ifndef PFTRACKFILLER_H
#define PFTRACKFILLER_H

#include "eflowRecTrack.h"
#include "PFData.h"

class PFTrackFiller {

public:
  PFTrackFiller(){};
  ~PFTrackFiller(){};

  static void fillTracksToRecover(PFData &data) ;
  static void fillTracksToConsider(PFData &data, eflowRecTrackContainer &recTrackContainer) ;

};
#endif