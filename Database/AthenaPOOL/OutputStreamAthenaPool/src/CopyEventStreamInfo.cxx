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
#include <algorithm>

//___________________________________________________________________________
CopyEventStreamInfo::CopyEventStreamInfo(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent) :
  base_class(type, name, parent) {
}
//___________________________________________________________________________
StatusCode CopyEventStreamInfo::initialize() {
   ATH_MSG_DEBUG("Initializing " << name());
   // Locate the MetaDataSvc and InputMetaDataStore
   ATH_CHECK( m_metaDataSvc.retrieve() );
   ATH_CHECK( m_inputMetaDataStore.retrieve() );

   return StatusCode::SUCCESS;
}


StatusCode CopyEventStreamInfo::beginInputFile(const SG::SourceID&)
{
   std::vector<std::string> keys = m_keys;
   if (keys.empty()) {
      m_inputMetaDataStore->keys<EventStreamInfo>(keys);
   } else {
     // remove keys not in the InputMetaDataStore
     std::erase_if(keys, [this](const std::string& key) {
               return !m_inputMetaDataStore->contains<EventStreamInfo>(key);
             });
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
            for (const auto& elem : evtStrInfo_in->getRunNumbers()) {
               evtStrInfo_out->insertRunNumber(elem);
            }
            for (const auto& elem : evtStrInfo_in->getLumiBlockNumbers()) {
               evtStrInfo_out->insertLumiBlockNumber(elem);
            }
            for (const auto& elem : evtStrInfo_in->getProcessingTags()) {
               evtStrInfo_out->insertProcessingTag(elem);
            }
            for (const auto& [classId, key] : evtStrInfo_in->getItemList()) {
               evtStrInfo_out->insertItemList(classId, key);
            }
            for (const auto& elem : evtStrInfo_in->getEventTypes()) {
               evtStrInfo_out->insertEventType(elem);
            }
         }
      }
   }
   return StatusCode::SUCCESS;
}
StatusCode CopyEventStreamInfo::endInputFile(const SG::SourceID&)
{
   return StatusCode::SUCCESS;
}
StatusCode CopyEventStreamInfo::metaDataStop()
{
   return StatusCode::SUCCESS;
}
