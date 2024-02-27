/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>

#include "./MaxMultFilter.h"
#include "./HypoJetPreds.h"  // HypoJetPtGreater


MaxMultFilter::MaxMultFilter(std::size_t end, double etaMin, double etaMax):
  m_end(end), m_etaMin(etaMin), m_etaMax(etaMax), m_nToSort(end) {}

HypoJetVector
MaxMultFilter::filter(const HypoJetVector& jv,
		    const std::unique_ptr<ITrigJetHypoInfoCollector>&) const {
  
  auto filtered = HypoJetVector();
  std::copy_if(jv.begin(), jv.end(), std::back_inserter(filtered), 
        [this](const pHypoJet& jp){
				return (std::abs(jp->eta())<this->m_etaMax && std::abs(jp->eta())>this->m_etaMin);} );

  auto nToSort = std::min(m_nToSort, filtered.size());
  
  std::partial_sort(filtered.begin(),
		    filtered.begin() + nToSort,
		    filtered.end(),
		    HypoJetPtGreater());

  filtered.resize(nToSort);

  return filtered;
}

std::string MaxMultFilter::toString() const {
  std::stringstream ss;
  const void* address = static_cast<const void*>(this);
  ss << "MaxMultFilter: (" << address << ") "
    << " eta range [" << m_etaMin << ", " << m_etaMax << "]:"
    << " end " << m_end << '\n';
  return ss.str();
}


std::ostream& operator<<(std::ostream& os, const MaxMultFilter& cf){
  os << cf.toString();
  return os;
}
