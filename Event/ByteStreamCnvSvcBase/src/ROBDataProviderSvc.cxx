/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//===================================================================
//  Implementation of ROBDataProviderSvc
//  Revision: November 2017
//      MT readiness
//  Revision:  July 11, 2002
//      Modified for eformat
//  Revision:  Aug 18, 2003
//      Modified to use ROBFragments directly and include methods
//      for online
//  Revision:  Apr 21, 2005
//      Remove dependency on Level-2 Data Collector, create special
//      version for online
//  Revision:  Oct 29, 2006
//      Increase MAX_ROBFRAGMENTS to 2048 to cover the special case
//      when in a "localhost" partition the complete event is given
//      to the Event Filter as one single ROS fragment (this case
//      should not happen for normal running when several ROSes are
//      used and the ROB fragments are grouped under different ROS
//      fragments)
//  Revision:  Nov 10, 2008
//      Mask off the module ID from the ROB source identifier of the
//      L2 and EF result when storing it in the ROB map. This is necessary
//      when the L2/PT node ID is stored in the source identifier as
//      module ID. With this modification the L2 and EF result can still be
//      found as 0x7b0000 and 0x7c0000
//  Revision:  Jan 12, 2009
//      Allow removal of individual ROBs and ROBs from given subdetectors
//      from the internal ROB map according to a given status code.
//      This may be necessary when corrupted and incomplete ROB fragments
//      are forwarded to the algorithms and the converters are not yet
//      prepared to handle the specific cases.
//      The filtering can be configured with job options as:
//
//      for individual ROBs as :
//      ------------------------
//      ROBDataProviderSvc.filterRobWithStatus = [ (ROB SourceId, StatusCode to remove),
//                                                 (ROB SourceId 2, StatusCode to remove 2), ... ]
//      and:
//      ROBDataProviderSvc.filterRobWithStatus += [ (ROB SourceId n, StatusCode to remove n) ]
//
//      Example:
//      ROBDataProviderSvc.filterRobWithStatus  = [ (0x42002a,0x0000000f), (0x42002e,0x00000008) ]
//      ROBDataProviderSvc.filterRobWithStatus += [ (0x42002b,0x00000000) ]
//
//      for all ROBs of a given sub detector as :
//      -----------------------------------------
//      ROBDataProviderSvc.filterSubDetWithStatus = [ (Sub Det Id, StatusCode to remove),
//                                                    (Sub Det Id 2, StatusCode to remove 2), ... ]
//      and:
//      ROBDataProviderSvc.filterSubDetWithStatus += [ (Sub Det Id n, StatusCode to remove n) ]
//
//      Example:
//      ROBDataProviderSvc.filterSubDetWithStatus  = [ (0x41,0x00000000), (0x42,0x00000000) ]
//      ROBDataProviderSvc.filterSubDetWithStatus += [ (0x41,0xcb0002) ]
//
//      For valid ROB Source Ids, Sub Det Ids and ROB Status elements see the event format
//      document ATL-D-ES-0019 (EDMS)
//  Revision:  Jan 28, 2014
//      For Run 1 the module ID from the ROB source identifier of the
//      L2 and EF result needed to be masked off before storing the ROB fragment 
//      in the ROB map. The module ID for these fragments contained an identifier 
//      of the machine on which they were produced. This produced as many different ROB IDs
//      for these fragments as HLT processors were used. The module IDs were not useful 
//      for analysis and needed to be masked off from these fragments in the ROB map in order 
//      to allow the access to the L2 or the EF result with the generic identifiers  
//      0x7b0000 and 0x7c0000. Also an event contained only one L2 and EF result.
//      From Run 2 on (eformat version 5) the HLT processor identifier is not anymore
//      stored in the module ID of the HLT result. Further there can be in one event several HLT result
//      records with the source identifier 0x7c. The different HLT results are distinguished
//      now with the module ID. A module ID 0 indicates a physiscs HLT result as before, while
//      HLT results with module IDs different from zero are produced by data scouting chains. 
//      In Run 2 the module ID should be therefore not any more masked.
//      The masking of the moduleID is switched on when a L2 result is found in the event or the
//      event header contains L2 trigger info words. This means the data were produced with run 1 HLT system.
//  Revision: Aug 2026
//      Remove HLT-specific interfaces related to ROB requests and partial events.
//      Remove support for Run-1 data, i.e. the L2/EF module ID masking.
//
//===================================================================

