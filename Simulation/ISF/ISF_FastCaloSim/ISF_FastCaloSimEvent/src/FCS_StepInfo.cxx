/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ISF_FastCaloSimEvent/FCS_StepInfo.h"

#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/getMessageSvc.h"

#include <cmath>

double
ISF_FCS_Parametrization::FCS_StepInfo::diff2(const FCS_StepInfo &other) const {
  return (this->position().diff2(other.position()));
}

ISF_FCS_Parametrization::FCS_StepInfo &
ISF_FCS_Parametrization::FCS_StepInfo::operator+=(
    const ISF_FCS_Parametrization::FCS_StepInfo &other) {
  if (identify() != other.identify()) {
    MsgStream log(Athena::getMessageSvc(), "FCS_StepInfo");
    log << MSG::WARNING << "Cannot merge hits from different cells: "
        << identify() << " / " << other.identify() << endmsg;
    return *this;
  }

  if ((std::abs(energy()) > 1e-9) &&
      (std::abs(other.energy()) > 1e-9)) {
    const double eabssum = std::abs(energy()) + std::abs(other.energy());
    const double esum = energy() + other.energy();
    const double w1 = std::abs(energy()) / eabssum;
    const double w2 = std::abs(other.energy()) / eabssum;
    m_pos = w1 * m_pos + w2 * other.m_pos;
    setEnergy(esum);
    setTime(w1 * time() + w2 * other.time());

  } else if (std::abs(energy()) < 1e-9) {
    setEnergy(other.energy());
    setP(other.position());
    setTime(other.time());
  } else if (std::abs(other.energy()) < 1e-9) {
    // Keep the original hit.
  } else {
    MsgStream log(Athena::getMessageSvc(), "FCS_StepInfo");
    log << MSG::WARNING << "Cannot merge hits at the energy threshold. "
        << "Original: " << energy() << " " << position()
        << "; second: " << other.energy() << " " << other.position()
        << endmsg;
  }

  return *this;
}
