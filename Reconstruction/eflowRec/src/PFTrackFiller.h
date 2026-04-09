/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFTRACKFILLER_H
#define EFLOWREC_PFTRACKFILLER_H

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
