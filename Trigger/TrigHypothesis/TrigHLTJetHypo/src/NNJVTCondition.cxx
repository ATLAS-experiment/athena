/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./NNJVTCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include <sstream>
#include <stdexcept>
#include <memory>

NNJVTCondition::NNJVTCondition(const std::string& nnjvt_name) :
  m_nnjvt_name(nnjvt_name)
{
  m_nnjvt_acc = std::make_unique<SG::ConstAccessor<float> >(m_nnjvt_name);
  m_nnjvtpass_acc = std::make_unique<SG::ConstAccessor<char> >(m_nnjvt_name+"Pass");
}

bool NNJVTCondition::isSatisfied(const HypoJetVector& ips, const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const{

  if(ips.size() != 1){
    std::stringstream ss;
    ss << "NNJVTCondition::isSatisfied must see exactly 1 particle, but received "
       << ips.size()
       << '\n';
    throw std::runtime_error(ss.str());
  }

  auto jet = ips[0];
  auto xJet = *jet->xAODJet();

  // Rely on the pass decorations to assess the NNJVT decision
  bool pass = (*m_nnjvtpass_acc)(*xJet);
  float nnjvt = (*m_nnjvt_acc)(*xJet);

  if(collector){
    std::stringstream ss0;
    const void* address = static_cast<const void*>(this);
    ss0 << "NNJVTCondition: (" << address 
        << ") nnjvt " << nnjvt
        << " pt "   << jet->pt()
        << " eta "  << jet->eta()
        << " pass: " <<std::boolalpha << pass <<  " jet group: \n";

    std::stringstream ss1;

    for(const auto& ip : ips){
      address = static_cast<const void*>(ip.get());
      ss1 << "    "  << address << " " << ip->eta() << " pt " << ip->pt() << '\n';
    }
    ss1 << '\n';
    collector -> collect(ss0.str(), ss1.str());
  }
  return pass;

}

std::string NNJVTCondition::toString() const {

  std::stringstream ss;
  const void* address = static_cast<const void*>(this);
  ss << "NNJVTCondition: (" << address << ") Capacity: " << s_capacity
     << " decoration name: " << m_nnjvt_name << '\n';
  
  return ss.str();
}
