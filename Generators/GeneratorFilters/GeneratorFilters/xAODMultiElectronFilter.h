/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_xAODMULTIELECTRONFILTER_H
#define GENERATORFILTERS_xAODMULTIELECTRONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

class xAODMultiElectronFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthElectrons"};
  Gaudi::Property<double> m_ptmin{this, "Ptcut", 10000.};
  Gaudi::Property<double> m_etaRange{this, "Etacut", 10.0};
  Gaudi::Property<int> m_nElectrons{this, "NElectrons", 2};

};

#endif
