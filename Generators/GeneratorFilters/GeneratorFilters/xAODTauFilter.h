/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GENERATORFILTERS_XAODTAUFILTER_H
#define GENERATORFILTERS_XAODTAUFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace CLHEP {
  class HepRandomEngine;
}

/// @author Michael Heldmann, Jan 2003
/// updated by Xin Chen, Nov. 2016
/// updated by Simon Arnling B????th, Nov. 2017

class xAODTauFilter : public GenFilter {
public:

  xAODTauFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterFinalize() override final;
  virtual StatusCode filterEvent() override final;

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthTaus"};
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};// Random number generator

  Gaudi::Property<int> m_Ntau{this, "Ntaus", 1};
  Gaudi::Property<double> m_etaMaxe{this, "EtaMaxe", 2.5};
  Gaudi::Property<double> m_etaMaxmu{this, "EtaMaxmu", 2.5};
  Gaudi::Property<double> m_etaMaxhad{this, "EtaMaxhad", 2.5};

  Gaudi::Property<double> m_pTmine{this, "Ptcute", 12000.0};
  Gaudi::Property<double> m_pTminmu{this, "Ptcutmu", 12000.0};
  Gaudi::Property<double> m_pTminhad{this, "Ptcuthad", 12000.0};

  // new option variables:
  Gaudi::Property<bool> m_NewOpt{this, "UseNewOptions", false};
  Gaudi::Property<int> m_Nleptau{this, "Nleptaus", 0};
  Gaudi::Property<int> m_Nhadtau{this, "Nhadtaus", 0};
  Gaudi::Property<double> m_etaMaxlep{this, "EtaMaxlep", 2.6};
  Gaudi::Property<double> m_pTminlep{this, "Ptcutlep", 7000.0};
  Gaudi::Property<double> m_pTminlep_lead{this, "Ptcutlep_lead", 7000.0};
  Gaudi::Property<double> m_pTminhad_lead{this, "Ptcuthad_lead", 12000.0};
  Gaudi::Property<bool> m_ReverseFilter{this, "ReverseFilter", false};
  Gaudi::Property<bool> m_HasTightRegion{this, "HasTightRegion", false};
  Gaudi::Property<double> m_LooseRejectionFactor{this, "LooseRejectionFactor", 1};
  Gaudi::Property<double> m_pTminlep_tight{this, "Ptcutlep_tight", 7000.0};
  Gaudi::Property<double> m_pTminlep_tight_lead{this, "Ptcutlep_tight_lead", 7000.0};
  Gaudi::Property<double> m_pTminhad_tight{this, "Ptcuthad_tight", 12000.0};
  Gaudi::Property<double> m_pTminhad_tight_lead{this, "Ptcuthad_tight_lead", 12000.0};
  Gaudi::Property<int> m_filterEventNumber{this, "filterEventNumber", 0};

  // Maximum amount of Taus variables:
  Gaudi::Property<bool> m_useMaxNTaus{this, "UseMaxNTaus", false};
  Gaudi::Property<int> m_maxNtau{this, "MaxNtaus", 100};
  Gaudi::Property<int> m_maxNhadtau{this, "MaxNhadtaus", 100};
  Gaudi::Property<int> m_maxNleptau{this, "MaxNleptaus", 100};

  double m_events[6];
  double m_events_sel[6];

  double m_eventse{};
  double m_eventsmu{};
  double m_eventshad{};

  double m_eventseacc{};
  double m_eventsmuacc{};
  double m_eventshadacc{};

};

#endif
