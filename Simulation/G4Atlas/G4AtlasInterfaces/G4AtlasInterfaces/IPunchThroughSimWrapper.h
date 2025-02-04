/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4AtlasInterfaces_IPunchThroughSimWrapper_H
#define G4AtlasInterfaces_IPunchThroughSimWrapper_H

// C++
#include <vector>

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// Random generator includes
#include "AthenaKernel/RNGWrapper.h"

// forward declarations
class G4FastTrack;
class G4FastStep;
class G4ParticleTable;

/**
  @class IPunchThroughSimWrapper

   Interface for wrapper to run punch through simulation.
   Linked to both IPunchThroughG4Classifier.h and IPunchThroughG4Tool.h

  @author firdaus.soberi@cern.ch, thomas.michael.carter@cern.ch
*/

class IPunchThroughSimWrapper : virtual public IAlgTool
{
 public:
    /** AlgTool interface method, handles constructor/destructor */
    DeclareInterfaceID(IPunchThroughSimWrapper, 1, 0);

    /** Runs both PunchThroughG4Classifier and PunchThroughG4Tool for PunchThrough simulation */
    virtual void DoPunchThroughSim(G4ParticleTable &ptable, ATHRNG::RNGWrapper* rngWrapper, const double simE, std::vector<double> simEfrac, const G4FastTrack& fastTrack, G4FastStep& fastStep) = 0;
};

#endif // IPunchThroughSimWrapper_H
