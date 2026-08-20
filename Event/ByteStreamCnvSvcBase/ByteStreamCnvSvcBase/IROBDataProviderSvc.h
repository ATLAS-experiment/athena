/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IROBDATAPROVIDERSVC_H
#define IROBDATAPROVIDERSVC_H

#include "GaudiKernel/IInterface.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/EventContext.h"

#include <cstdint>
#include <vector>
#include <string_view>
#include <functional>


/** @class IROBDataProviderSvc
    @brief Interface class for managing ROB for both online and offline.
*/
class IROBDataProviderSvc : virtual public IInterface {

public:
   using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
   using VROBFRAG = std::vector<const ROBF*>;

   DeclareInterfaceID(IROBDataProviderSvc, 1, 1);

   /// Add all ROBFragments of a RawEvent to cache
   virtual void setNextEvent( const EventContext& context, const RawEvent* re) = 0;
   /// Retrieve ROBFragments for given ROB ids from cache
   virtual void getROBData(const EventContext& context, const std::vector<uint32_t>& robIds, VROBFRAG& robFragments, const std::string_view callerName="UNKNOWN") = 0;
   /// Retrieve the whole event.
   virtual const RawEvent* getEvent(const EventContext& context) = 0;
   /// Store the status for the event.
   virtual void setEventStatus(const EventContext& context, uint32_t ) = 0;
   /// Retrieve the status for the event.
   virtual uint32_t getEventStatus(const EventContext& context) = 0;
   
   /// @brief Interface to access cache of ROBs (it is a full event in case of offline)
   /// In online implementation the cache will contain only a subset of ROBs. 
   /// This method allows read access to the cache. 
   /// @warning in case the cache is updated in the meantime the iteration is guaranteed to be safe 
   /// but may not give access to all the ROBs available n the very moment
   /// Example of counting: size_t counter = 0; svc->processCachedROBs(ctx, [&](const ROBF*){ counter ++; })
   /// Example of printout: svc->processCachedROBs(ctx, [&](const ROBF* rob){ log() << MSG::DEBUG << "ROB " << rob->source_id() << endmsg; })
   virtual void processCachedROBs(const EventContext& context, 
                                  const std::function< void(const ROBF* )>& fn ) const = 0;
};

#endif
