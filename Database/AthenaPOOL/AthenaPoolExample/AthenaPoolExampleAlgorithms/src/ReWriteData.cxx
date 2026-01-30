/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file ReWriteData.cxx
 *  @brief This file contains the implementation for the ReWriteData class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "ReWriteData.h"

// the user data-class definitions
#include "AthenaPoolExampleData/ExampleHitContainer.h"
#include "AthenaPoolExampleData/ExampleTrackContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <cmath>

using namespace AthPoolEx;

//___________________________________________________________________________
ReWriteData::ReWriteData(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}
//___________________________________________________________________________
StatusCode ReWriteData::initialize() {
   ATH_MSG_INFO("in initialize()");
   if (m_exampleHitKey.key().empty()) {
     m_exampleTrackKey = "";
   }
   else {
     ATH_CHECK( m_exampleHitKey.initialize() );
     ATH_CHECK( m_exampleTrackKey.initialize() );
   }
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReWriteData::execute (const EventContext& ctx) const {
   ATH_MSG_DEBUG("in execute()");

   if (!m_exampleHitKey.key().empty()) {

     // Take in the ExampleHit, obtain pT, eta and phi. 
     SG::ReadHandle<ExampleHitContainer> hitCont (m_exampleHitKey, ctx);
     double pT = 0.0, eta = 0.0, phi = 0.0;
     for (const ExampleHit* hit : *hitCont) {
       ATH_MSG_INFO("Hit x = " << hit->getX() << " y = " << hit->getY() << " z = " << hit->getZ() << " detector = " << hit->getDetector());
       pT = pT + sqrt(hit->getX() * hit->getX() + hit->getY() * hit->getY());
       eta = eta + hit->getX() / hit->getZ();
       phi = phi + hit->getX() / hit->getY();
     }
     
     // Create an ExampleTrack object, set the hit values appropriately
     auto trackObj = std::make_unique<ExampleTrack>();
     trackObj->setPT(pT / hitCont->size());
     trackObj->setEta(eta);
     trackObj->setPhi(phi);
     trackObj->setDetector("Track made in: " + (*hitCont->begin())->getDetector());
     trackObj->getElementLink1()->toContainedElement(*hitCont, *hitCont->begin());
     ATH_MSG_INFO("ElementLink1 = " << trackObj->getElement1()->getX());
     trackObj->getElementLink2()->toIndexedElement(*hitCont, hitCont->size() - 1);
     ATH_MSG_INFO("ElementLink2 = " << trackObj->getElement2()->getX());
     
     // ElementLink creation
     ElementLink<ExampleHitContainer> eLink1, eLink2, eLink3;
     eLink1.toContainedElement(*hitCont, *hitCont->begin());
     trackObj->getElementLinkVector()->push_back(eLink1);
     eLink2.toIndexedElement(*hitCont, 1);
     trackObj->getElementLinkVector()->push_back(eLink2);
     eLink3.toContainedElement(*hitCont, (*hitCont)[3]);
     trackObj->getElementLinkVector()->push_back(eLink3);
     ATH_MSG_INFO("Link ElementLinkVector = " << trackObj->getElementLinkVector()->size());
     for (const auto link : *trackObj->getElementLinkVector()) {
       ATH_MSG_INFO("Element = " << (*link)->getX());
     }
     
     // Print out Navigable elements
     trackObj->getNavigable()->putElement(hitCont.cptr(), *hitCont->begin());
     trackObj->getNavigable()->putElement(hitCont.cptr(), (*hitCont)[5]);
     ATH_MSG_INFO("Link Navigable = " << trackObj->getNavigable()->size());
     for (const auto* elem : *trackObj->getNavigable()) {
       ATH_MSG_INFO("Element = " << elem->getX());
     }

     // Print out WeightedNavigable elements
     trackObj->getWeightedNavigable()->putElement(hitCont.cptr(), *hitCont->begin(), 3.33);
     trackObj->getWeightedNavigable()->putElement(hitCont.cptr(), (*hitCont)[5], 1.11);
     trackObj->getWeightedNavigable()->putElement(hitCont.cptr(), (*hitCont)[3], 5.55);
     ATH_MSG_INFO("Link Weighted Navigable = " << trackObj->getWeightedNavigable()->size());
     for (const auto* elem : *trackObj->getWeightedNavigable()) {
       ATH_MSG_INFO("Element = " << elem->getX());
     }
     
     // Print out Track info
     ATH_MSG_INFO("Track pt = " << trackObj->getPT() << " eta = " << trackObj->getEta() << " phi = " << trackObj->getPhi() << " detector = " << trackObj->getDetector());
     
     // Create Track container, record it. 
     auto trackCont = std::make_unique<ExampleTrackContainer>();
     trackCont->push_back(std::move(trackObj));
     SG::WriteHandle<ExampleTrackContainer> trackContH (m_exampleTrackKey, ctx);
     ATH_CHECK( trackContH.record (std::move (trackCont)) );
     
   } // end if

   ATH_MSG_INFO("registered all data");
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReWriteData::finalize() {
   ATH_MSG_INFO("in finalize()");
   return StatusCode::SUCCESS;
}
