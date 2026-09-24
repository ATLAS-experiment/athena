//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#include <traccc/alpaka/utils/get_device_info.hpp>
#include "AlpakaExampleAlg.h"


StatusCode AlpakaExampleAlg::execute(const EventContext&) const {

  ATH_MSG_INFO(traccc::alpaka::get_device_info());
  return StatusCode::SUCCESS;

}
