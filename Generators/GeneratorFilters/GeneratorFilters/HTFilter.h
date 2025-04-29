/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERSHTFILTER_H
#define GENERATORFILTERSHTFILTER_H

#include "GeneratorModules/GenFilter.h"
#include <string>
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

class MsgStream;
class StoreGateSvc;

class HTFilter:public GenFilter {

public:

    HTFilter(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~HTFilter();
    virtual StatusCode filterInitialize();
    virtual StatusCode filterFinalize();
    virtual StatusCode filterEvent();

private:

  Gaudi::Property<double> m_MinJetPt{this, "MinJetPt", 0*Gaudi::Units::GeV};
  Gaudi::Property<double> m_MaxJetEta{this, "MaxJetEta",10.0};
  Gaudi::Property<std::string> m_TruthJetContainerName{this,"TruthJetContainer","AntiKt4TruthWZJets","Truht jet container name"};
  Gaudi::Property<double> m_MinHT{this, "MinHT", 20.*Gaudi::Units::GeV};
  Gaudi::Property<double> m_MaxHT{this, "MaxHT", 14000.*Gaudi::Units::GeV};
  Gaudi::Property<bool> m_UseNu{this, "UseNeutrinosFromWZTau",false, "Include neutrinos from W/Z/tau decays in the calculation of HT"};
  Gaudi::Property<bool> m_UseLep{this,"UseLeptonsFromWZTau", false, "Include e/mu from W/Z/tau decays in the HT"};
  Gaudi::Property<double> m_MinLepPt{this, "MinLeptonPt",0*Gaudi::Units::GeV};
  Gaudi::Property<double> m_MaxLepEta{this, "MaxLeptonEta", 10.0};
  Gaudi::Property<bool> m_allowOld{this, "AllowOldFilter", false};

    long m_total;    //!< Total number of events tested
    long m_passed;   //!< Number of events passing all cuts
    long m_ptfailed; //!< Number of events failing the pT cuts 
       
};

#endif