#include "ByteStreamCnvSvcBase/ROBDataProviderSvc.h"
#include "eformat/Status.h"

#include <algorithm>


/// Initialization.
StatusCode ROBDataProviderSvc::initialize() {
   ATH_MSG_INFO("Initializing");

   for (const auto& [srcid, status] : m_filterRobWithStatus) {
      eformat::helper::SourceIdentifier src(srcid);
      if (src.human_detector() != "UNKNOWN") {
         m_filterRobMap[src.code()].push_back(status);
      }
   }
   for (const auto& [subdet, status] : m_filterSubDetWithStatus) {
      eformat::helper::SourceIdentifier src(static_cast<eformat::SubDetector>(subdet), 0);
      if (src.human_detector() != "UNKNOWN") {
         m_filterSubDetMap[src.subdetector_id()].push_back(status);
      }
   }
   ATH_MSG_INFO("---> Filter out empty ROB fragments = " << std::boolalpha << m_filterEmptyROB.value());
   ATH_MSG_INFO("---> Filter out specific ROBs by Status Code: # ROBs = " << m_filterRobMap.size());

   for (const auto& [id, status_vec] : m_filterRobMap) {
      eformat::helper::SourceIdentifier src(id);
      ATH_MSG_INFO("      RobId=0x" << MSG::hex << id << " -> in Sub Det = " << src.human_detector());
      for (uint32_t status : status_vec) {
         eformat::helper::Status tmpstatus(status);
         ATH_MSG_INFO("         Status Code=0x"
                      << MSG::hex << std::setfill( '0' ) << std::setw(8) << tmpstatus.code()
                      << " Generic Part=0x" << std::setw(4) << tmpstatus.generic()
                      << " Specific Part=0x" << std::setw(4) << tmpstatus.specific());
      }
   }

   ATH_MSG_INFO("---> Filter out Sub Detector ROBs by Status Code: # Sub Detectors = " <<
                m_filterSubDetMap.size());

   for (const auto& [det, status_vec] : m_filterSubDetMap) {
      eformat::helper::SourceIdentifier src(det, 0);
      ATH_MSG_INFO("      SubDetId=0x" << MSG::hex << det << " -> " << src.human_detector());
      for (uint32_t status : status_vec) {
         eformat::helper::Status tmpstatus(status);
         ATH_MSG_INFO("         Status Code=0x" <<
                      MSG::hex << std::setfill( '0' ) << std::setw(8) << tmpstatus.code() <<
                      " Generic Part=0x" << std::setw(4) << tmpstatus.generic() <<
                      " Specific Part=0x" << std::setw(4) << tmpstatus.specific());
      }
   }
   return StatusCode::SUCCESS;
}


