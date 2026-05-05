/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file EventSelectorAthenaPoolSharedIO.cxx
 *  @brief This file contains the implementation for the EventSelectorAthenaPoolSharedIO class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "EventSelectorAthenaPoolSharedIO.h"
#include "EventContextAthenaPool.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "PersistentDataModel/Token.h"

// Framework
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/IDataShare.h"

// Pool
#include "CollectionSvc/ICollectionCursor.h"
#include "CollectionSvc/CollectionRowBuffer.h"

namespace {
   /// Helper to suppress thread-checker warnings for single-threaded execution
   StatusCode putEvent_ST(const IAthenaIPCTool& tool,
                          long eventNumber, const void* source,
                          size_t nbytes, unsigned int status) {
      StatusCode sc ATLAS_THREAD_SAFE = tool.putEvent(eventNumber, source, nbytes, status);
      return sc;
   }
}

//________________________________________________________________________________
EventSelectorAthenaPoolSharedIO::EventSelectorAthenaPoolSharedIO(const std::string& name, ISvcLocator* pSvcLocator) :
	base_class(name, pSvcLocator) {
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::initialize() {
   if (!EventSelectorAthenaPool::initialize().isSuccess()) {
      return StatusCode::FAILURE;
   }
   // Get SharedMemoryTool (if configured)
   if (!m_eventStreamingTool.empty() && !m_eventStreamingTool.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Cannot get " << m_eventStreamingTool.typeAndName() << "");
      return StatusCode::FAILURE;
   } else if (m_makeStreamingToolClient.value() == -1) {
      std::string dummyStr;
      ATH_CHECK(m_eventStreamingTool->makeClient(m_makeStreamingToolClient.value(), dummyStr));
   }
   // Don't listen to the Event Processing incidents
   if (!m_eventStreamingTool.empty()) {
      m_incidentSvc->removeListener(this, IncidentType::BeginProcessing);
      m_incidentSvc->removeListener(this, IncidentType::EndProcessing);
   }
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::start() {
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      return StatusCode::SUCCESS;
   }
   return EventSelectorAthenaPool::start();
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::stop() {
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      return StatusCode::SUCCESS;
   }
   return EventSelectorAthenaPool::stop();
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::finalize() {
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      delete m_endIter;   m_endIter   = nullptr;
      return ::AthService::finalize();
   }
   return EventSelectorAthenaPool::finalize();
}

//__________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::io_reinit() {
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      m_guid = Guid::null();
      m_evtCount = 0;
   }
   return EventSelectorAthenaPool::io_reinit();
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::next(IEvtSelector::Context& ctxt) const {
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      if (m_makeStreamingToolClient.value() == -1) {
         StatusCode sc = m_eventStreamingTool->lockEvent(m_evtCount);
         while (sc.isRecoverable()) {
            usleep(1000);
            sc = m_eventStreamingTool->lockEvent(m_evtCount);
         }
      }
      // Increase event count
      ++m_evtCount;
      void* tokenStrPtr = nullptr;
      unsigned int status = 0;
      StatusCode sc = m_eventStreamingTool->getLockedEvent(&tokenStrPtr, status);
      std::unique_ptr<const char[]> tokenStr{static_cast<const char*>(tokenStrPtr)};
      if (sc.isRecoverable()) {
         // Return end iterator
         ctxt = *m_endIter;
         // This is not a real failure but a Gaudi way of handling "end of job"
         return StatusCode::FAILURE;
      }
      if (sc.isFailure()) {
         ATH_MSG_FATAL("Cannot get NextEvent from AthenaSharedMemoryTool");
         return StatusCode::FAILURE;
      }
      if (!eventStore()->clearStore().isSuccess()) {
         ATH_MSG_WARNING("Cannot clear Store");
      }
      std::unique_ptr<AthenaAttributeList> athAttrList = std::make_unique<AthenaAttributeList>();
      athAttrList->extend("eventRef", "string");
      (*athAttrList)["eventRef"].data<std::string>() = tokenStr.get();
      SG::WriteHandle<AthenaAttributeList> wh(m_attrListKey, eventStore()->name());
      if (!wh.record(std::move(athAttrList)).isSuccess()) {
         ATH_MSG_ERROR("Cannot record AttributeList to StoreGate " << StoreID::storeName(eventStore()->storeID()));
         return StatusCode::FAILURE;
      }
      Token token;
      token.fromString(tokenStr.get());
      Guid guid = token.dbID();
      if (guid != m_guid && m_processMetadata.value()) {
         InputFileIncidentGuard::transition(m_inputFileGuard, *m_incidentSvc, name(),
                                          "FID:" + guid.toString(), guid.toString(),
                                          /*endFileName=*/{});
         m_guid = guid;
      }
      return StatusCode::SUCCESS;
   }
   return EventSelectorAthenaPool::next(ctxt);
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::next(IEvtSelector::Context& ctxt, int jump) const {
   if (jump > 0) {
      for (int i = 0; i < jump; i++) {
         ATH_CHECK(next(ctxt));
      }
      return StatusCode::SUCCESS;
   }
   return StatusCode::FAILURE;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::makeServer(int num) {
   IDataShare* ds = dynamic_cast<IDataShare*>(m_athenaPoolCnvSvc.get());
   if (ds == nullptr) {
      ATH_MSG_ERROR("Cannot cast AthenaPoolCnvSvc to DataShare");
      return StatusCode::FAILURE;
   }
   if (num < 0) {
      if (ds->makeServer(num - 1).isFailure()) {
         ATH_MSG_ERROR("Failed to switch AthenaPoolCnvSvc to output DataStreaming server");
      }
      return StatusCode::SUCCESS;
   }
   if (ds->makeServer(num + 1).isFailure()) {
      ATH_MSG_ERROR("Failed to switch AthenaPoolCnvSvc to input DataStreaming server");
      return StatusCode::FAILURE;
   }
   if (m_eventStreamingTool.empty()) {
      return StatusCode::SUCCESS;
   }
   m_processMetadata = false;
   ATH_MSG_DEBUG("makeServer: " << m_eventStreamingTool << " = " << num);
   return(m_eventStreamingTool->makeServer(1, ""));
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::makeClient(int num) {
   IDataShare* ds = dynamic_cast<IDataShare*>(m_athenaPoolCnvSvc.get());
   if (ds == nullptr) {
      ATH_MSG_ERROR("Cannot cast AthenaPoolCnvSvc to DataShare");
      return StatusCode::FAILURE;
   }
   if (ds->makeClient(num + 1).isFailure()) {
      ATH_MSG_ERROR("Failed to switch AthenaPoolCnvSvc to DataStreaming client");
      return StatusCode::FAILURE;
   }
   if (m_eventStreamingTool.empty()) {
      return StatusCode::SUCCESS;
   }
   ATH_MSG_DEBUG("makeClient: " << m_eventStreamingTool << " = " << num);
   std::string dummyStr;
   return(m_eventStreamingTool->makeClient(0, dummyStr));
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::share(int evtnum) {
   IDataShare* ds = dynamic_cast<IDataShare*>(m_athenaPoolCnvSvc.get());
   if (ds == nullptr) {
      ATH_MSG_ERROR("Cannot cast AthenaPoolCnvSvc to DataShare");
      return StatusCode::FAILURE;
   }
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      StatusCode sc = m_eventStreamingTool->lockEvent(evtnum);
      while (sc.isRecoverable()) {
         usleep(1000);
         sc = m_eventStreamingTool->lockEvent(evtnum);
      }
// Send stop client and wait for restart
      if (sc.isFailure()) {
         if (ds->makeClient(0).isFailure()) {
            return StatusCode::FAILURE;
         }
         sc = m_eventStreamingTool->lockEvent(evtnum);
         while (sc.isRecoverable() || sc.isFailure()) {
            usleep(1000);
            sc = m_eventStreamingTool->lockEvent(evtnum);
         }
//FIXME
         if (ds->makeClient(1).isFailure()) {
            return StatusCode::FAILURE;
         }
      }
      return(sc);
   }
   return StatusCode::FAILURE;
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::readEvent(int maxevt) {
   IDataShare* ds = dynamic_cast<IDataShare*>(m_athenaPoolCnvSvc.get());
   if (ds == nullptr) {
      ATH_MSG_ERROR("Cannot cast AthenaPoolCnvSvc to DataShare");
      return StatusCode::FAILURE;
   }
   if (m_eventStreamingTool.empty()) {
      ATH_MSG_ERROR("No AthenaSharedMemoryTool configured for readEvent()");
      return StatusCode::FAILURE;
   }
   ATH_MSG_VERBOSE("Called read Event " << maxevt);
   std::unique_ptr<IEvtSelector::Context> ctxt = std::make_unique<EventContextAthenaPool>(this);
   for (int i = 0; i < maxevt || maxevt == -1; ++i) {
      if (!next(*ctxt).isSuccess()) {
         if (m_evtCount == -1) {
            ATH_MSG_VERBOSE("Called read Event and read last event from input: " << i);
            break;
         }
         ATH_MSG_ERROR("Cannot read Event " << m_evtCount - 1 << " into AthenaSharedMemoryTool");
         return StatusCode::FAILURE;
      } else {
         ATH_MSG_VERBOSE("Called next, read Event " << m_evtCount - 1);
      }
   }
   ctxt.reset();
   // End of file, wait for last event to be taken
   StatusCode sc;
   while ( (sc = putEvent_ST(*m_eventStreamingTool, 0, 0, 0, 0)).isRecoverable() ) {
      while (ds->readData().isSuccess()) {
         ATH_MSG_VERBOSE("Called last readData, while marking last event in readEvent()");
      }
      usleep(1000);
   }
   if (!sc.isSuccess()) {
      ATH_MSG_ERROR("Cannot put last Event marker to AthenaSharedMemoryTool");
      return StatusCode::FAILURE;
   } else {
      sc = ds->readData();
      while (sc.isSuccess() || sc.isRecoverable()) {
         sc = ds->readData();
      }
      ATH_MSG_DEBUG("Failed last readData -> Clients are stopped, after marking last event in readEvent()");
   }
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode EventSelectorAthenaPoolSharedIO::recordAttributeList() const {
   if (!m_eventStreamingTool.empty()) {
      if (m_eventStreamingTool->isServer()) {
         IDataShare* ds = dynamic_cast<IDataShare*>(m_athenaPoolCnvSvc.get());
         if (ds == nullptr) {
            ATH_MSG_ERROR("Cannot cast AthenaPoolCnvSvc to DataShare");
            return StatusCode::FAILURE;
         }
         std::string token = m_headerIterator->eventRef().toString();
         StatusCode sc;
         while ( (sc = putEvent_ST(*m_eventStreamingTool,
                                   m_evtCount - 1, token.c_str(),
                                   token.length() + 1, 0)).isRecoverable() ) {
            while (ds->readData().isSuccess()) {
               ATH_MSG_VERBOSE("Called last readData, while putting next event in next()");
            }
            // Nothing to do right now, trigger alternative (e.g. caching) here? Currently just fast loop.
         }
         if (!sc.isSuccess()) {
            ATH_MSG_ERROR("Cannot put Event " << m_evtCount - 1 << " to AthenaSharedMemoryTool");
            return StatusCode::FAILURE;
         }
      } else {
         return StatusCode::SUCCESS;
      }
   } else {
      return EventSelectorAthenaPool::recordAttributeList();
   }
   return StatusCode::SUCCESS;
}
