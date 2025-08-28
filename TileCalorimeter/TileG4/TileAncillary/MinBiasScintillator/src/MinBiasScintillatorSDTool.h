/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class MinBiasScintillatorSDTool
// Tool for configuring the Sensitive detector for the Minimum Bias Scintillator
//
//************************************************************

#ifndef MINBIASSCINTILLATOR_MINBIASSCINTILLATORSDTOOL_H
#define MINBIASSCINTILLATOR_MINBIASSCINTILLATORSDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

#include "MinBiasScintSDOptions.h"

class MinBiasScintillatorSDTool: public SensitiveDetectorBase {
public:
  MinBiasScintillatorSDTool(const std::string& type, const std::string& name, const IInterface *parent);
  ~MinBiasScintillatorSDTool() = default;
  virtual StatusCode initialize() override;

  /** End of an athena event */
  virtual StatusCode Gather() override final; //FIXME would be good to be able to avoid this.

protected:
    // Make me an SD!
  virtual G4VSensitiveDetector* makeSD() const override final;
  
private:
  // Options for the SD configuration
  MinBiasScintSDOptions m_options;
  
  Gaudi::Property<std::vector<double>> m_deltaTHit{this, "DeltaTHit", {0.5 , -75.25 , 75.25 , 5.}};
  Gaudi::Property<double> m_timeCut{this, "TimeCut", 350.5};
  Gaudi::Property<bool> m_tileTB{this, "TileTB", false};
  Gaudi::Property<bool> m_doBirk{this, "DoBirk", true};
  Gaudi::Property<double> m_birk1{this, "Birk1", 0.0130 * CLHEP::g / (CLHEP::MeV * CLHEP::cm2)};
  Gaudi::Property<double> m_birk2{this, "Birk2", 9.6e-6 * CLHEP::g / (CLHEP::MeV * CLHEP::cm2) * CLHEP::g / (CLHEP::MeV * CLHEP::cm2)};
  Gaudi::Property<bool> m_doTOFCorrection{this, "DoTOFCorrection", true};

};

#endif //MINBIASSCINTILLATOR_MINBIASSCINTILLATORSDTOOL_H
