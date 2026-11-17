//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBPARTICLEIDCHERENKOVMUTAG_H
#define TBREC_TBPARTICLEIDCHERENKOVMUTAG_H

#include "AthenaBaseComps/AthAlgorithm.h"

class TBPartIDCherenkovMuTag : public AthAlgorithm
{
 public:
  TBPartIDCherenkovMuTag(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBPartIDCherenkovMuTag();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
 private:
  Gaudi::Property<std::string> m_SGkeyscint{this, "SGScintkey", "ScintillatorCont", "SG key of the TBScintillatorCont"};
  Gaudi::Property<std::string> m_muonTagName{this, "MuonTagName", "muTag", "Name of the muon tag scintillator"};
  Gaudi::Property<std::string> m_CherenkovName{this, "CherenkovName", "C1", "Name of the Cherenkov counter"};
  Gaudi::Property<std::string> m_particleKey{this, "IdentifiedParticleKey", "TBIdentifiedParticle", "SG key of the output TBIdentifiedParticle"};
  Gaudi::Property<int> m_muonADCcut{this, "MuonADCcut", 500, "ADC cut for the muon tag"};
  Gaudi::Property<int> m_cherenkovADCcut{this, "CherenkovADCcut", 410, "ADC cut for the Cherenkov counter"};

  Gaudi::Property<bool> m_printSummary{this, "PrintSummary", false, "Print summary in finalize"};
  unsigned m_NElectrons{0},m_NPions{0},m_NMuons{0},m_Nunknown{0};
};

#endif
