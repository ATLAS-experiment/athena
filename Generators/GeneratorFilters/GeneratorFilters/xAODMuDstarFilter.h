/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 
*/
// --------------------------------------------------
// 
// File:  GeneratorFilters/xAODMuDstarFilter.h
// Description:
//
//   Allows the user to search for (mu D*) combinations
//   with both same and opposite charges.
//   For D*+-, the decay D*+ -> D0 pi_s+ is selected
//   with D0 in the nominal decay mode, D0 -> K- pi+ (Br = 3.947%),
//   and, if "D0Kpi_only=false", 14 main other decays to 2 or 3 particles (except nu_e and nu_mu)
//   in case they can immitate the nominal decay:
//   D0 -> K-mu+nu, K-e+nu, pi-mu+nu, pi-e+nu,
//         K-mu+nu pi0, K-e+nu pi0, pi-mu+nu pi0, pi-e+nu pi0,
//         pi-pi+, K-K+, K-pi+pi0, K-K+pi0, pi-pi+pi0, K-pi+gamma
//         Doubly Cabbibo supressed modes are also considered
//   Requirements for non-nominal decay modes:
//         D*+ -> ("K-" "pi+") pi_s+ (+c.c.) charges
//         mKpiMin < m("K" "pi") < mKpiMax
//         m("K" "pi" pi_s) - m("K" "pi") < delta_m_Max
//
// AuthorList:         
//   L K Gladilin (gladilin@mail.cern.ch)  March 2023


#ifndef GENERATORFILTERSXAODMUDSTARFILTER_H
#define GENERATORFILTERSXAODMUDSTARFILTER_H

#include "GeneratorModules/GenFilter.h"

#include "CLHEP/Vector/LorentzVector.h"
#include "TLorentzVector.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/ParticleConstants.h"

class xAODMuDstarFilter:public GenFilter {
public:
        xAODMuDstarFilter(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~xAODMuDstarFilter();
        virtual StatusCode filterInitialize();
        virtual StatusCode filterEvent();

private:
	// Setable Properties:-
        Gaudi::Property<double>  m_PtMinMuon{this,"PtMinMuon", 2500.};
        Gaudi::Property<double>  m_PtMaxMuon{this,"PtMaxMuon", 1e9};
        Gaudi::Property<double>  m_EtaRangeMuon{this,"EtaRangeMuon", 2.7};
        //
        Gaudi::Property<double>  m_PtMinDstar{this,"PtMinDstar", 4500.};
        Gaudi::Property<double>  m_PtMaxDstar{this,"PtMaxDstar", 1e9};
        Gaudi::Property<double>  m_EtaRangeDstar{this, "EtaRangeDstar", 2.7};
        Gaudi::Property<double>  m_RxyMinDstar{this, "RxyMinDstar", -1e9};
        //
        Gaudi::Property<double>  m_PtMinPis{this, "PtMinPis", 450.};
        Gaudi::Property<double>  m_PtMaxPis{this, "PtMaxPis", 1e9};
        Gaudi::Property<double>  m_EtaRangePis{this, "EtaRangePis", 2.7};
        //
        Gaudi::Property<double>  m_PtMinKpi{this, "PtMinKpi", 900.};
        Gaudi::Property<double>  m_PtMaxKpi{this, "PtMaxKpi", 1e9};
        Gaudi::Property<double>  m_EtaRangeKpi{this, "EtaRangeKpi", 2.7};
        //
        Gaudi::Property<bool>    m_D0Kpi_only{this, "D0Kpi_only", false};
        //
        Gaudi::Property<double>  m_mKpiMin{this, "mKpiMin", 1665.};
        Gaudi::Property<double>  m_mKpiMax{this, "mKpiMax", 2065.};
        //
        Gaudi::Property<double>  m_delta_m_Max{this, "delta_m_Max", 220.};
        //
        Gaudi::Property<double>  m_DstarMu_m_Max{this, "DstarMu_m_Max", 12000.};

       const double m_MuonMass = ParticleConstants::muonMassInMeV;
       const double m_PionMass = ParticleConstants::chargedPionMassInMeV;
       const double m_KaonMass = ParticleConstants::chargedKaonMassInMeV;

       // ReadHandle for the Gen TruthParticles 
       SG::ReadHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameGenKey{this, "TruthParticlesKey", "TruthGen", "ReadHandleKey for the xAODTruthParticlesGen"};

  // Private Methods:=

};

#endif


