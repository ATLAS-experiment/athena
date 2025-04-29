/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODZWINDOWFILTER_H
#define GENERATORFILTERS_XAODZWINDOWFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Filter on dileptons within a Z mass window
///
/// - Apply Pt and Eta cuts on leptons.  Default is Pt > 5 GeV and |eta| < 5
/// - Optionally allow same sign paris.  Default is true
/// - Optionally allow Z -> e mu.  Default is false
///
/// @author Carl Gwilliam <gwilliam@mail.cern.ch>
class xAODDiLeptonMassFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize();
  virtual StatusCode filterFinalize();
  virtual StatusCode filterEvent();

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthLightLeptons"};
  Gaudi::Property<double> m_minPt{this, "MinPt", 5000.};
  Gaudi::Property<double> m_maxEta{this, "MaxEta", 5.0};
  Gaudi::Property<double> m_minMass{this, "MinMass", 1000};      // To avoid fsr etc
  Gaudi::Property<double> m_maxMass{this, "MaxMass", 14000000};
  Gaudi::Property<double> m_minDilepPt{this, "MinDilepPt", -1.};
  Gaudi::Property<bool>   m_allowElecMu{this, "AllowElecMu", false};
  Gaudi::Property<bool>   m_allowSameCharge{this, "AllowSameCharge", true};
  int m_AthenaCalls{};

};

#endif //GENERATORFILTERS_XAODZWINDOWFILTER_H
