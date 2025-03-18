/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUON_SMEARINGALG_H
#define L0MUON_SMEARINGALG_H 

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"

namespace L0Muon {

class TruthTrackSmearer;

class L0MuonSmearingAlg: public ::AthReentrantAlgorithm { 
 public: 
  L0MuonSmearingAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~L0MuonSmearingAlg();

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  virtual StatusCode  finalize() override;

 private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inputTruthParticleKey{this, "InputTruthParticle", "TruthParticles",
                                                                          "key for retrieval of input Truth particle"};

  SG::WriteHandleKey<xAOD::MuonRoIContainer> m_outputMuonRoIKey{this, "OutputMuonRoI", "LVL1MuonRoIs",
                                                                "key for LVL0 emulated muon RoIs" };

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring Tool"};
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};   //!< Random number generator engine

  std::unique_ptr<TruthTrackSmearer> m_mySmearer{nullptr};
};

}   // end of namespace

#endif  // L0MUON_SMEARINGALG_H

