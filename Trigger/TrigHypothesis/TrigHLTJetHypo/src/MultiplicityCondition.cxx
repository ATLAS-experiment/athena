/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "./MultiplicityCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

MultiplicityCondition::MultiplicityCondition(std::size_t multMin,
					     std::size_t multMax):
  m_multMin{multMin}, m_multMax{multMax} {
}


bool
MultiplicityCondition::isSatisfied(const HypoJetVector& ips,
				      const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {
  
  auto mult = ips.size();
  bool pass = mult >= m_multMin and mult < m_multMax;
  
  if(collector){
    std::stringstream ss0;
    const void* address = static_cast<const void*>(this);
    ss0 << "MultCondition: (" << address << ") mult "
	<< mult << " "
	<< std::boolalpha << pass <<  " jet group: \n";
    
    collector -> collect(ss0.str(), "");
  }
  
  return pass;
  
}
    

std::string MultiplicityCondition::toString() const {
  std::stringstream ss;
  ss << "MultiplicityCondition: multMin: "
     << m_multMin
     << " multMax "
     << m_multMax << '\n';

  return ss.str();
}

