/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file ReadCond.cxx
 *  @brief This file contains the implementation for the ReadCond class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: ReadCond.cxx,v 1.13 2008-12-10 21:28:11 gemmeren Exp $
 **/

#include "ReadCond.h"

// the user data-class definitions
#include "AthenaPoolExampleData/ExampleHitContainer.h"

using namespace AthPoolEx;

//___________________________________________________________________________
ReadCond::ReadCond(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator) {
}
//___________________________________________________________________________
StatusCode ReadCond::initialize() {
   ATH_MSG_INFO("in initialize()");
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReadCond::execute() {
   ATH_MSG_DEBUG("in execute()");

   if (detStore()->contains<ExampleHitContainer>("PedestalWriteData")) {
      const ExampleHitContainer* ep = nullptr;
      ATH_CHECK( detStore()->retrieve(ep, "PedestalWriteData") );
      for (const ExampleHit* obj : *ep) {
         ATH_MSG_INFO("Pedestal x = " << obj->getX() << " y = " << obj->getY() << " z = " << obj->getZ() << " string = " << obj->getDetector());
      }
   }
   if (detStore()->contains<ExampleHitContainer>("PedestalAppendData")) {
      const ExampleHitContainer* ep = nullptr;
      ATH_CHECK( detStore()->retrieve(ep, "PedestalAppendData") );
      for (const ExampleHit* obj : *ep) {
         ATH_MSG_INFO("Pedestal (2) x = " << obj->getX() << " y = " << obj->getY() << " z = " << obj->getZ() << " string = " << obj->getDetector());
      }
   }
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReadCond::finalize() {
   ATH_MSG_INFO("in finalize()");
   return StatusCode::SUCCESS;
}
