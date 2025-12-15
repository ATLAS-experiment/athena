/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolExampleAlgorithms/src/ReadData.cxx
 *  @brief This file contains the implementation for the ReadData class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: ReadData.cxx,v 1.26 2009-03-14 22:05:57 gemmeren Exp $
 **/

#include "ReadData.h"

// the user data-class definitions
#include "AthenaPoolExampleData/ExampleHitContainer.h"
#include "AthenaPoolExampleData/ExampleTrackContainer.h"

#include "PersistentDataModel/DataHeader.h"

#include "EventInfo/EventID.h"
#include "EventInfo/EventStreamInfo.h"
#include "EventBookkeeperMetaData/EventBookkeeperCollection.h"

#include <set>

using namespace AthPoolEx;

//___________________________________________________________________________
ReadData::ReadData(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator),
    p_SGinMeta("StoreGateSvc/InputMetaDataStore", name),
    p_SGmeta("StoreGateSvc/MetaDataStore", name)
{
}
//___________________________________________________________________________
StatusCode ReadData::initialize() {
   ATH_MSG_INFO("in initialize()");

   // Locate the StoreGateSvc and initialize our local ptr
   ATH_CHECK( p_SGinMeta.retrieve() );
   ATH_CHECK( p_SGmeta.retrieve() );

   ATH_CHECK( m_dataHeaderKey.initialize() );
   if (!m_exampleTrackKey.key().empty()) {
     ATH_CHECK( m_exampleTrackKey.initialize() );
   }
   ATH_CHECK( m_exampleHitKey.initialize() );
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReadData::execute (const EventContext& ctx) const {
   ATH_MSG_DEBUG("in execute()");

   SG::ConstIterator<EventStreamInfo> esi1, esi2;
   if (p_SGinMeta->retrieve(esi1, esi2).isFailure() || esi1 == esi2) {
      ATH_MSG_WARNING("Could not find EventStreamInfo");
   } else {
      for (; esi1 != esi2; ++esi1) {
         ATH_MSG_INFO("EventStreamInfo: Number of events = " << esi1->getNumberOfEvents());
         ATH_MSG_INFO("EventStreamInfo: ItemList:");
         for (const std::pair<CLID, std::string>& p : esi1->getItemList()) {
            ATH_MSG_INFO("CLID = " << p.first << ", key = " << p.second);
         }
         for (const EventType& et : esi1->getEventTypes()) {
            ATH_MSG_INFO("EventType: " << et.typeToString());
            ATH_MSG_INFO("TagInfo: " << et.get_detdescr_tags());
         }
      }
   }
   const std::string ebcKey = "EventSelector.Counter";
   if (p_SGmeta->contains<EventBookkeeperCollection>(ebcKey)) {
      const EventBookkeeperCollection* ebc = nullptr;
      if (p_SGmeta->retrieve(ebc, ebcKey).isFailure()) {
         ATH_MSG_FATAL("Could not find EventBookkeeperCollection, key =");
         return StatusCode::FAILURE;
      }
      for (const EventBookkeeper* bk : *ebc) {
         ATH_MSG_INFO("EventBookkeeper " << bk->getName() << " accepted events: = " << bk->getNAcceptedEvents());
      }
   }
   const std::string ebcInKey = "EventBookkeepers";
   if (p_SGinMeta->contains<EventBookkeeperCollection>(ebcInKey)) {
      const EventBookkeeperCollection* ebc = nullptr;
      if (p_SGinMeta->retrieve(ebc, ebcInKey).isFailure()) {
         ATH_MSG_FATAL("Could not find EventBookkeeperCollection, key =");
         return StatusCode::FAILURE;
      }
      for (const EventBookkeeper* bk : *ebc) {
         ATH_MSG_INFO("EventBookkeeper (In) " << bk->getName() << " accepted events: = " << bk->getNAcceptedEvents());
      }
   }

   SG::ReadHandle<DataHeader> dh (m_dataHeaderKey, ctx);
   for (const DataHeaderElement& dhe : *dh) {
      ATH_MSG_INFO("DataHeader (Event Content) " << dhe.getToken()->toString());
   }
   for (const DataHeaderElement& dhe : std::ranges::subrange(dh->beginProvenance(), dh->endProvenance())) {
      ATH_MSG_INFO("DataHeader (Provenance) " << dhe.getToken()->toString());
   }

   // Get the event header, print out event and run number
   const EventIDBase& eid = ctx.eventID();
   ATH_MSG_INFO("EventInfo event: " << eid.event_number() << " run: " << eid.run_number());
   // Get the DataObject, print out its contents
   ATH_MSG_INFO("Get Smart data ptr 1");

   if (!m_exampleTrackKey.key().empty()) {
     SG::ReadHandle<ExampleTrackContainer> tracks (m_exampleTrackKey, ctx);
     if (tracks.isValid()) {
       for (const ExampleTrack* track : *tracks) {
         ATH_MSG_INFO("Track pt = " << track->getPT() << " eta = " << track->getEta() << " phi = " << track->getPhi() << " detector = " << track->getDetector());
         try {
           double x = track->getElement1()->getX();
           ATH_MSG_INFO("ElementLink1 = " << x);
           ATH_MSG_INFO("ElementLink2 = " << track->getElement2()->getX());
           ATH_MSG_INFO("Link ElementLinkVector = " << track->getElementLinkVector()->size());
           for (const auto link : *track->getElementLinkVector()) {
             ATH_MSG_INFO("Element = " << (*link) << " : " << (*link)->getX());
           }
           ATH_MSG_INFO("Link Navigable = " << track->getNavigable()->size());
           for (const auto* elem : *track->getNavigable()) {
             ATH_MSG_INFO("Element = " << elem << " : " << elem->getX());
           }
           ATH_MSG_INFO("Link Weighted Navigable = " << track->getWeightedNavigable()->size());
           for (const auto* elem : *track->getWeightedNavigable()) {
             ATH_MSG_INFO("Element = " << elem << " : " << elem->getX());
           }
         } catch (...) {
           ATH_MSG_WARNING("Could not follow ExampleTrackContainer/MyTracks ElementLinks to ExampleHitContainer/MyHits");
         }
       }
     }
     else {
       ATH_MSG_INFO("Could not find ExampleTrackContainer/MyTracks");
     }
   }

   SG::ReadHandle<ExampleHitContainer> hits (m_exampleHitKey, ctx);
   if (hits.isValid()) {
     for (const ExampleHit* hit : *hits) {
       ATH_MSG_INFO("Hit x = " << hit->getX() << " y = " << hit->getY() << " z = " << hit->getZ() << " detector = " << hit->getDetector());
     }
     
     if (evtStore()->contains<ExampleHitContainer>("PetersHits")) {
       ATH_MSG_INFO("Found ExampleHitContainer/PetersHits (alias)");
     }
   }
   else {
     ATH_MSG_INFO("Could not find ExampleHitContainer/MyHits");
   }

   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReadData::finalize() {
   ATH_MSG_INFO("in finalize()");
   return StatusCode::SUCCESS;
}
