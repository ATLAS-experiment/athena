/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file EventSelectorAthenaPool.cxx
 *  @brief This file contains the implementation for the EventSelectorAthenaPool class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "EventSelectorAthenaPool.h"
#include "EventContextAthenaPool.h"
#include "PoolCollectionConverter.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PersistentDataModel/DataHeader.h"
#include "PoolSvc/IPoolSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// Framework
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/IDataShare.h"

// Pool
#include "CollectionSvc/ICollectionCursor.h"
#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/TokenList.h"
#include "StorageSvc/DbType.h"

#include <boost/tokenizer.hpp>
#include <algorithm>
#include <format>
#include <vector>


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
EventSelectorAthenaPool::EventSelectorAthenaPool(const std::string& name, ISvcLocator* pSvcLocator) :
	base_class(name, pSvcLocator)
{

   // TODO: validate if those are even used
   m_runNo.verifier().setLower(0);
   m_oldRunNo.verifier().setLower(0);
   m_eventsPerRun.verifier().setLower(0);
   m_firstEventNo.verifier().setLower(1);
   m_firstLBNo.verifier().setLower(0);
   m_eventsPerLB.verifier().setLower(0);
   m_initTimeStamp.verifier().setLower(0);

   m_inputCollectionsProp.declareUpdateHandler(&EventSelectorAthenaPool::inputCollectionsHandler, this);
   m_inputCollectionsChanged = false;
}
//________________________________________________________________________________
void EventSelectorAthenaPool::inputCollectionsHandler(Gaudi::Details::PropertyBase&) {
   if (this->FSMState() != Gaudi::StateMachine::OFFLINE) {
      m_inputCollectionsChanged = true;
   }
}
//________________________________________________________________________________
EventSelectorAthenaPool::~EventSelectorAthenaPool() {
}
//________________________________________________________________________________
StoreGateSvc* EventSelectorAthenaPool::eventStore() const {
   return StoreGateSvc::currentStoreGate();
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::initialize() {

   m_autoRetrieveTools = false;
   m_checkToolDeps = false;
  
   if (m_isSecondary.value()) {
      ATH_MSG_DEBUG("Initializing secondary event selector " << name());
   } else {
      ATH_MSG_DEBUG("Initializing " << name());
   }

   ATH_CHECK(::AthService::initialize());
   // Check for input collection
   if (m_inputCollectionsProp.value().empty()) {
      ATH_MSG_FATAL("Use the property: EventSelector.InputCollections = "
		      << "[ \"<collectionName>\" ] (list of collections)");
      return StatusCode::FAILURE;
   }
   boost::char_separator<char> sep_coma(","), sep_hyph("-");
   boost::tokenizer  ranges(m_skipEventRangesProp.value(), sep_coma);
   for( const std::string& r: ranges ) {
      boost::tokenizer  fromto(r, sep_hyph);
      auto from_iter = fromto.begin();
      long from = std::stol(*from_iter);
      long to = from;
      if( ++from_iter != fromto.end() ) {
         to = std::stol(*from_iter);
      }
      m_skipEventRanges.emplace_back(from, to);
   }

   for( auto v : m_skipEventSequenceProp.value() ) {
      m_skipEventRanges.emplace_back(v, v);
   }
   std::sort(m_skipEventRanges.begin(), m_skipEventRanges.end());
   if( msgLvl(MSG::DEBUG) ) {
      std::string skip_ranges_str;
      for( const auto& [first, second] : m_skipEventRanges ) {
         if( !skip_ranges_str.empty() ) skip_ranges_str += ", ";
         skip_ranges_str += std::to_string(first);
         if( first != second) skip_ranges_str += std::format("-{}", second);
      }
      if( !skip_ranges_str.empty() )
         ATH_MSG_DEBUG("Events to skip: " << skip_ranges_str);
   }
   // CollectionType must be one of:
   if (m_collectionType.value() != "RootCollection" && m_collectionType.value() != "ImplicitCollection") {
      ATH_MSG_FATAL("EventSelector.CollectionType must be one of: RootCollection, ImplicitCollection (default)");
      return StatusCode::FAILURE;
   }
   // Get IncidentSvc
   ATH_CHECK(m_incidentSvc.retrieve());
   // Listen to the Event Processing incidents
   if (m_eventStreamingTool.empty()) {
      m_incidentSvc->addListener(this, IncidentType::BeginProcessing, 0);
      m_incidentSvc->addListener(this, IncidentType::EndProcessing, 0);
   }

   // Get AthenaPoolCnvSvc
   ATH_CHECK(m_athenaPoolCnvSvc.retrieve());
   // Get CounterTool (if configured)
   if (!m_counterTool.empty()) {
      ATH_CHECK(m_counterTool.retrieve());
   }
   // Get HelperTools
   ATH_CHECK(m_helperTools.retrieve());
   // Get SharedMemoryTool (if configured)
   if (!m_eventStreamingTool.empty() && !m_eventStreamingTool.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Cannot get " << m_eventStreamingTool.typeAndName() << "");
      return StatusCode::FAILURE;
   } else if (m_makeStreamingToolClient.value() == -1) {
      std::string dummyStr;
      ATH_CHECK(m_eventStreamingTool->makeClient(m_makeStreamingToolClient.value(), dummyStr));
   }

   // Ensure the xAODCnvSvc is listed in the EventPersistencySvc
   ServiceHandle<IProperty> epSvc("EventPersistencySvc", name());
   std::vector<std::string> propVal;
   ATH_CHECK(Gaudi::Parsers::parse(propVal , epSvc->getProperty("CnvServices").toString()));
   bool foundCnvSvc = false;
   for (const auto& property : propVal) {
      if (property == m_athenaPoolCnvSvc.type()) { foundCnvSvc = true; }
   }
   if (!foundCnvSvc) {
      propVal.push_back(m_athenaPoolCnvSvc.type());
      if (!epSvc->setProperty("CnvServices", Gaudi::Utils::toString(propVal)).isSuccess()) {
         ATH_MSG_FATAL("Cannot set EventPersistencySvc Property for CnvServices");
         return StatusCode::FAILURE;
      }
   }

   // Register this service for 'I/O' events
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   ATH_CHECK(iomgr.retrieve());
   ATH_CHECK(iomgr->io_register(this));
   // Register input file's names with the I/O manager
   const std::vector<std::string>& incol = m_inputCollectionsProp.value();
   bool allGood = true;
   std::string fileName;
   std::string fileType;
   for (const auto& inputCollection : incol) {
      if (inputCollection.starts_with("LFN:") || inputCollection.starts_with("FID:")) {
         m_athenaPoolCnvSvc->getPoolSvc()->lookupBestPfn(inputCollection, fileName, fileType);
      } else {
         fileName = inputCollection;
      }
      if (fileName.starts_with("PFN:")) {
         fileName = fileName.substr(4);
      }
      if (!iomgr->io_register(this, IIoComponentMgr::IoMode::READ, inputCollection, fileName).isSuccess()) {
         ATH_MSG_FATAL("could not register [" << inputCollection << "] for output !");
         allGood = false;
      } else {
         ATH_MSG_VERBOSE("io_register[" << this->name() << "](" << inputCollection << ") [ok]");
      }
   }
   if (!allGood) {
      return StatusCode::FAILURE;
   }

   // Connect to PersistencySvc
   if (!m_athenaPoolCnvSvc->getPoolSvc()->connect(pool::ITransaction::READ, IPoolSvc::kInputStream).isSuccess()) {
      ATH_MSG_FATAL("Cannot connect to POOL PersistencySvc.");
      return StatusCode::FAILURE;
   }
   // Jump to reinit() to execute common init/reinit actions
   m_guid = Guid::null();
   return reinit();
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::reinit() const {
   ATH_MSG_DEBUG("reinitialization...");

   // reset markers
   m_numEvt.resize(m_inputCollectionsProp.value().size(), -1);
   m_firstEvt.resize(m_inputCollectionsProp.value().size(), -1);

   // Initialize InputCollectionsIterator
   m_inputCollectionsIterator = m_inputCollectionsProp.value().begin();
   m_curCollection = 0;
   if (!m_firstEvt.empty()) {
      m_firstEvt[0] = 0;
   }
   m_inputCollectionsChanged = false;
   m_evtCount = 0;
   m_headerIterator = 0;
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      ATH_MSG_INFO("Done reinitialization for shared reader client");
      return StatusCode::SUCCESS;
   }
   bool retError = false;
   for (auto& tool : m_helperTools) {
      if (!tool->postInitialize().isSuccess()) {
         ATH_MSG_FATAL("Failed to postInitialize() " << tool->name());
         retError = true;
      }
   }
   if (retError) {
      ATH_MSG_FATAL("Failed to postInitialize() helperTools");
      return StatusCode::FAILURE;
   }

   // Create an m_poolCollectionConverter to read the objects in
   m_poolCollectionConverter = getCollectionCnv();
   if (!m_poolCollectionConverter) {
      ATH_MSG_INFO("No Events found in any Input Collections");
      if (m_processMetadata.value()) {
	 m_inputCollectionsIterator = m_inputCollectionsProp.value().end();
	 if (!m_inputCollectionsProp.value().empty()) --m_inputCollectionsIterator;
	//NOTE (wb may 2016): this will make the FirstInputFile incident correspond to last file in the collection ... if want it to be first file then move iterator to begin and then move above two lines below this incident firing
         if (m_collectionType.value() == "ImplicitCollection" && !m_firedIncident && !m_inputCollectionsProp.value().empty()) {
            FileIncident firstInputFileIncident(name(), "FirstInputFile", *m_inputCollectionsIterator);
            m_incidentSvc->fireIncident(firstInputFileIncident);
            m_firedIncident = true;
         }
      }
      return StatusCode::SUCCESS;
   }
   // Get DataHeader iterator
   try {
      m_headerIterator = &m_poolCollectionConverter->selectAll();
   } catch (std::exception &e) {
      ATH_MSG_FATAL("Cannot open implicit collection - check data/software version.");
      ATH_MSG_ERROR(e.what());
      return StatusCode::FAILURE;
   }
   while (m_headerIterator == nullptr || m_headerIterator->next() == 0) { // no selected events
      if (m_poolCollectionConverter) {
         m_poolCollectionConverter->disconnectDb().ignore();
         m_poolCollectionConverter.reset();
      }
      ++m_inputCollectionsIterator;
      m_poolCollectionConverter = getCollectionCnv();
      if (m_poolCollectionConverter) {
         m_headerIterator = &m_poolCollectionConverter->selectAll();
      } else {
         break;
      }
   }
   if (!m_poolCollectionConverter || m_headerIterator == nullptr) { // no event selected in any collection
      m_inputCollectionsIterator = m_inputCollectionsProp.value().begin();
      m_curCollection = 0;
      m_poolCollectionConverter = getCollectionCnv();
      if (!m_poolCollectionConverter) {
         return StatusCode::SUCCESS;
      }
      m_headerIterator = &m_poolCollectionConverter->selectAll();
      while (m_headerIterator == nullptr || m_headerIterator->next() == 0) { // empty collection
         if (m_poolCollectionConverter) {
            m_poolCollectionConverter->disconnectDb().ignore();
            m_poolCollectionConverter.reset();
         }
         ++m_inputCollectionsIterator;
         m_poolCollectionConverter = getCollectionCnv();
         if (m_poolCollectionConverter) {
            m_headerIterator = &m_poolCollectionConverter->selectAll();
         } else {
            break;
         }
      }
   }
   if (!m_poolCollectionConverter || m_headerIterator == nullptr) {
      return StatusCode::SUCCESS;
   }
   const Token& headRef = m_headerIterator->eventRef();
   const std::string fid = headRef.dbID().toString();
   const int tech = headRef.technology();
   ATH_MSG_VERBOSE("reinit(): First DataHeder Token=" << headRef.toString() );

   // Check if File is BS, for which Incident is thrown by SingleEventInputSvc
   if (tech != 0x00001000 && m_processMetadata.value() && !m_firedIncident) {
      FileIncident firstInputFileIncident(name(), "FirstInputFile", "FID:" + fid, fid);
      m_incidentSvc->fireIncident(firstInputFileIncident);
      m_firedIncident = true;
   }
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::start() {
   if (m_poolCollectionConverter) {
      // Reset iterators and apply new query
      m_poolCollectionConverter->disconnectDb().ignore();
      m_poolCollectionConverter.reset();
   }
   m_inputCollectionsIterator = m_inputCollectionsProp.value().begin();
   m_curCollection = 0;
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      return StatusCode::SUCCESS;
   }
   m_poolCollectionConverter = getCollectionCnv(true);
   if (!m_poolCollectionConverter) {
      ATH_MSG_INFO("No Events found in any Input Collections");
      m_inputCollectionsIterator = m_inputCollectionsProp.value().end();
      if (!m_inputCollectionsProp.value().empty()) {
         --m_inputCollectionsIterator; //leave iterator in state of last input file
      }
   } else {
      m_headerIterator = &m_poolCollectionConverter->selectAll();
   }
   m_evtCount = 0;
   delete m_endIter;
   m_endIter = nullptr;
   m_endIter = new EventContextAthenaPool(nullptr);
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::stop() {
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      return StatusCode::SUCCESS;
   }
   IEvtSelector::Context* ctxt(nullptr);
   if (!releaseContext(ctxt).isSuccess()) {
      ATH_MSG_WARNING("Cannot release context");
   }
   return StatusCode::SUCCESS;
}

//________________________________________________________________________________
void EventSelectorAthenaPool::fireEndFileIncidents(bool isLastFile) const {
   if (m_processMetadata.value()) {
      if (m_evtCount >= 0) {
         // Assume that the end of collection file indicates the end of payload file.
         if (m_guid != Guid::null()) {
            // Fire EndInputFile incident
            FileIncident endInputFileIncident(name(), "EndInputFile", "FID:" + m_guid.toString(), m_guid.toString());
            m_incidentSvc->fireIncident(endInputFileIncident);
         }
      }
      if (isLastFile && m_firedIncident) {
         m_firedIncident = false;
      }
   }
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::finalize() {
   if (m_eventStreamingTool.empty() || !m_eventStreamingTool->isClient()) {
      if (!m_counterTool.empty() && !m_counterTool->preFinalize().isSuccess()) {
         ATH_MSG_WARNING("Failed to preFinalize() CounterTool");
      }
      for (auto& tool : m_helperTools) {
         if (!tool->preFinalize().isSuccess()) {
            ATH_MSG_WARNING("Failed to preFinalize() " << tool->name());
         }
      }
   }
   delete m_endIter;   m_endIter   = nullptr;
   m_headerIterator = nullptr;
   if (m_poolCollectionConverter) {
     m_poolCollectionConverter.reset();
   }
   // Finalize the Service base class.
   return ::AthService::finalize();
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::createContext(IEvtSelector::Context*& ctxt) const {
   ctxt = new EventContextAthenaPool(this);
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::next(IEvtSelector::Context& ctxt) const {
   std::lock_guard<CallMutex> lockGuard(m_callLock);
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
      void* tokenStr = nullptr;
      unsigned int status = 0;
      StatusCode sc = m_eventStreamingTool->getLockedEvent(&tokenStr, status);
      if (sc.isRecoverable()) {
         delete [] (char*)tokenStr; tokenStr = nullptr;
         // Return end iterator
         ctxt = *m_endIter;
         // This is not a real failure but a Gaudi way of handling "end of job"
         return StatusCode::FAILURE;
      }
      if (sc.isFailure()) {
         ATH_MSG_FATAL("Cannot get NextEvent from AthenaSharedMemoryTool");
         delete [] (char*)tokenStr; tokenStr = nullptr;
         return StatusCode::FAILURE;
      }
      if (!eventStore()->clearStore().isSuccess()) {
         ATH_MSG_WARNING("Cannot clear Store");
      }
      std::unique_ptr<AthenaAttributeList> athAttrList(new AthenaAttributeList());
      athAttrList->extend("eventRef", "string");
      (*athAttrList)["eventRef"].data<std::string>() = std::string((char*)tokenStr);
      SG::WriteHandle<AthenaAttributeList> wh(m_attrListKey.value(), eventStore()->name());
      if (!wh.record(std::move(athAttrList)).isSuccess()) {
         delete [] (char*)tokenStr; tokenStr = nullptr;
         ATH_MSG_ERROR("Cannot record AttributeList to StoreGate " << StoreID::storeName(eventStore()->storeID()));
         return StatusCode::FAILURE;
      }
      Token token;
      token.fromString(std::string((char*)tokenStr));
      delete [] (char*)tokenStr; tokenStr = nullptr;
      Guid guid = token.dbID();
      if (guid != m_guid && m_processMetadata.value()) {
         if (m_evtCount >= 0 && m_guid != Guid::null()) {
            // Fire EndInputFile incident
            FileIncident endInputFileIncident(name(), "EndInputFile", "FID:" + m_guid.toString(), m_guid.toString());
            m_incidentSvc->fireIncident(endInputFileIncident);
         }
         m_guid = guid;
         FileIncident beginInputFileIncident(name(), "BeginInputFile", "FID:" + m_guid.toString(), m_guid.toString());
         m_incidentSvc->fireIncident(beginInputFileIncident);
      }
      return StatusCode::SUCCESS;
   }
   for (const auto& tool : m_helperTools) {
      if (!tool->preNext().isSuccess()) {
         ATH_MSG_WARNING("Failed to preNext() " << tool->name());
      }
   }
   for (;;) {
      // Handle possible file transition
      StatusCode sc = nextHandleFileTransition(ctxt);
      if (sc.isRecoverable()) {
        continue; // handles empty files
      }
      if (sc.isFailure()) {
         return StatusCode::FAILURE;
      }
      // Increase event count
      ++m_evtCount;
      if (!m_counterTool.empty() && !m_counterTool->preNext().isSuccess()) {
         ATH_MSG_WARNING("Failed to preNext() CounterTool.");
      }
      if( m_evtCount > m_skipEvents
          && (m_skipEventRanges.empty() || m_evtCount < m_skipEventRanges.front().first))
      {
         if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isServer()) {
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
            if (!m_isSecondary.value()) {
               if (!eventStore()->clearStore().isSuccess()) {
                  ATH_MSG_WARNING("Cannot clear Store");
               }
               if (!recordAttributeList().isSuccess()) {
                  ATH_MSG_ERROR("Failed to record AttributeList.");
                  return StatusCode::FAILURE;
               }
            }
         }
         StatusCode status = StatusCode::SUCCESS;
         for (const auto& tool : m_helperTools) {
            StatusCode toolStatus = tool->postNext();
            if (toolStatus.isRecoverable()) {
               ATH_MSG_INFO("Request skipping event from: " << tool->name());
               if (status.isSuccess()) {
                  status = StatusCode::RECOVERABLE;
               }
            } else if (toolStatus.isFailure()) {
               ATH_MSG_WARNING("Failed to postNext() " << tool->name());
               status = StatusCode::FAILURE;
            }
         }
         if (status.isRecoverable()) {
            ATH_MSG_INFO("skipping event " << m_evtCount);
         } else if (status.isFailure()) {
            ATH_MSG_WARNING("Failed to postNext() HelperTool.");
         } else {
            if (!m_counterTool.empty() && !m_counterTool->postNext().isSuccess()) {
               ATH_MSG_WARNING("Failed to postNext() CounterTool.");
            }
            break;
         }
      } else {
         while( !m_skipEventRanges.empty() && m_evtCount >= m_skipEventRanges.front().second ) {
            m_skipEventRanges.erase(m_skipEventRanges.begin());
         }
         ATH_MSG_INFO("skipping event " << m_evtCount);
      }
   }
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::next(IEvtSelector::Context& ctxt, int jump) const {
   if (jump > 0) {
      for (int i = 0; i < jump; i++) {
         ATH_CHECK(next(ctxt));
      }
      return StatusCode::SUCCESS;
   }
   return StatusCode::FAILURE;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::nextHandleFileTransition(IEvtSelector::Context& ctxt) const
{
   if( m_inputCollectionsChanged ) {
      StatusCode rc = reinit();
      if( rc != StatusCode::SUCCESS ) return rc;
   }
   else {   // advance to the next (not needed after reinit)
      // Check if we're at the end of file
      if (m_headerIterator == nullptr || m_headerIterator->next() == 0) {
         m_headerIterator = nullptr;
         // Close previous collection.
         m_poolCollectionConverter.reset();

         // zero the current DB ID (m_guid) before disconnect() to indicate it is no longer in use
         const SG::SourceID old_guid = m_guid.toString();
         m_guid = Guid::null();
         disconnectIfFinished( old_guid );

         // check if somebody updated Inputs in the EOF incident (like VP1 does)
         if( m_inputCollectionsChanged ) {
            StatusCode rc = reinit();
            if( rc != StatusCode::SUCCESS ) return rc;
         } else {
            // Open next file from inputCollections list.
            ++m_inputCollectionsIterator;
            // Create PoolCollectionConverter for input file
            m_poolCollectionConverter = getCollectionCnv(true);
            if (!m_poolCollectionConverter) {
               // Return end iterator
               ctxt = *m_endIter;
               // This is not a real failure but a Gaudi way of handling "end of job"
               return StatusCode::FAILURE;
            }
            // Get DataHeader iterator
            m_headerIterator = &m_poolCollectionConverter->selectAll();

            // Return RECOVERABLE to mark we should still continue
            return StatusCode::RECOVERABLE;
         }
      }
   }
   const Token& headRef = m_headerIterator->eventRef();
   const Guid guid = headRef.dbID();
   const int tech = headRef.technology();
   ATH_MSG_VERBOSE("next(): DataHeder Token=" << headRef.toString() );

   if (guid != m_guid) {
      // we are starting reading from a new DB. Check if the old one needs to be retired
      if (m_guid != Guid::null()) {
         // zero the current DB ID (m_guid) before trying disconnect() to indicate it is no longer in use
         const SG::SourceID old_guid = m_guid.toString();
         m_guid = Guid::null();
         disconnectIfFinished( old_guid );
      }
      m_guid = guid;
      m_activeEventsPerSource[guid.toString()] = 0;
      // Fire BeginInputFile incident if current InputCollection is a payload file;
      // otherwise, ascertain whether the pointed-to file is reachable before firing any incidents and/or proceeding
      if (m_collectionType.value() == "ImplicitCollection") {
         // For now, we can only deal with input metadata from POOL files, but we know we have a POOL file here
         if (!m_athenaPoolCnvSvc->setInputAttributes(*m_inputCollectionsIterator).isSuccess()) {
               ATH_MSG_ERROR("Failed to set input attributes.");
               return StatusCode::FAILURE;
         }
         if (m_processMetadata.value()) {
            FileIncident beginInputFileIncident(name(), "BeginInputFile", *m_inputCollectionsIterator, m_guid.toString());
            m_incidentSvc->fireIncident(beginInputFileIncident);
         }
      } else {
         // Check if File is BS
         if (tech != 0x00001000 && m_processMetadata.value()) {
            FileIncident beginInputFileIncident(name(), "BeginInputFile", "FID:" + m_guid.toString(), m_guid.toString());
            m_incidentSvc->fireIncident(beginInputFileIncident);
         }
      }
   }  // end if (guid != m_guid)
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::nextWithSkip(IEvtSelector::Context& ctxt) const {
   ATH_MSG_DEBUG("EventSelectorAthenaPool::nextWithSkip");

   for (;;) {
      // Check if we're at the end of file
      StatusCode sc = nextHandleFileTransition(ctxt);
      if (sc.isRecoverable()) {
         continue; // handles empty files
      }
      if (sc.isFailure()) {
         return StatusCode::FAILURE;
      }

      // Increase event count
      ++m_evtCount;

      if (!m_counterTool.empty() && !m_counterTool->preNext().isSuccess()) {
         ATH_MSG_WARNING("Failed to preNext() CounterTool.");
      }
      if( m_evtCount > m_skipEvents
         && (m_skipEventRanges.empty() || m_evtCount < m_skipEventRanges.front().first))
      {
         return StatusCode::SUCCESS;
      } else {
         while( !m_skipEventRanges.empty() && m_evtCount >= m_skipEventRanges.front().second ) {
            m_skipEventRanges.erase(m_skipEventRanges.begin());
         }
         if (m_isSecondary.value()) {
            ATH_MSG_INFO("skipping secondary event " << m_evtCount);
         } else {
            ATH_MSG_INFO("skipping event " << m_evtCount);
         }
      }
   }

   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::previous(IEvtSelector::Context& /*ctxt*/) const {
   ATH_MSG_ERROR("previous() not implemented");
   return StatusCode::FAILURE;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::previous(IEvtSelector::Context& ctxt, int jump) const {
   if (jump > 0) {
      for (int i = 0; i < jump; i++) {
         ATH_CHECK(previous(ctxt));
      }
      return StatusCode::SUCCESS;
   }
   return StatusCode::FAILURE;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::last(IEvtSelector::Context& ctxt) const {
   if (ctxt.identifier() == m_endIter->identifier()) {
      ATH_MSG_DEBUG("last(): Last event in InputStream.");
      return StatusCode::SUCCESS;
   }
   return StatusCode::FAILURE;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::rewind(IEvtSelector::Context& ctxt) const {
   ATH_CHECK(reinit());
   ctxt = EventContextAthenaPool(this);
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::createAddress(const IEvtSelector::Context& /*ctxt*/,
		IOpaqueAddress*& iop) const {
   std::string tokenStr;
   SG::ReadHandle<AthenaAttributeList> attrList(m_attrListKey.value(), eventStore()->name());
   if (attrList.isValid()) {
      try {
         tokenStr = (*attrList)["eventRef"].data<std::string>();
         ATH_MSG_DEBUG("found AthenaAttribute, name = eventRef = " << tokenStr);
      } catch (std::exception &e) {
         ATH_MSG_ERROR(e.what());
         return StatusCode::FAILURE;
      }
   } else {
      ATH_MSG_WARNING("Cannot find AthenaAttribute, key = " << m_attrListKey.value());
      tokenStr = m_headerIterator->eventRef().toString();
   }
   auto token = std::make_unique<Token>();
   token->fromString(tokenStr);
   iop = new TokenAddress(pool::POOL_StorageType.type(), ClassID_traits<DataHeader>::ID(), "", "EventSelector", IPoolSvc::kInputStream, std::move(token));
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::releaseContext(IEvtSelector::Context*& /*ctxt*/) const {
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::resetCriteria(const std::string& /*criteria*/,
		IEvtSelector::Context& /*ctxt*/) const {
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode EventSelectorAthenaPool::seek(Context& /*ctxt*/, int evtNum) const {

   if( m_inputCollectionsChanged ) {
      StatusCode rc = reinit();
      if( rc != StatusCode::SUCCESS ) return rc;
   }

   long newColl = findEvent(evtNum);
   if (newColl == -1 && evtNum >= m_firstEvt[m_curCollection] && evtNum < m_evtCount - 1) {
      newColl = m_curCollection;
   }
   if (newColl == -1) {
      m_headerIterator = nullptr;
      ATH_MSG_INFO("seek: Reached end of Input.");
      fireEndFileIncidents(true);
      return StatusCode::RECOVERABLE;
   }
   if (newColl != m_curCollection) {
      if (!m_keepInputFilesOpen.value() && m_poolCollectionConverter) {
         m_poolCollectionConverter->disconnectDb().ignore();
      }
      m_poolCollectionConverter.reset();
      m_curCollection = newColl;
      try {
         ATH_MSG_DEBUG("Seek to item: \""
	         <<  m_inputCollectionsProp.value()[m_curCollection]
	         << "\" from the collection list.");
         // Reset input collection iterator to the right place
         m_inputCollectionsIterator = m_inputCollectionsProp.value().begin();
         m_inputCollectionsIterator += m_curCollection;
         m_poolCollectionConverter = std::make_unique<PoolCollectionConverter>(m_collectionType.value() + ":" + m_collectionTree.value(),
	         m_inputCollectionsProp.value()[m_curCollection],
	         IPoolSvc::kInputStream,
	         m_athenaPoolCnvSvc->getPoolSvc());
         if (!m_poolCollectionConverter->initialize().isSuccess()) {
            m_headerIterator = nullptr;
            ATH_MSG_ERROR("seek: Unable to initialize PoolCollectionConverter.");
            return StatusCode::FAILURE;
         }
         // Create DataHeader iterators
         m_headerIterator = &m_poolCollectionConverter->selectAll();
         EventContextAthenaPool* beginIter = new EventContextAthenaPool(this);
         m_evtCount = m_firstEvt[m_curCollection];
         next(*beginIter).ignore();
         ATH_MSG_DEBUG("Token " << m_headerIterator->eventRef().toString());
      } catch (std::exception &e) {
         m_headerIterator = nullptr;
         ATH_MSG_ERROR(e.what());
         return StatusCode::FAILURE;
      }
   }

   if (m_headerIterator->seek(evtNum - m_firstEvt[m_curCollection]) == 0) {
      m_headerIterator = nullptr;
      ATH_MSG_ERROR("Did not find event, evtNum = " << evtNum);
      return StatusCode::FAILURE;
   } else {
      m_evtCount = evtNum + 1;
   }
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
int EventSelectorAthenaPool::curEvent (const Context& /*ctxt*/) const {
  return(m_evtCount);
}
//__________________________________________________________________________
// Search for event number evtNum.
// Return the index of the collection containing it, or -1 if not found.
// Note: passing -1 for evtNum will always yield failure,
// but this can be used to force filling in the entire m_numEvt array.
int EventSelectorAthenaPool::findEvent(int evtNum) const {
   for (std::size_t i = 0, imax = m_numEvt.size(); i < imax; i++) {
      if (m_numEvt[i] == -1) {
         PoolCollectionConverter pcc(m_collectionType.value() + ":" + m_collectionTree.value(),
	         m_inputCollectionsProp.value()[i],
	         IPoolSvc::kInputStream,
	         m_athenaPoolCnvSvc->getPoolSvc());
         if (!pcc.initialize().isSuccess()) {
            break;
         }
         int collection_size = 0;
         if (pcc.isValid()) {
            pool::ICollectionCursor* hi = &pcc.selectAll();
            collection_size = hi->size();
         }
         if (i > 0) {
            m_firstEvt[i] = m_firstEvt[i - 1] + m_numEvt[i - 1];
         } else {
            m_firstEvt[i] = 0;
         }
         m_numEvt[i] = collection_size;
      }
      if (evtNum >= m_firstEvt[i] && evtNum < m_firstEvt[i] + m_numEvt[i]) {
         return(i);
      }
   }
   return(-1);
}

//________________________________________________________________________________
StatusCode EventSelectorAthenaPool::makeServer(int num) {
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
StatusCode EventSelectorAthenaPool::makeClient(int num) {
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
StatusCode EventSelectorAthenaPool::share(int evtnum) {
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
StatusCode EventSelectorAthenaPool::readEvent(int maxevt) {
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
   IEvtSelector::Context* ctxt = new EventContextAthenaPool(this);
   for (int i = 0; i < maxevt || maxevt == -1; ++i) {
      if (!next(*ctxt).isSuccess()) {
         if (m_evtCount == -1) {
            ATH_MSG_VERBOSE("Called read Event and read last event from input: " << i);
            break;
         }
         ATH_MSG_ERROR("Cannot read Event " << m_evtCount - 1 << " into AthenaSharedMemoryTool");
         delete ctxt; ctxt = nullptr;
         return StatusCode::FAILURE;
      } else {
         ATH_MSG_VERBOSE("Called next, read Event " << m_evtCount - 1);
      }
   }
   delete ctxt; ctxt = nullptr;
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
int EventSelectorAthenaPool::size(Context& /*ctxt*/) const {
   // Fetch sizes of all collections.
   findEvent(-1);
   return std::accumulate(m_numEvt.begin(), m_numEvt.end(), 0);
}
//__________________________________________________________________________
std::unique_ptr<PoolCollectionConverter>
EventSelectorAthenaPool::getCollectionCnv(bool throwIncidents) const {
   while (m_inputCollectionsIterator != m_inputCollectionsProp.value().end()) {
      if (m_curCollection != 0) {
         m_numEvt[m_curCollection] = m_evtCount - m_firstEvt[m_curCollection];
         m_curCollection++;
         m_firstEvt[m_curCollection] = m_evtCount;
      }
      ATH_MSG_DEBUG("Try item: \"" << *m_inputCollectionsIterator << "\" from the collection list.");
      auto pCollCnv = std::make_unique<PoolCollectionConverter>(m_collectionType.value() + ":" + m_collectionTree.value(),
	      *m_inputCollectionsIterator,
	      IPoolSvc::kInputStream,
	      m_athenaPoolCnvSvc->getPoolSvc());
      StatusCode status = pCollCnv->initialize();
      if (!status.isSuccess()) {
         // Close previous collection.
         pCollCnv.reset();
         if (!status.isRecoverable()) {
            ATH_MSG_ERROR("Unable to initialize PoolCollectionConverter.");
            throw GaudiException("Unable to read: " + *m_inputCollectionsIterator, name(), StatusCode::FAILURE);
         } else {
            ATH_MSG_ERROR("Unable to open: " << *m_inputCollectionsIterator);
            throw GaudiException("Unable to open: " + *m_inputCollectionsIterator, name(), StatusCode::FAILURE);
         }
      } else {
         if (!pCollCnv->isValid().isSuccess()) {
            pCollCnv.reset();
            ATH_MSG_DEBUG("No events found in: " << *m_inputCollectionsIterator << " skipped!!!");
            if (throwIncidents && m_processMetadata.value()) {
               FileIncident beginInputFileIncident(name(), "BeginInputFile", *m_inputCollectionsIterator);
               m_incidentSvc->fireIncident(beginInputFileIncident);
               FileIncident endInputFileIncident(name(), "EndInputFile", "eventless " + *m_inputCollectionsIterator);
               m_incidentSvc->fireIncident(endInputFileIncident);
            }
            m_athenaPoolCnvSvc->getPoolSvc()->disconnectDb(*m_inputCollectionsIterator).ignore();
            ++m_inputCollectionsIterator;
         } else {
            return(pCollCnv);
         }
      }
   }
   return(nullptr);
}
//__________________________________________________________________________
StatusCode EventSelectorAthenaPool::recordAttributeList() const {
   // Get access to AttributeList
   ATH_MSG_DEBUG("Get AttributeList from the collection");
   // MN: accessing only attribute list, ignoring token list
   const coral::AttributeList& attrList = m_headerIterator->currentRow().attributeList();
   ATH_MSG_DEBUG("AttributeList size " << attrList.size());
   std::unique_ptr<AthenaAttributeList> athAttrList(new AthenaAttributeList(attrList));
   // Fill the new attribute list
   ATH_CHECK(fillAttributeList(athAttrList.get(), "", false));
   // Write the AttributeList
   SG::WriteHandle<AthenaAttributeList> wh(m_attrListKey.value(), eventStore()->name());
   ATH_CHECK(wh.record(std::move(athAttrList)));
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode EventSelectorAthenaPool::fillAttributeList(coral::AttributeList *attrList, const std::string &suffix, bool copySource) const
{
   const pool::TokenList& tokenList = m_headerIterator->currentRow().tokenList();
   for (pool::TokenList::const_iterator iter = tokenList.begin(), last = tokenList.end(); iter != last; ++iter) {
      attrList->extend(iter.tokenName() + suffix, "string");
      (*attrList)[iter.tokenName() + suffix].data<std::string>() = iter->toString();
      ATH_MSG_DEBUG("record AthenaAttribute, name = " << iter.tokenName() + suffix << " = " << iter->toString() << ".");
   }

   std::string eventRef = "eventRef";
   if (m_isSecondary.value()) {
      eventRef.append(suffix);
   }
   attrList->extend(eventRef, "string");
   (*attrList)[eventRef].data<std::string>() = m_headerIterator->eventRef().toString();
   ATH_MSG_DEBUG("record AthenaAttribute, name = " + eventRef + " = " <<  m_headerIterator->eventRef().toString() << ".");

   if (copySource) {
      const coral::AttributeList& sourceAttrList = m_headerIterator->currentRow().attributeList();
      for (const auto &attr : sourceAttrList) {
         attrList->extend(attr.specification().name() + suffix, attr.specification().type());
         (*attrList)[attr.specification().name() + suffix] = attr;
      }
   }

   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode EventSelectorAthenaPool::io_reinit() {
   ATH_MSG_INFO("I/O reinitialization...");
   if (m_poolCollectionConverter) {
      m_poolCollectionConverter->disconnectDb().ignore();
      m_poolCollectionConverter.reset();
   }
   m_headerIterator = nullptr;
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   if (!iomgr.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Could not retrieve IoComponentMgr !");
      return StatusCode::FAILURE;
   }
   if (!iomgr->io_hasitem(this)) {
      ATH_MSG_FATAL("IoComponentMgr does not know about myself !");
      return StatusCode::FAILURE;
   }
   if (!m_eventStreamingTool.empty() && m_eventStreamingTool->isClient()) {
      m_guid = Guid::null();
      return(this->reinit());
   }
   std::vector<std::string> inputCollections = m_inputCollectionsProp.value();
   std::set<std::size_t> updatedIndexes;
   for (std::size_t i = 0, imax = m_inputCollectionsProp.value().size(); i < imax; i++) {
      if (updatedIndexes.find(i) != updatedIndexes.end()) continue;
      std::string savedName = inputCollections[i];
      std::string &fname = inputCollections[i];
      if (!iomgr->io_contains(this, fname)) {
         ATH_MSG_ERROR("IoComponentMgr does not know about [" << fname << "] !");
         return StatusCode::FAILURE;
      }
      if (!iomgr->io_retrieve(this, fname).isSuccess()) {
         ATH_MSG_FATAL("Could not retrieve new value for [" << fname << "] !");
         return StatusCode::FAILURE;
      }
      if (savedName != fname) {
         ATH_MSG_DEBUG("Mapping value for [" << savedName << "] to [" << fname << "]");
         m_athenaPoolCnvSvc->getPoolSvc()->renamePfn(savedName, fname);
      }
      updatedIndexes.insert(i);
      for (std::size_t j = i + 1; j < imax; j++) {
         if (inputCollections[j] == savedName) {
            inputCollections[j] = fname;
            updatedIndexes.insert(j);
         }
      }
   }
   // all good... copy over.
   m_inputCollectionsProp = inputCollections;
   m_guid = Guid::null();
   return reinit();
}
//__________________________________________________________________________
StatusCode EventSelectorAthenaPool::io_finalize() {
   ATH_MSG_INFO("I/O finalization...");
   if (m_poolCollectionConverter) {
      m_poolCollectionConverter->disconnectDb().ignore();
      m_poolCollectionConverter.reset();
   }
   return StatusCode::SUCCESS;
}

//__________________________________________________________________________
/* Listen to IncidentType::BeginProcessing and EndProcessing
   Maintain counters of how many events from a given file are being processed.
   Files are identified by SG::SourceID (string GUID).
   When there are no more events from a file, see if it can be closed.
*/
void EventSelectorAthenaPool::handle(const Incident& inc)
{
   SG::SourceID fid;
   if (inc.type() == IncidentType::BeginProcessing) {
     if ( Atlas::hasExtendedEventContext(inc.context()) ) {
       fid = Atlas::getExtendedEventContext(inc.context()).proxy()->sourceID();
     }
     *m_sourceID.get(inc.context()) = fid;
   }
   else {
     fid = *m_sourceID.get(inc.context());
   }

   if( fid.empty() ) {
      ATH_MSG_WARNING("could not read event source ID from incident event context");
      return;
   }
   if( m_activeEventsPerSource.find( fid ) == m_activeEventsPerSource.end()) {
      ATH_MSG_DEBUG("Incident handler ignoring unknown input FID: " << fid );
      return;
   }
   ATH_MSG_DEBUG("**  MN Incident handler " << inc.type() << " Event source ID=" << fid );
   if( inc.type() == IncidentType::BeginProcessing ) {
      // increment the events-per-file counter for FID
      m_activeEventsPerSource[fid]++;
   } else if( inc.type() == IncidentType::EndProcessing ) {
      m_activeEventsPerSource[fid]--;
      disconnectIfFinished( fid );
      *m_sourceID.get(inc.context()) = "";
   }
   if( msgLvl(MSG::DEBUG) ) {
      for( auto& source: m_activeEventsPerSource )
         msg(MSG::DEBUG) << "SourceID: " << source.first << " active events: " << source.second << endmsg;
   }
}

//__________________________________________________________________________
/* Disconnect APR Database identifieed by a SG::SourceID when it is no longer in use:
   m_guid is not pointing to it and there are no events from it being processed
   (if the EventLoopMgr was not firing Begin/End incidents, this will just close the DB)
*/
bool EventSelectorAthenaPool::disconnectIfFinished( const SG::SourceID &fid ) const
{
   if( m_eventStreamingTool.empty() && m_activeEventsPerSource.find(fid) != m_activeEventsPerSource.end() 
           && m_activeEventsPerSource[fid] <= 0 && m_guid != fid ) {
      // Explicitly disconnect file corresponding to old FID to release memory
      if( !m_keepInputFilesOpen.value() ) {
         // Assume that the end of collection file indicates the end of payload file.
         if (m_processMetadata.value()) {
            FileIncident endInputFileIncident(name(), "EndInputFile", "FID:" + fid, fid);
            m_incidentSvc->fireIncident(endInputFileIncident);
         }
         ATH_MSG_INFO("Disconnecting input sourceID: " << fid );
         m_athenaPoolCnvSvc->getPoolSvc()->disconnectDb("FID:" + fid, IPoolSvc::kInputStream).ignore();
         m_activeEventsPerSource.erase( fid );
         return true;
      }
   }
   return false;
}
