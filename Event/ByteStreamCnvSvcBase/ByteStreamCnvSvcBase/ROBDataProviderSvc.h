/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVCBASE_ROBDATAPROVIDERSVC_H
#define BYTESTREAMCNVSVCBASE_ROBDATAPROVIDERSVC_H

#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "ByteStreamData/RawEvent.h"

#include "Gaudi/Property.h"
#include "eformat/SourceIdentifier.h"

#include <vector>
#include <memory>
#include <unordered_map>

/**
 * ROBDataProviderSvc provides access to individual ROB fragments.
 *
 *    Created:      Sept 19, 2002
 *         By:      Hong Ma
 *    Modified:     Aug. 18  2003 (common class for Online/Offline)
 *         By:      Werner Wiedenmann
 *    Modified:     Apr  21  2005 (remove dependency on data flow repository)
 *         By:      Werner Wiedenmann
 *    Modified:     Aug 2026 (major rewrite/cleanup for Phase-II)
 *         By:      Frank Winklmeier
 */
class ROBDataProviderSvc : public extends<AthService, IROBDataProviderSvc> {

public:
   /// ROB Fragment class
   using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

   /// Base class constructor
   using base_class::base_class;

   /// Initialize
   virtual StatusCode initialize() override;

   /// Add all ROBFragments of a RawEvent to cache.
   virtual void setNextEvent(const EventContext& ctx, const RawEvent* re) override;

   /// Retrieve ROBFragments for given ROB ids from cache.
   virtual void getROBData(const EventContext& ctx, const std::vector<uint32_t>& robIds,
                           VROBFRAG& robFragments,
                           const std::string_view callerName="UNKNOWN") override;

   /// Retrieve the whole event.
   virtual const RawEvent* getEvent(const EventContext& ctx) override;

   /// Store the status for the event.
   virtual void setEventStatus(const EventContext& ctx, uint32_t status) override;

   /// Retrieve the status for the event.
   virtual uint32_t getEventStatus(const EventContext& ctx) override;

   virtual void processCachedROBs(const EventContext& ctx,
                                  const std::function< void(const ROBF* )>& fn ) const override;

private:
   /// method to filter ROBs with given Status code
   bool filterRobWithStatus(const ROBF* rob);

   /// ROB fragment cache per slot
   struct EventCache {
      const RawEvent* event = nullptr;
      uint32_t eventStatus = 0;
      uint32_t currentLvl1ID = 0;
      std::unordered_map<uint32_t, std::unique_ptr<const ROBF>> robmap;
   };
   SG::SlotSpecificObj<EventCache> m_eventsCache;

   /// map of full ROB Source ids and status words to be ignored for the ROB map
   std::unordered_map<uint32_t, std::vector<uint32_t>> m_filterRobMap;

   /// map of Sub Det Source ids and status words to be ignored for the ROB map
   std::unordered_map<eformat::SubDetector, std::vector<uint32_t>> m_filterSubDetMap;

   // Properties
   Gaudi::Property<std::vector<std::pair<int, int>>> m_filterRobWithStatus{
      this, "filterRobWithStatus", {}, "ROB IDs and status words to filter (full ROB SourceID)"};
   Gaudi::Property<std::vector<std::pair<int, int>>> m_filterSubDetWithStatus{
      this, "filterSubDetWithStatus", {}, "Sub-detector IDs and status words to filter"};
   Gaudi::Property<bool> m_filterEmptyROB{
      this, "filterEmptyROB", false, "Filter out empty ROB fragments"};
};

#endif
