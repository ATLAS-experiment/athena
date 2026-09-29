/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "VectorHelpers/DecoratorHelpers.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

void CalcPartonHistory::FillGammaPartonHistory() {
  PtEtaPhiMVector gamma;
  int gamma_origin = 0;

  m_dec.decorateDefault("MC_gamma");

  if (Retrievep4Gamma(gamma, gamma_origin))
    m_dec.decorateParticle("MC_gamma", gamma);

  m_dec.decorateCustom("MC_gamma_origin", gamma_origin);
}
}  // namespace CP
