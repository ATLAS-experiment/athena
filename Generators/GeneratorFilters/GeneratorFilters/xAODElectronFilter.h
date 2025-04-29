/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODELECTRONFILTER_H
#define GENERATORFILTERS_XAODELECTRONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"


/// @brief  Filters and looks for electrons
///
/// This class allows the user to search for electrons or positrons. An event
/// will pass the filter successfully if there is an electron or positron with
/// p_t and eta in the specified range. Default is pt > 10 GeV and unlimited
/// eta.  Please note that the parameters (energy, momenta) are in ATLAS units,
/// i.e. CLHEP::MeV!
///
/// @author I Hinchliffe, December 2001
class xAODElectronFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthElectrons"};
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 10000.};
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 10.0};

};

#endif
