/** emacs: this is -*- c++ -*-
 **
 **   @file  ParticleSelector.h
 **
 **   @date    Sun 26 Jul 2026 10:22:51 BST
 **
 **   @class ParticleSelector
 **   @brief Uses the GenericSelectorTool and the OfflineSelectionTool
 **          and working points defined there for track selection (how 
 **          working points if we have to specify all the parameters?) 
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_PARTICLESELECTOR_H
#define IDTPM2_PARTICLESELECTOR_H

#include "TrackSelector.h"
#include "TruthTrackSelector.h"


#include <string>


namespace IDTPM2 {

class ParticleSelector : public TrackSelector, public TruthTrackSelector { 

public:

  ASG_TOOL_CLASS( ParticleSelector, ITrackSelector );

  ParticleSelector( const std::string& name );

  virtual StatusCode initialize() override;

  /// just take the baseclass version ...

  virtual bool selectTrack( const TrackView& t ) const override;
  
};

} // namespace IDTPM2

#endif // IDTPM2_PARTICLESELECTOR_H
