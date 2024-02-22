/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "./GNTauCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/xAODJetAsIJet.h"
#include "TrigBjetHypo/safeLogRatio.h"

#include <sstream>
#include <cmath>
#include <TLorentzVector.h>

GNTauCondition::GNTauCondition(double workingPoint,
                             const std::string &decName_ptau,
                             const std::string &decName_pu,
                             const std::string &decName_isValid) :
  m_workingPoint(workingPoint),
  m_decName_ptau(decName_ptau),
  m_decName_pu(decName_pu),
  m_decName_isValid(decName_isValid)
{

}

float GNTauCondition::getGNTauDecValue(const pHypoJet &ip,
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
      ss0 << "GNTauCondition: "
          << " unable to retrieve " << decName << '\n';
      std::stringstream ss1;
      ss1 << "     jet : (" << j_addr << ")";
      collector->collect(ss0.str(), ss1.str());
    }

    throw std::runtime_error("Impossible to retrieve decorator \'" + decName + "\' for jet hypo");
  }

  return momentValue;
}

float GNTauCondition::evaluateGNTau(const float &gntau_ptau,
                                  const float &gntau_pu) const {
  return safeLogRatio(gntau_ptau, gntau_pu);
}

bool GNTauCondition::isSatisfied(const pHypoJet &ip,
                                const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const
{
  if (!m_decName_isValid.empty()) {
    // short circuit if gntau is invalid and we ask for a check
    //
    // we have to dynamic cast to xAOD jets here because there's no char
    // accessor on IJet and it's not clear if we need one generally.
    const auto* jet = dynamic_cast<const HypoJet::xAODJetAsIJet*>(ip.get());
    if (!jet) throw std::runtime_error("Fast gntau has to run on xAOD::Jet");
    char valid = (*jet->xAODJet())->getAttribute<char>(m_decName_isValid);
    if (valid == 0) return false;
  }

  // if we got this far check the gntau hypo
  float gntau_ptau = getGNTauDecValue(ip, collector, m_decName_ptau);
  float gntau_pu = getGNTauDecValue(ip, collector, m_decName_pu);
  float gntau_output = evaluateGNTau(gntau_ptau, gntau_pu);

  bool pass = (gntau_output >= m_workingPoint);

  if (collector)
  {
    const void *address = static_cast<const void *>(this);

    std::stringstream ss0;
    ss0 << "GNTauCondition: (" << address
        << ")"
        << " pass: " << std::boolalpha << pass << '\n';

    auto j_addr = static_cast<const void *>(ip.get());
    std::stringstream ss1;
    ss1 << "     jet : (" << j_addr << ") "
        << m_decName_ptau << " value: " << gntau_ptau << '\n';

    collector->collect(ss0.str(), ss1.str());
  }

  return pass;
}

bool
GNTauCondition::isSatisfied(const HypoJetVector& ips,
			   const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto result =  isSatisfied(ips[0], c);
  return result;
}


std::string GNTauCondition::toString() const {
  std::stringstream ss;
  ss << "GNTauCondition (" << this << ") "
     << " Cleaning decs: "
     << m_decName_ptau << ", "
     << m_decName_pu << ", "
     <<'\n';

  return ss.str();
}
