/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// Written by Bill Balunas (balunas@cern.ch)
// Based on DiBjetFilter by Stephen Bienek

#ifndef GENERATORFILTERSMULTIBJETFILTER_H
#define GENERATORFILTERSMULTIBJETFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "AthenaKernel/IAthRNGSvc.h"

namespace CLHEP {
  class HepRandomEngine;
}


class MultiBjetFilter:public GenFilter {

  public:
    MultiBjetFilter(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MultiBjetFilter();
    virtual StatusCode filterInitialize();
    virtual StatusCode filterFinalize();
    virtual StatusCode filterEvent();

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

    // Basic jet requirements
    double m_deltaRFromTruth;
    double m_jetPtMin;
    double m_jetEtaMax;
    int m_nJetsMin;
    int m_nJetsMax;

    // Variables for cutting sample into pt slices
    double m_leadJet_ptMin;
    double m_leadJet_ptMax;

    // Flavor filter variables
    double m_bottomPtMin;
    double m_bottomEtaMax;
    int m_nBJetsMin;
    int m_nBJetsMax;

    // inclusive filter efficiency
    double m_inclusiveEff;
    ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};
    std::string m_TruthJetContainerName;

    // Internal bookkeeping variables
    int    m_NPass;
    int    m_Nevt;
    double m_SumOfWeights_Pass;
    double m_SumOfWeights_Evt;
};

#endif


