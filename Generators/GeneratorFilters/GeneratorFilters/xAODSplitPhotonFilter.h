/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 
*/
#ifndef GENERATORFILTERS_xAODSplitPhotonFilter_H
#define GENERATORFILTERS_xAODSplitPhotonFilter_H

#include "GeneratorModules/GenFilter.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthEventContainer.h"

class xAODSplitPhotonFilter : public GenFilter {
public:

  xAODSplitPhotonFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterEvent() override;

private:

  
  Gaudi::Property<double> m_Ptmin{this,"Ptcut",15000," "};
  Gaudi::Property<double> m_EtaRange{this,"Etacut",2.50," "};
  Gaudi::Property<int> m_NPhotons{this,"NPhotons",1," "};
  Gaudi::Property<std::vector<int> > m_dauPdg{this,"AcceptedSplit",std::vector<int>()," "};
 

};

#endif
