/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_DEADMATERIALSHOWERTOOL_H
#define G4FASTSIMULATION_DEADMATERIALSHOWERTOOL_H

#include "G4AtlasTools/FastSimulationBase.h"

#include "CLHEP/Units/SystemOfUnits.h"

class G4VFastSimulationModel;
class DeadMaterialShowerTool: public FastSimulationBase
{
 public:

  using FastSimulationBase::FastSimulationBase;

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;

 private:
  Gaudi::Property<double> m_highEnergy{this, "HighEnergy", 1000.0*CLHEP::GeV, "Kill everything but muons up to this energy"};
  Gaudi::Property<double> m_lowEnergy{this, "LowEnergy", 0.0*CLHEP::GeV, "Kill only electrons and positrons up to this energy"};
  Gaudi::Property<double> m_zcutoff{this, "ZCutoff", 5698.*CLHEP::mm, "Z-position to start killing everything but muons"};

};

#endif //G4FASTSIMULATION_DEADMATERIALSHOWERTOOL_H

