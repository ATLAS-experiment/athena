/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKWRITEFASTSIM_StoppedParticleFastSimTool_h
#define TRACKWRITEFASTSIM_StoppedParticleFastSimTool_h

#include "G4AtlasTools/FastSimulationBase.h"

#include "GaudiKernel/ToolHandle.h"
#include "G4AtlasInterfaces/ISensitiveDetector.h" // What we have a handle on

class G4VFastSimulationModel;

class StoppedParticleFastSimTool: public FastSimulationBase
{
public:

  using FastSimulationBase::FastSimulationBase;
  StatusCode initialize() override final;
protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;
private:
  PublicToolHandle<ISensitiveDetector> m_trackFastSimSDTool{this, "TrackFastSimSDTool", "TrackFastSimSDTool"};
};

#endif // TRACKWRITEFASTSIM_StoppedParticleFastSimTool_h
