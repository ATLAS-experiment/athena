/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration 
*/
// --------------------------------------------------
// 
// File:  GeneratorFilters/MuDstarFilter.h
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


#ifndef GENERATORFILTERSMUDSTARFILTER_H
#define GENERATORFILTERSMUDSTARFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "TruthUtils/ParticleConstants.h"

#include "CLHEP/Vector/LorentzVector.h"
#include "TLorentzVector.h"


class MuDstarFilter:public GenFilter {
public:
        MuDstarFilter(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~MuDstarFilter();
        virtual StatusCode filterInitialize();
        virtual StatusCode filterFinalize();
        virtual StatusCode filterEvent();

private:
	// Setable Properties:-

	// Local Member Data:-
	double m_PtMinMuon;
	double m_PtMaxMuon;
	double m_EtaRangeMuon;
        //
	double m_PtMinDstar;
	double m_PtMaxDstar;
	double m_EtaRangeDstar;
	double m_RxyMinDstar;
        //
	double m_PtMinPis;
	double m_PtMaxPis;
	double m_EtaRangePis;
        //
	double m_PtMinKpi;
	double m_PtMaxKpi;
	double m_EtaRangeKpi;
        //
	bool m_D0Kpi_only;
        //
	double m_mKpiMin;
	double m_mKpiMax;
        //
	double m_delta_m_Max;
        //
	double m_DstarMu_m_Max;
        //

       // PDG 2022:
       const double m_MuonMass = ParticleConstants::muonMassInMeV;
       const double m_PionMass = ParticleConstants::chargedPionMassInMeV;
       const double m_KaonMass = 493.677;

  // Private Methods:=

};

#endif


