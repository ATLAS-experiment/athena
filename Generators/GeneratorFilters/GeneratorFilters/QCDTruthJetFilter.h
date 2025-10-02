/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_QCDTRUTHJETFILTER_H
#define GENERATORFILTERS_QCDTRUTHJETFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODJet/JetContainer.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <string>

namespace CLHEP {
  class HepRandomEngine;
}

class QCDTruthJetFilter : public GenFilter {
public:

  QCDTruthJetFilter(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode filterInitialize();
  StatusCode filterFinalize();
  StatusCode filterEvent();

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  double m_minPtCut{-1.*Gaudi::Units::GeV};
  Gaudi::Property<double> m_MinPt{this, "MinPt", -1.*Gaudi::Units::GeV};  //!< Min pT for the truth jets
  double m_maxPtCut{7000.*Gaudi::Units::GeV};
  Gaudi::Property<double> m_MaxPt{this, "MaxPt", 7000*Gaudi::Units::GeV};  //!< Max pT for the truth jets
  static constexpr double s_startMinEta{-10};
  Gaudi::Property<double> m_MinEta{this, "MinEta", s_startMinEta}; //!< Min eta for the truth jets
  double m_minEtaCut{s_startMinEta};
  Gaudi::Property<double> m_MaxEta{this, "MaxEta", 999.0}; //!< Max eta for the truth jets
  double m_maxEtaCut{999.0};
  Gaudi::Property<double> m_MinPhi{this, "MinPhi", -999.0};  //!< Min phi for the lead truth jet
  Gaudi::Property<double> m_MaxPhi{this, "MaxPhi", 999.0};
  Gaudi::Property<bool>   m_SymEta{this, "SymEta", false}; //!< Use symmetric cut for min eta? (Default false for p-Pb run filters)
  Gaudi::Property<bool> m_doShape{this, "DoShape", false};  //!< Attempt to flatten the pT distribution

  SG::ReadHandleKey<xAOD::JetContainer> m_TruthJetContainerName{this, "TruthJetContainer", "AntiKt4TruthWZJets"}; // Name of the truth jet container

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  long m_total{0};    //!< Total number of events tested
  long m_passed{0};   //!< Number of events passing all cuts
  long m_ptfailed{0}; //!< Number of events failing the pT cuts

  double m_norm{1.};   //!< Normalization for weights
  double m_high{1.};   //!< High-side function level


public:

  /// @todo Move to an inline in the cc
  static double fitFnR(const double * x , const double *) {
    static const double p[7] = //{100000,-5,0.0004,0.0000001,1./3600.,23,0.8};
      { 1000000, -5.5, 0.0001, 0.00000012, 1./4150., 23, 0.4 };
    return p[0]*std::pow(x[0],p[1]+p[2]*x[0]+p[3]*pow(x[0],2))*pow(1-x[0]*p[4],p[5])*pow(x[0],p[6]);
  }

  /// @todo Move to an inline in the cc
  static double fitFn(const double x ) {
    static const double p[7] = //{100000,-5,0.0004,0.0000001,1./3600.,23,0.8};
      { 1000000, -5.5, 0.0001, 0.00000012, 1./4150., 23, 0.4 };
    return p[0]*std::pow(x,p[1]+p[2]*x+p[3]*pow(x,2))*pow(1-x*p[4],p[5])*pow(x,p[6]);
  }

};

#endif
