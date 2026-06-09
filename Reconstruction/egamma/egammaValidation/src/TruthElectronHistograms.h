/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_TRUTHELECTRONHISTOGRAMS_H
#define EGAMMAVALIDATION_TRUTHELECTRONHISTOGRAMS_H

#include "xAODTruth/TruthParticleFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "ParticleHistograms.h"
#include <map>
#include <string>

class TH2D;

namespace egammaMonitoring {

  class TruthElectronHistograms : public ParticleHistograms
  {

  public:

    using ParticleHistograms::ParticleHistograms;

    virtual StatusCode initializePlots() override;
    StatusCode initializePlots(bool reducedHistSet);

    using ParticleHistograms::fill;
    
    void fill(const xAOD::TruthParticle* truth, const xAOD::Electron* el = nullptr) ;
    virtual ~TruthElectronHistograms(){};

    std::map<std::string, TH2D*> histoMap2D;

  private:    

    bool m_reducedHistSet{};

  };

}

#endif
