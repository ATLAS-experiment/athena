/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
  Configurer for a HypoJetVector MaxMultFilter
*/

#include "TrigJetHypoToolConfig_maxmultfilter.h"
#include "MaxMultFilter.h"

#include "GaudiKernel/StatusCode.h"
#include <vector>


TrigJetHypoToolConfig_maxmultfilter::TrigJetHypoToolConfig_maxmultfilter(const std::string& type,
								     const std::string& name,
								     const IInterface* parent) :
  base_class(type, name, parent){
  
}


StatusCode TrigJetHypoToolConfig_maxmultfilter::initialize() {
  CHECK(checkVals());
  return StatusCode::SUCCESS;
}

FilterPtr
TrigJetHypoToolConfig_maxmultfilter::getHypoJetVectorFilter() const {
  /* create and return a MaxMultFilter with the configured range limits.*/

  FilterPtr fp = std::unique_ptr<IHypoJetVectorFilter>(nullptr);
  auto a2d = ArgStrToDouble();
  fp.reset(new MaxMultFilter(m_end, a2d(m_min), a2d(m_max)));

  return fp;
}

StatusCode TrigJetHypoToolConfig_maxmultfilter::checkVals() const {
  auto a2d = ArgStrToDouble();

  auto min_val = a2d(m_min);
  auto max_val = a2d(m_max);  
  
  if (min_val > max_val){
    ATH_MSG_ERROR(" min eta >  max eta: " << min_val << max_val);
    return StatusCode::FAILURE;
  }

  if (m_end < 1u) {
    ATH_MSG_ERROR("MaxMultFilter < 1: " << m_end);
    return StatusCode::FAILURE;
  }  
  return StatusCode::SUCCESS;
}



