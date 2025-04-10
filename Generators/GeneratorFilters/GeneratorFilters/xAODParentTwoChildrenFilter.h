/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODPARENTTWOCHILDRENFILTER_H
#define GENERATORFILTERS_XAODPARENTTWOCHILDRENFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Allows the user to search for any given decay  Parent -> Child1 + anti(Child1) AND Parent -> Child2 + anti(Child2)...
/// @author Chaouki Boulahouache, Oct 2008
class xAODParentTwoChildrenFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<double> m_PtMinParent{this,"PtMinParent",0.,"Min parent Pt"};
  Gaudi::Property<std::vector<int>> m_PDGParent{this,"PDGParent",0,"PDG ID of parents"};
  Gaudi::Property<double> m_EtaRangeParent{this,"EtaRangeParent",10.0,"Parent eta range"};
  Gaudi::Property<std::vector<int>> m_PDGChild{this,"PDGChild",0,"PDG ID of child"};
  Gaudi::Property<double> m_PtMinChild{this,"PtMinChild",0.,"Min child Pt"};
  Gaudi::Property<double> m_EtaRangeChild{this,"EtaRangeChild",0.,"Child eta range"};

};

#endif
