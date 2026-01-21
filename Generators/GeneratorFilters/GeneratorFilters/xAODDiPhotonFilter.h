/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_xAODDIPHOTONFILTER_H
#define GENERATORFILTERS_xAODDIPHOTONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "GaudiKernel/SystemOfUnits.h"

/// Filters and looks for di-photons
/// @author J Tanaka, Aug 2009
class xAODDiPhotonFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;


private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  
  Gaudi::Property<double> m_Ptmin_1st{this, "PtCut1st", 20000.* Gaudi::Units::MeV};
  Gaudi::Property<double> m_Ptmin_2nd{this, "PtCut2nd", 15000.* Gaudi::Units::MeV};
  Gaudi::Property<double> m_Ptmin_others{this, "PtCutOthers", 15000.* Gaudi::Units::MeV};
  Gaudi::Property<double> m_EtaRange_1st{this, "EtaCut1st", 2.50};
  Gaudi::Property<double> m_EtaRange_2nd{this, "EtaCut2nd", 2.50};
  Gaudi::Property<double> m_EtaRange_others{this, "EtaCutOthers", 2.50};
  Gaudi::Property<double> m_diphoton_deltaRmin{this, "DeltaRCutFrom", -1.};
  Gaudi::Property<double> m_diphoton_deltaRmax{this, "DeltaRCutTo", -1.};
  Gaudi::Property<double> m_diphoton_massmin{this, "MassCutFrom", -1.* Gaudi::Units::MeV};
  Gaudi::Property<double> m_diphoton_massmax{this, "MassCutTo", -1.* Gaudi::Units::MeV};
  Gaudi::Property<double> m_diphoton_PtMin{this, "DiPhotonPtMin", -1* Gaudi::Units::MeV};
  Gaudi::Property<double> m_diphoton_PtMax{this, "DiPhotonPtMax", -1* Gaudi::Units::MeV};
  Gaudi::Property<double> m_use1st2ndPhotonsforMassAndDeltaRCuts{this, "Use1st2ndPhotons", false};
  
};

#endif
