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

void CalcPartonHistory::FillGenericPartonHistory(
    const std::string& retrievalstring, const std::string& decorationstring,
    const int idx) {
  PtEtaPhiMVector v;
  int pdgId = 0;
  m_dec.decorateDefault(decorationstring);
  if (RetrieveParticleInfo(m_prefix + "_" + retrievalstring, v, pdgId, idx))
    m_dec.decorateParticle(decorationstring, v, pdgId);
}

void CalcPartonHistory::FillGenericPartonHistory(
    const std::vector<std::string>& retrievalStrings,
    const std::string& decorationstring, const int idx) {
  PtEtaPhiMVector v;
  int pdgId = 0;
  m_dec.decorateDefault(decorationstring);
  for (const auto& retrievalString : retrievalStrings) {
    if (RetrieveParticleInfo(m_prefix + "_" + retrievalString, v, pdgId, idx)) {
      m_dec.decorateParticle(decorationstring, v, pdgId);
      break;
    }
  }
}

void CalcPartonHistory::FillGenericPartonHistory(
    const std::vector<std::string>& retrievalStrings,
    const std::string& decorationstring) {
  FillGenericPartonHistory(retrievalStrings, decorationstring, 0);
}

void CalcPartonHistory::FillGenericVectorPartonHistory(
    const std::string& retrievalstring, const std::string& decorationstring) {
  std::vector<PtEtaPhiMVector> v;
  std::vector<int> pdgId;
  m_dec.decorateVectorDefault(decorationstring);
  if (RetrieveParticleInfo(m_prefix + "_" + retrievalstring, v, pdgId))
    m_dec.decorateVectorParticle(decorationstring, v, pdgId);
}

}  // namespace CP