/// Add a new RAW event and rebuild map.
void ROBDataProviderSvc::setNextEvent( const EventContext& ctx, const RawEvent* re ) {

   EventCache* cache = m_eventsCache.get(ctx);
   // assign the event
   cache->event=re;
   // clear the old map
   cache->robmap.clear();
   // set the LVL1 id
   cache->currentLvl1ID = re->lvl1_id();

   // loop over all ROBs
   auto iter = re->child_iter();
   while (OFFLINE_FRAGMENTS_NAMESPACE::PointerType fp = iter.next()) {
      // create ROBFragment
      auto rob = std::make_unique<const ROBF>(fp);
      const uint32_t id = rob->source_id();

      if (filterRobWithStatus(rob.get())) {
         if (rob->nstatus() > 0) {
            const uint32_t* it_status;
            rob->status(it_status);
            eformat::helper::Status tmpstatus(*it_status);
            ATH_MSG_DEBUG("---> ROB Id = 0x" << MSG::hex << id << std::setfill('0') <<
                          " with Generic Status Code = 0x" << std::setw(4) << tmpstatus.generic() <<
                          " and Specific Status Code = 0x" << std::setw(4) << tmpstatus.specific() << MSG::dec <<
                          " removed for L1 Id = " << cache->currentLvl1ID);
         }
         rob.reset();
      } else if ((rob->rod_ndata() == 0) && m_filterEmptyROB) {
         ATH_MSG_DEBUG("---> Empty ROB Id = 0x" << MSG::hex << id << MSG::dec <<
                       " removed for L1 Id = " << cache->currentLvl1ID);
         rob.reset();
      } else {
         // add to the map, warn in case of overwrite
         const auto& [itr, new_entry] = cache->robmap.insert_or_assign(id, std::move(rob));
         if (!new_entry) {
            ATH_MSG_WARNING("ROBDataProviderSvc:: Duplicate ROBID 0x" << MSG::hex << id <<
                            " found. " << MSG::dec << "Overwriting the previous one.");
         }
      }
   }
   ATH_MSG_DEBUG("---> setNextEvent offline for " << name() <<
                 ", current LVL1 id = " << cache->currentLvl1ID <<
                 ", size of ROB cache = " << cache->robmap.size());
}


/// Return ROBData for ROB ids.
void ROBDataProviderSvc::getROBData(const EventContext& ctx, const std::vector<uint32_t>& ids,
                                    VROBFRAG& robFragments, const std::string_view callerName) {

   EventCache* cache = m_eventsCache.get(ctx);

   for (uint32_t id : ids) {
      const auto map_it = cache->robmap.find(id);
      if (map_it != cache->robmap.end()) {
         robFragments.push_back(map_it->second.get());
      } else {
        ATH_MSG_DEBUG("Failed to find ROB for id 0x" << MSG::hex << id << MSG::dec << ", Caller Name = " << callerName);
      }
   }
}


/// Retrieve the whole event.
const RawEvent* ROBDataProviderSvc::getEvent( const EventContext& ctx ) {
   return m_eventsCache.get(ctx)->event;
}


/// Set the status for the event.
void ROBDataProviderSvc::setEventStatus(const EventContext& ctx, uint32_t status) {
   m_eventsCache.get(ctx)->eventStatus = status;
}


/// Retrieve the status for the event.
uint32_t ROBDataProviderSvc::getEventStatus( const EventContext& ctx ) {
   return m_eventsCache.get(ctx)->eventStatus;
}


void ROBDataProviderSvc::processCachedROBs(const EventContext& ctx,
					   const std::function< void(const ROBF* )>& fn ) const {
   for ( const auto&  el : m_eventsCache.get(ctx)->robmap ) {
      fn( el.second.get() );
   }
}


/// Filter ROB with Sub Detector Id and Status Code
bool ROBDataProviderSvc::filterRobWithStatus(const ROBF* rob) {
   // No filter criteria defined
   if (m_filterRobMap.empty() && m_filterSubDetMap.empty()) {
      return false;
   }
   // There should be at least one status element if there was an error
   // in case there are 0 status elements then there was no known error
   // (see event format document ATL-D-ES-0019 (EDMS))
   if (rob->nstatus() == 0) {
      return false;
   }
   // The ROB has at least one status element, access it via an iterator
   const uint32_t* rob_it_status;
   rob->status(rob_it_status);

   // Build the full ROB Sourceidentifier
   eformat::helper::SourceIdentifier src(rob->rob_source_id());
   // Check if there is a ROB specific filter rule defined for this ROB Id and match the status code
   if (auto it = m_filterRobMap.find(src.code()); it != m_filterRobMap.end()) {
      if (std::ranges::contains(it->second, *rob_it_status))
         return true;
   }

   // Check if there is a sub detector specific filter rule defined for this ROB Id and match the status code
   if (auto it = m_filterSubDetMap.find(src.subdetector_id()); it != m_filterSubDetMap.end()) {
      if (std::ranges::contains(it->second, *rob_it_status))
         return true;
   }

   return false;
}
