/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKWRITEFASTSIM_NeutronFastSimTool_h
#define TRACKWRITEFASTSIM_NeutronFastSimTool_h

#include "G4AtlasTools/FastSimulationBase.h"

class G4VFastSimulationModel;

class NeutronFastSimTool: public FastSimulationBase
{
public:

  using FastSimulationBase::FastSimulationBase;
protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;
private:
  Gaudi::Property<std::string> m_trackFastSimSDName{this, "TrackFastSimSDName", "TrackFastSimSDTool"};
  Gaudi::Property<double> m_etaCut{this, "PrimaryEtaCut", 6.0, "Eta cut for primaries"};
  Gaudi::Property<double> m_timeCut{this, "NeutronTimeCut", 150., "Time cut for neutrons"};
};

#endif // TRACKWRITEFASTSIM_NeutronFastSimTool_h
