/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file CopyEventStreamInfo.cxx
 *  @brief This file contains the implementation for the CopyEventStreamInfo class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "CopyEventStreamInfo.h"

#include "EventInfo/EventStreamInfo.h"
#include "StoreGate/StoreGateSvc.h"

//___________________________________________________________________________
CopyEventStreamInfo::CopyEventStreamInfo(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent) :
  base_class(type, name, parent),
  m_metaDataSvc("MetaDataSvc", name),
  m_inputMetaDataStore("StoreGateSvc/InputMetaDataStore", name) {
}
//___________________________________________________________________________
CopyEventStreamInfo::~CopyEventStreamInfo() {
}
//___________________________________________________________________________
StatusCode CopyEventStreamInfo::initialize() {
   ATH_MSG_INFO("Initializing " << name());
   // Locate the MetaDataSvc and InputMetaDataStore
   ATH_CHECK( m_metaDataSvc.retrieve() );
   ATH_CHECK( m_inputMetaDataStore.retrieve() );

   return(StatusCode::SUCCESS);
}


StatusCode CopyEventStreamInfo::beginInputFile(const SG::SourceID&)
{
   std::vector<std::string> keys = m_keys;
   if (keys.empty()) {
      m_inputMetaDataStore->keys<EventStreamInfo>(keys);
   } else {
     // remove keys not in the InputMetaDataStore
     keys.erase(
         std::remove_if(
             keys.begin(), keys.end(),
             [this](std::string& key) {
               return !m_inputMetaDataStore->contains<EventStreamInfo>(key);
             }),
         keys.end());
   }

   // If the input file doesn't have any event stream info metadata,
   // then finish right away:
   if (keys.empty()) return StatusCode::SUCCESS;

   for (const auto &key : keys) {
      // Ignore versioned container
      if (key.substr(0, 1) == ";" && key.substr(3, 1) == ";") {
         ATH_MSG_VERBOSE( "Ignore versioned container: " << key );
         continue;
      }
      std::list<SG::ObjectWithVersion<EventStreamInfo> > allVersions;
      ATH_CHECK( m_inputMetaDataStore->retrieveAllVersions(allVersions, key) );

      EventStreamInfo* evtStrInfo_out = 0;
      for (SG::ObjectWithVersion<EventStreamInfo>& obj : allVersions) {
         const EventStreamInfo* evtStrInfo_in = obj.dataObject.cptr();
         evtStrInfo_out = m_metaDataSvc->tryRetrieve<EventStreamInfo>(key);
         if( !evtStrInfo_out ) {
            auto esinfo_up = std::make_unique<EventStreamInfo>(*evtStrInfo_in);
            ATH_CHECK( m_metaDataSvc->record( std::move(esinfo_up), key ) );
         } else {
            evtStrInfo_out->addEvent(evtStrInfo_in->getNumberOfEvents());
            for (auto elem = evtStrInfo_in->getRunNumbers().begin(),
                        lastElem = evtStrInfo_in->getRunNumbers().end(); 
                        elem != lastElem; elem++) {
               evtStrInfo_out->insertRunNumber(*elem);
            }
            for (auto elem = evtStrInfo_in->getLumiBlockNumbers().begin(),
                        lastElem = evtStrInfo_in->getLumiBlockNumbers().end(); 
                        elem != lastElem; elem++) {
               evtStrInfo_out->insertLumiBlockNumber(*elem);
            }
            for (auto elem = evtStrInfo_in->getProcessingTags().begin(),
                        lastElem = evtStrInfo_in->getProcessingTags().end(); 
                        elem != lastElem; elem++) {
               evtStrInfo_out->insertProcessingTag(*elem);
            }
            for (auto elem = evtStrInfo_in->getItemList().begin(),
                        lastElem = evtStrInfo_in->getItemList().end(); 
                        elem != lastElem; elem++) {
               evtStrInfo_out->insertItemList((*elem).first, (*elem).second);
            }
            for (auto elem = evtStrInfo_in->getEventTypes().begin(),
                        lastElem = evtStrInfo_in->getEventTypes().end(); 
                        elem != lastElem; elem++) {
               evtStrInfo_out->insertEventType(*elem);
            }
         }
      }
   }
   return(StatusCode::SUCCESS);
}
StatusCode CopyEventStreamInfo::endInputFile(const SG::SourceID&)
{
   return(StatusCode::SUCCESS);
}
StatusCode CopyEventStreamInfo::metaDataStop()
{
   return(StatusCode::SUCCESS);
}
