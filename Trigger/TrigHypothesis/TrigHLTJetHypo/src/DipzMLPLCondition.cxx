/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./DipzMLPLCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

DipzMLPLCondition::DipzMLPLCondition(double wp,
                            unsigned int capacity,
                            const std::string &decName_z,
                            const std::string &decName_negLogSigma2) :
  m_workingPoint(wp),
  m_capacity(capacity),
  m_likelihoodCalculator(decName_z, decName_negLogSigma2)
{

}

bool DipzMLPLCondition::isSatisfied(const HypoJetVector& ips,
				      const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {
  
  if(collector){
    std::stringstream ss0;
    const void* address = static_cast<const void*>(this);
    ss0 << "DipzMLPLCondition: (" << address << ") starts\n";
    collector -> collect(ss0.str(), "");
  }

  double logproduct = m_likelihoodCalculator(ips);

  bool pass = logproduct >= m_workingPoint;
  
  if(collector){
    std::stringstream ss0;
    const void* address = static_cast<const void*>(this);
    ss0 << "DipzMLPLCondition: (" << address << ") logproduct term "
	<< logproduct << " >= "
  << m_workingPoint << ": "
	<< std::boolalpha << pass <<  " jet group: \n";
    
    std::stringstream ss1;
    
    for(const auto& ip : ips){
      address = static_cast<const void*>(ip.get());
      ss1 << "    "  << address << " " << ip->eta() << " e " << ip->e() << '\n';
    }
    ss1 << '\n';
    collector -> collect(ss0.str(), ss1.str());
  }
  
  return pass;
  
}

std::string DipzMLPLCondition::toString() const {
  std::stringstream ss;
  const void* address = static_cast<const void*>(this);

  ss << "DipzMLPLCondition: (" << address << ") Capacity: " << m_capacity
    << " working point: " << m_workingPoint;
  ss <<'\n';

  return ss.str();
}
