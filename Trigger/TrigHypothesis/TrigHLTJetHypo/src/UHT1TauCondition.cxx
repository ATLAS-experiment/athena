/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./UHT1TauCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/xAODJetAsIJet.h"
#include "TrigBjetHypo/safeLogRatio.h"

#include <sstream>
#include <cmath>
#include <TLorentzVector.h>

UHT1TauCondition::UHT1TauCondition(float workingPoint,
                             const std::string &decName_ptau,
                             const std::string &decName_pu,
                             const std::string &decName_isValid) :
  m_workingPoint(workingPoint),
  m_decName_ptau(decName_ptau),
  m_decName_pu(decName_pu),
  m_decName_isValid(decName_isValid)
{

}

float UHT1TauCondition::getUHT1TauDecValue(const pHypoJet &ip,
                                     const std::unique_ptr<ITrigJetHypoInfoCollector> &collector,
                                     const std::string &decName) const
{

  float momentValue = -1;
  if (!(ip->getAttribute(decName, momentValue)))
  {
    if (collector)
    {
      auto j_addr = static_cast<const void *>(ip.get());

      std::stringstream ss0;
      ss0 << "UHT1TauCondition: "
          << " unable to retrieve " << decName << '\n';
      std::stringstream ss1;
      ss1 << "     jet : (" << j_addr << ")";
      collector->collect(ss0.str(), ss1.str());
    }

    throw std::runtime_error("Impossible to retrieve decorator \'" + decName + "\' for jet hypo");
  }

  return momentValue;
}

float UHT1TauCondition::evaluateUHT1Tau(const float &ptau,
                                  const float &pu) const {
  return safeLogRatio(ptau, pu);
}

bool UHT1TauCondition::isSatisfied(const pHypoJet &ip,
                                const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const
{
  if (!m_decName_isValid.empty()) {
    // short circuit if uht1tau is invalid and we ask for a check
    //
    // we have to dynamic cast to xAOD jets here because there's no char
    // accessor on IJet and it's not clear if we need one generally.
    const auto* jet = dynamic_cast<const HypoJet::xAODJetAsIJet*>(ip.get());
    if (!jet) throw std::runtime_error("Fast uht1tau has to run on xAOD::Jet");
    char valid = (*jet->xAODJet())->getAttribute<char>(m_decName_isValid);
    if (valid == 0) return false;
  }

  // if we got this far check the uht1tau hypo
  float ptau = getUHT1TauDecValue(ip, collector, m_decName_ptau);
  float pu = getUHT1TauDecValue(ip, collector, m_decName_pu);
  float output = evaluateUHT1Tau(ptau, pu);

  bool pass = (output >= m_workingPoint);

  if (collector)
  {
    const void *address = static_cast<const void *>(this);

    std::stringstream ss0;
    ss0 << "UHT1TauCondition: (" << address
        << ")"
        << " pass: " << std::boolalpha << pass << '\n';

    auto j_addr = static_cast<const void *>(ip.get());
    std::stringstream ss1;
    ss1 << "     jet : (" << j_addr << ") "
        << m_decName_ptau << " value: " << ptau << '\n';

    collector->collect(ss0.str(), ss1.str());
  }

  return pass;
}

bool
UHT1TauCondition::isSatisfied(const HypoJetVector& ips,
			   const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto result =  isSatisfied(ips[0], c);
  return result;
}


std::string UHT1TauCondition::toString() const {
  std::stringstream ss;
  ss << "UHT1TauCondition (" << this << ") "
     << " Cleaning decs: "
     << m_decName_ptau << ", "
     << m_decName_pu << ", "
     <<'\n';

  return ss.str();
}
