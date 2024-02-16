/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#
#include "./EMFCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include "./DebugInfoCollector.h"

#include <sstream>
#include <cmath>
#include <TLorentzVector.h>

EMFCondition::EMFCondition(double threshold) : m_min(threshold) {
}


bool EMFCondition::isSatisfied(const pHypoJet& ip,
				const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {

  float emf =999; 
  ip->getAttribute("EMFrac",emf);
  bool pass = m_min >= emf;
 
  if(collector){
    const void* address = static_cast<const void*>(this);

    std::stringstream ss0;
    ss0 << "EMFCondition: (" << address << ") " 
        << " emf thresh " << m_min
        << " pass: "  << std::boolalpha << pass << '\n';

    auto j_addr = static_cast<const void*>(ip.get());
    std::stringstream ss1;
    ss1 <<  "     jet : ("<< j_addr << ")"
        " emf " << emf << '\n';
    
    collector->collect(ss0.str(), ss1.str());

  }
  return pass;
}


bool 
EMFCondition::isSatisfied(const HypoJetVector& ips,
			   const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto result =  isSatisfied(ips[0], c);
  return result;
}


std::string EMFCondition::toString() const {
  std::stringstream ss;
  ss << "EMFCondition (" << this << ") "
     << " EMF threshold: " 
     << m_min
     <<'\n';

  return ss.str();
}
