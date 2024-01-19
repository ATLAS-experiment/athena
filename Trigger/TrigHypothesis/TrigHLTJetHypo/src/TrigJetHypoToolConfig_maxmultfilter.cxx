/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
  /* create and return a RangeFilter with the configure range limits.*/

  FilterPtr fp = std::unique_ptr<IHypoJetVectorFilter>(nullptr);
  fp.reset(new MaxMultFilter(m_end));

  return fp;
}

StatusCode TrigJetHypoToolConfig_maxmultfilter::checkVals() const {
  if (m_end < 1u) {ATH_MSG_ERROR("MaxMultFilter < 1");
    return StatusCode::FAILURE;
  }  
  return StatusCode::SUCCESS;
}



