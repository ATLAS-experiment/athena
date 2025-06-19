/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4FASTSIMULATION_LARFASTSHOWERTOOL_H
#define LARG4FASTSIMULATION_LARFASTSHOWERTOOL_H

// Base class header
#include "G4AtlasTools/FastSimulationBase.h"

// Member variable headers
#include "GaudiKernel/ServiceHandle.h"
#include "LArG4ShowerLibSvc/ILArG4ShowerLibSvc.h"
#include "FastShowerConfigStruct.h"

// STL headers
#include <string>

/// NEEDS DOCUMENTATION
class LArFastShowerTool: public FastSimulationBase
{
public:

  LArFastShowerTool(const std::string& type, const std::string& name, const IInterface *parent);  //!< Default constructor

  virtual ~LArFastShowerTool() = default;

  StatusCode initialize() override final;

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;

private:
  Gaudi::Property<std::string> m_fastSimDedicatedSD{this, "SensitiveDetector", ""
      , "Fast sim dedicated SD for this setup"}; //!< Shower library sensitive detector for this shower
  ServiceHandle<ILArG4ShowerLibSvc> m_showerLibSvc{this, "ShowerLibSvc", "LArG4ShowerLibSvc"
    , "Handle on the shower library service"};       //!< Pointer to the shower library service

  Gaudi::Property<bool> m_e_FlagShowerLib {this, "EFlagToShowerLib", true, "Switch for e+/- frozen showers"};
  Gaudi::Property<double> m_e_MinEneShowerLib {this, "EMinEneShowerLib", 0.0*CLHEP::GeV, "Minimum energy for e+/- frozen showers"};
  Gaudi::Property<double> m_e_MaxEneShowerLib {this, "EMaxEneShowerLib", 1.0*CLHEP::GeV, "Maximum energy for e+/- frozen showers"};

  Gaudi::Property<bool> m_g_FlagShowerLib {this, "GFlagToShowerLib", true, "Switch for photon frozen showers"};
  Gaudi::Property<double> m_g_MinEneShowerLib {this, "GMinEneShowerLib", 0.*CLHEP::GeV, "Minimum energy for photon frozen showers"};
  Gaudi::Property<double> m_g_MaxEneShowerLib {this, "GMaxEneShowerLib", 0.010*CLHEP::GeV, "Maximum energy for photon frozen showers"};

  Gaudi::Property<bool> m_Neut_FlagShowerLib {this, "NeutFlagToShowerLib", true, "Switch for neutron frozen showers"};
  Gaudi::Property<double> m_Neut_MinEneShowerLib {this, "NeutMinEneShowerLib", 0.0*CLHEP::GeV, "Minimum energy for neutron frozen showers"};
  Gaudi::Property<double> m_Neut_MaxEneShowerLib {this, "NeutMaxEneShowerLib", 0.1*CLHEP::GeV, "Maximum energy for neutron frozen showers"};

  Gaudi::Property<bool> m_Pion_FlagShowerLib {this, "PionFlagToShowerLib", true, "Switch for neutron frozen showers"};
  Gaudi::Property<double> m_Pion_MinEneShowerLib {this, "PionMinEneShowerLib", 0.0*CLHEP::GeV, "Minimum energy for neutron frozen showers"};
  Gaudi::Property<double> m_Pion_MaxEneShowerLib {this, "PionMaxEneShowerLib", 2.0*CLHEP::GeV, "Maximum energy for neutron frozen showers"};

  Gaudi::Property<bool> m_containLow {this, "ContainLow", true, "Switch for containment at low eta"};
  Gaudi::Property<double> m_absLowEta {this, "AbsLowEta", 3.8, ""};
  Gaudi::Property<bool> m_containHigh {this, "ContainHigh", true, "Switch for containment at high eta"};
  Gaudi::Property<double> m_absHighEta {this, "AbsHighEta", 4.4, ""};
  Gaudi::Property<bool> m_containCrack {this, "ContainCrack", true, "Switch for containment in the crack region"};
  Gaudi::Property<double> m_absCrackEta1 {this, "AbsCrackEta1", 0.5, ""};
  Gaudi::Property<double> m_absCrackEta2 {this, "AbsCrackEta2", 1.1, ""};

  Gaudi::Property<std::string> m_generated_starting_points_file {this, "GeneratedStartingPointsFile", "",
    "Name of file for generated SPs. Do not touch until you want to produce a new library"};
  Gaudi::Property<float> m_generated_starting_points_ratio {this, "GeneratedStartingPointsRatio", 0.02f
    , "Ratio of SPs that goes to output"};
  Gaudi::Property<int> m_detector_tag {this, "DetectorTag", 0, "Which detector is this?"};

  Gaudi::Property<bool> m_applyRRWeights {this, "ApplyRRWeights", false
    , "Should the weights set by NRR/PRR be applied to Frozen Shower Energy deposits?"};

  FastShowerConfigStruct m_configuration;
};

#endif //LARG4FASTSIMULATION_LARFASTSHOWERTOOL_H
