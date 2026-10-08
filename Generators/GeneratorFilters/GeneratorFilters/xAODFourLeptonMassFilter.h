
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef GENERATORFILTER_xAODFOUREPTONMASSFILTER
#define GENERATORFILTER_xAODFOUREPTONMASSFILTER

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

/// Filter on two dilepton-pairs with two mass windows (by default >60GeV and
/// >12GeV)
///
/// - Apply Pt and Eta cuts on leptons.  Default is Pt > 5 GeV and |eta| < 3
/// - Optionally allow same sign pairs.  Default is true
/// - Optionally allow emu pairs.  Default is true
///
/// @author Theodota Lagouri Theodota Lagouri <theodota.lagouri@cern.ch>
/// @author Konstantinos Nikolopoulos <konstantinos.nikolopoulos@cern.ch>
class xAODFourLeptonMassFilter : public GenFilter {
 public:
  xAODFourLeptonMassFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize() override;
  virtual StatusCode filterEvent(const EventContext& ctx) override;

 private:
  Gaudi::Property<double> m_minPt{this, "MinPt", 5000., " "};
  Gaudi::Property<double> m_maxEta{this, "MaxEta", 3.0, " "};
  Gaudi::Property<double> m_minMass1{this, "MinMass1", 60000, " "};
  Gaudi::Property<double> m_maxMass1{this, "MaxMass1", 14000000, " "};
  Gaudi::Property<double> m_minMass2{this, "MinMass2", 12000, " "};
  Gaudi::Property<double> m_maxMass2{this, "MaxMass2", 14000000, " "};
  Gaudi::Property<bool> m_allowElecMu{this, "AllowElecMu", true, " "};
  Gaudi::Property<bool> m_allowSameCharge{this, "AllowSameCharge", true, " "};


  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameLightLeptonKey
  {this, "xAODTruthParticleContainerNameLightLepton","TruthLightLeptons","Name of Truth Light Leptons container from the slimmer"};
    
};

#endif
