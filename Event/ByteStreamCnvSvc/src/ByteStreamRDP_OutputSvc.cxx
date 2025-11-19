/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//===================================================================
//  Implementation of ByteStreamRDP_OutputSvc
//  Initial implementation Jan 10, 2003
//             
//===================================================================
//

// Include files.
#include "ByteStreamRDP_OutputSvc.h"
#include "ByteStreamCnvSvcBase/ROBDataProviderSvc.h"

// Constructor.
ByteStreamRDP_OutputSvc::ByteStreamRDP_OutputSvc(const std::string& name, ISvcLocator* svcloc)
	: base_class(name, svcloc) {
      if (m_bsOutputStreamName.empty()) m_bsOutputStreamName = name;
}

// Open the first input file and read the first event.
StatusCode ByteStreamRDP_OutputSvc::initialize() {
   ATH_MSG_INFO("Initializing");

   // Retrieve ROBDataProviderSvc
   ATH_CHECK( m_robProvider.retrieve() );

   return StatusCode::SUCCESS;
}

// Receive the next event without explicit context
bool ByteStreamRDP_OutputSvc::putEvent(const RawEvent* re) {
   return putEvent(re, Gaudi::Hive::currentContext());
}

// Receive the next event
bool ByteStreamRDP_OutputSvc::putEvent(const RawEvent* re, const EventContext& ctx) {
   EventCache* cache = m_eventsCache.get(ctx);
   cache->releaseEvent();
   const uint32_t reSize = re->fragment_size_word();
   const uint32_t* reStart = re->start();
   cache->dataBuffer = std::make_unique<uint32_t[]>(reSize);
   std::copy(reStart, reStart+reSize, cache->dataBuffer.get());

   // Create a cached RawEvent object from the cached data buffer
   cache->rawEvent = std::make_unique<RawEvent>(cache->dataBuffer.get());

   // Give the RawEvent to ROBDataProvider
   m_robProvider->setNextEvent(ctx, cache->rawEvent.get());

   return true;
}
