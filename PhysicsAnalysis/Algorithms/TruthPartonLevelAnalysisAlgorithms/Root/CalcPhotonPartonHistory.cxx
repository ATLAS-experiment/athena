/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "PartonHistory/PartonHistoryUtils.h"
#include "VectorHelpers/DecoratorHelpers.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

void CalcPartonHistory::FillGammaPartonHistory(const std::string& parent) {
  std::string parentstring = parent.empty() ? "" : "_from_" + parent;
  PtEtaPhiMVector gamma;
  int gamma_origin = -1;

  m_dec.decorateDefault("MC_gamma" + parentstring);
  m_dec.decorateCustom("MC_gamma_origin", 0);

  if (Retrievep4Gamma(gamma, gamma_origin))
    m_dec.decorateParticle("MC_gamma" + parentstring, gamma);

  m_dec.decorateCustom("MC_gamma_origin", gamma_origin);
}
}  // namespace CP
