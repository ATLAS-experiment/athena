/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolExampleAlgorithms/src/PassNoneFilter.cxx
 *  @brief This file contains the implementation for the PassNoneFilter class.
 **/

#include "PassNoneFilter.h"

using namespace AthPoolEx;

PassNoneFilter::PassNoneFilter(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator) {}

StatusCode PassNoneFilter::initialize() { return StatusCode::SUCCESS; }

StatusCode PassNoneFilter::execute(const EventContext& ctx) {
   setFilterPassed(false, ctx);
   return StatusCode::SUCCESS;
}

StatusCode PassNoneFilter::finalize() { return StatusCode::SUCCESS; }
