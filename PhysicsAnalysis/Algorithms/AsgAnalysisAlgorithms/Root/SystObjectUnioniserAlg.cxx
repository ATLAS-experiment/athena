/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Teng Jian Khoo

#include "AsgAnalysisAlgorithms/SystObjectUnioniserAlg.h"

namespace CP
{
  // Define the following classes concretely for source code configuration.
  template class SystObjectUnioniserAlg<xAOD::Jet,xAOD::JetContainer>;
  template class SystObjectUnioniserAlg<xAOD::Electron,xAOD::ElectronContainer>;
  template class SystObjectUnioniserAlg<xAOD::Photon,xAOD::PhotonContainer>;
  template class SystObjectUnioniserAlg<xAOD::Muon,xAOD::MuonContainer>;
  template class SystObjectUnioniserAlg<xAOD::TauJet,xAOD::TauJetContainer>;
  template class SystObjectUnioniserAlg<xAOD::DiTauJet,xAOD::DiTauJetContainer>;
}
