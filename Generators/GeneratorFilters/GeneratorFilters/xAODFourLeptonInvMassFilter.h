/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** 
    @class GeneratorFilters/FourLeptonInvMassFilter.h
    
    Filter on 4lepton-mass in the mass windows (by default >60GeV)
    
    - Apply Pt and Eta cuts on leptons.  Default is Pt > 5 GeV and |eta| < 5
    
    based on FourLeptonMassFilter by 
    @author Theodota Lagouri Theodota Lagouri <theodota.lagouri@cern.ch>
    @author Konstantinos Nikolopoulos <konstantinos.nikolopoulos@cern.ch>
    
    @author Antonio Salvucci <antonio.salvucci@cern.ch>
*/

#ifndef GENERATORFILTER_xAODFOUREPTONINVMASSFILTER  
#define GENERATORFILTER_xAODFOUREPTONINVMASSFILTER        

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

class xAODFourLeptonInvMassFilter:public GenFilter {
public:
  xAODFourLeptonInvMassFilter(const std::string& name, ISvcLocator* pSvcLocator);
 
  virtual StatusCode filterInitialize() override;
  virtual StatusCode filterEvent(const EventContext& ctx) override;
  
private:
 
  Gaudi::Property<double> m_minPt{this, "MinPt", 5000.};
  Gaudi::Property<double> m_maxEta{this, "MaxEta", 5.0};
  Gaudi::Property<double> m_minMass{this, "MinMass", 60000};
  Gaudi::Property<double> m_maxMass{this, "MaxMass", 14000000};


  bool passesLeptonSelection(const xAOD::TruthParticle* p) const;



  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameLightLeptonKey
  {this, "xAODTruthParticleContainerNameLightLepton","TruthLightLeptons","Name of Truth Light Leptons container from the slimmer"};
    
};

#endif
