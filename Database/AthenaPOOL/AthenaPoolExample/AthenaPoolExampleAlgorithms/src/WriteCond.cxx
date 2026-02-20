/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file WriteCond.cxx
 *  @brief This file contains the implementation for the WriteCond class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: WriteCond.cxx,v 1.15 2008-12-10 21:28:11 gemmeren Exp $
 **/

#include "WriteCond.h"

// the user data-class definitions
#include "AthenaPoolExampleData/ExampleHitContainer.h"
#include "StoreGate/ReadHandle.h"

using namespace AthPoolEx;

//___________________________________________________________________________
WriteCond::WriteCond(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}
//___________________________________________________________________________
StatusCode WriteCond::initialize() {
   ATH_MSG_INFO("in initialize()");

   auto pPedestal = std::make_unique<ExampleHitContainer>();
   auto pEntry = std::make_unique<ExampleHit>();
   pEntry->setDetector("<");
   pPedestal->push_back(std::move(pEntry));

   ATH_CHECK( detStore()->record(std::move(pPedestal), m_conditionName) );

   ATH_CHECK( m_exampleHitKey.initialize() );
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode WriteCond::execute (const EventContext& ctx) const {
   ATH_MSG_DEBUG("in execute()");

   SG::ReadHandle<ExampleHitContainer> hits (m_exampleHitKey, ctx);
   ExampleHitContainer* ep = nullptr;
   ATH_CHECK( detStore()->retrieve(ep, m_conditionName) );
   ExampleHit* pEntry = *ep->begin();
   for (const ExampleHit* hit : *hits) {
     ATH_MSG_INFO("Hit x = " << hit->getX() << " y = " << hit->getY() << " z = " << hit->getZ() << " detector = " << hit->getDetector());
     pEntry->setX(pEntry->getX() + m_offset + hit->getX() * (1.0 + m_weight));
     pEntry->setY(pEntry->getY() + m_offset + hit->getY() * (1.0 + m_weight));
     pEntry->setZ(pEntry->getZ() + m_offset + hit->getZ() * (1.0 + m_weight));
     pEntry->setDetector(pEntry->getDetector() + ".");
   }
   pEntry->setDetector(pEntry->getDetector() + "o");

   ATH_MSG_INFO("registered all data");
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode WriteCond::stop() {
   ExampleHitContainer* ep = nullptr;
   ATH_CHECK( detStore()->retrieve(ep, m_conditionName) );
   ExampleHit* pEntry = *ep->begin();
   pEntry->setDetector(pEntry->getDetector() + ">");
   ATH_MSG_INFO("in finalize()");
   ATH_MSG_INFO("Pedestal x = " << pEntry->getX() << " y = " << pEntry->getY() << " z = " << pEntry->getZ() << " string = " << pEntry->getDetector());
   return StatusCode::SUCCESS;
}
