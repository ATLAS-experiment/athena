//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "AlgRegister.h"
#include "IGSAlg.h"

#include <sstream>


namespace GlobalSim {
    
  std::unique_ptr<IGSAlg> AlgRegister::get(const std::string& label) const {
    const auto& iter = m_ledger.find(label);
    
    if (iter == std::cend(m_ledger)) {
      std::stringstream ss;
      ss << "Key " << label << " not in registery" << '\n';
      throw std::runtime_error(ss.str());
      }
    return (*iter).second();
  }
  
  bool AlgRegister::insert(const std::string label, CreatorFn* f){
    return  (m_ledger.insert({label, f})).second;
  }
}
