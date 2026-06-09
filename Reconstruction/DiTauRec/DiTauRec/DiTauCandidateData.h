/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_DITAUCANDIDATEDATA_H
#define DITAUREC_DITAUCANDIDATEDATA_H

#include "xAODJet/JetContainer.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODTau/DiTauJetAuxContainer.h"

#include <vector>

class CaloCell;
namespace fastjet{
  class PseudoJet;
}


class DiTauCandidateData {
 public:
  xAOD::DiTauJet* xAODDiTau;
  xAOD::DiTauJetContainer* xAODDiTauContainer;
  xAOD::DiTauJetAuxContainer* diTauAuxContainer;

  const xAOD::Jet* seed{};
  const xAOD::JetContainer* seedContainer{};
  std::vector<fastjet::PseudoJet> subjets;
  std::vector<const CaloCell*> subjetCells;

  float Rjet{};
  float Rsubjet{};
  float Rcore{};

};

#endif // DITAUREC_DITAUCANDIDATEDATA_H
