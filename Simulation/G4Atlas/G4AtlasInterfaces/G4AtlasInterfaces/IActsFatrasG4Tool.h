/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IActsFatrasG4Tool_H
#define IActsFatrasG4Tool_H

// C++
#include <vector>
#include <string>

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// forward declarations
class G4FastTrack;
class G4FastStep;
class SiHit;
class EventContext;

/**
  @class IActsFatrasG4Tool

   Interface for ACTS Fatras G4 tool.
   
  @author marilena.bandieramonte@cern.ch, rui.wang@cern.ch, firdaus.soberi@cern.ch
*/

class IActsFatrasG4Tool : virtual public IAlgTool
{
 public:
     /** AlgTool interface method, handles constructor/destructor */
    DeclareInterfaceID(IActsFatrasG4Tool, 1, 0);

    /** create ActsFatras track */
    virtual void simulateFatrasTrack(const G4FastTrack& fastTrack, G4FastStep& fastStep) = 0;

    /** for saving si hits cache, expose to interface */
    virtual const std::vector<SiHit>& getPixelHitsCache(const EventContext& ctx) const = 0;
    virtual const std::vector<SiHit>& getSCTHitsCache(const EventContext& ctx) const = 0;
    virtual void clearCaches(const EventContext& ctx) const = 0;
    
};

#endif // IActsFatrasG4Tool_H
