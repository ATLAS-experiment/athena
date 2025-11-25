/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVCBASE_ROBDATAPROVIDERSVC_H
#define BYTESTREAMCNVSVCBASE_ROBDATAPROVIDERSVC_H

/** ===============================================================
 * @class    ROBDataProviderSvc.h
 * @brief  ROBDataProvider class for accessing ROBData
 *
 *    Requirements: define a ROBData class in the scope
 *                  provide a method
 *       void getROBData(const vector<uint>& ids, vector<ROBData*>& v)
 *    Implementation: Use an interal map to store all ROBs
 *                    We can not assume any ROB/ROS relationship, no easy
 *                    way to search.
 *                    This implementation is used in offline
 *
 *    Created:      Sept 19, 2002
 *         By:      Hong Ma
 *    Modified:     Aug. 18  2003 (common class for Online/Offline)
 *         By:      Werner Wiedenmann
 *    Modified:     Apr  21  2005 (remove dependency on data flow repository)
 *         By:      Werner Wiedenmann
 */

#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "eformat/SourceIdentifier.h"
#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "Gaudi/Property.h"
#include <vector>
#include <map>
#include <memory>

class ROBDataProviderSvc :  public extends<AthService, IROBDataProviderSvc> {

public:
   /// ROB Fragment class
   using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

   /// Constructor
   ROBDataProviderSvc(const std::string& name, ISvcLocator* svcloc);

   /// initialize the service
   virtual StatusCode initialize() override;

   /// Add ROBFragments to cache for given ROB ids, ROB fragments may be retrieved with DataCollector
   virtual void addROBData(const EventContext& context, const std::vector<uint32_t>& robIds, const std::string_view callerName="UNKNOWN") override;

   /// Add a given LVL1/LVL2 ROBFragment to cache
   virtual void setNextEvent(const EventContext& context, const std::vector<OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment>& result) override;

   /// Add all ROBFragments of a RawEvent to cache
   virtual void setNextEvent(const EventContext& context, const RawEvent* re) override;

   /// Retrieve ROBFragments for given ROB ids from cache
   virtual void getROBData(const EventContext& context, const std::vector<uint32_t>& robIds, VROBFRAG& robFragments, 
			   const std::string_view callerName="UNKNOWN") override;

   /// Retrieve the whole event.
   virtual const RawEvent* getEvent(const EventContext& context) override;

   /// Store the status for the event.
   virtual void setEventStatus(const EventContext& context, uint32_t status) override;

   /// Retrieve the status for the event.
   virtual uint32_t getEventStatus(const EventContext& context) override;

   virtual void processCachedROBs(const EventContext& context, 
				  const std::function< void(const ROBF* )>& fn ) const override;

   virtual bool isEventComplete(const EventContext& /*context*/) const override { return true; }
   virtual int collectCompleteEventData(const EventContext& /*context*/, const std::string_view /*callerName*/ ) override {  return 0; }

protected:
   /// vector of ROBFragment class
   //typedef std::vector<ROBF*> VROBF;

   /// map for all the ROB fragments
   using ROBMAP = std::map<uint32_t, std::unique_ptr<const ROBF>, std::less<uint32_t>>;

  struct EventCache {
    const RawEvent* event = nullptr;
    uint32_t eventStatus = 0;
    uint32_t currentLvl1ID = 0;    
    ROBMAP robmap;
 
  };
  SG::SlotSpecificObj<EventCache> m_eventsCache;

   /// Remaining attributes are for configuration
   /// vector of Source ids and status words to be ignored for the ROB map
   using ArrayPairIntType = std::vector<std::pair<int, int>>;
   Gaudi::Property<ArrayPairIntType> m_filterRobWithStatus{
      this, "filterRobWithStatus", {}, "ROB IDs and status words to filter (full ROB SourceID)"};
   Gaudi::Property<ArrayPairIntType> m_filterSubDetWithStatus{
      this, "filterSubDetWithStatus", {}, "Sub-detector IDs and status words to filter"};

   /// map of full ROB Source ids and status words to be ignored for the ROB map
   using FilterRobMap = std::map<uint32_t, std::vector<uint32_t>>;
   FilterRobMap          m_filterRobMap;
   /// map of Sub Det Source ids and status words to be ignored for the ROB map
   using FilterSubDetMap = std::map<eformat::SubDetector, std::vector<uint32_t>>;
   FilterSubDetMap       m_filterSubDetMap;
   /// method to filter ROBs with given Status code
   bool filterRobWithStatus(const ROBF* rob);

   /// Filter out empty ROB fragments which are send by the ROS
   Gaudi::Property<bool> m_filterEmptyROB{this, "filterEmptyROB", false, "Filter out empty ROB fragments"};
   bool m_maskL2EFModuleID = false;    

private:
  static void robmapClear(ROBMAP& toclear);
};

#endif
