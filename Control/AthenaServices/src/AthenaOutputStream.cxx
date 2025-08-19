/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaOutputStream.h"

// STL include files
#include <cassert>
#include <format>
#include <sstream>
#include <ranges>

// Framework include files
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "AthContainersInterfaces/IConstAuxStore.h"
#include "AthenaKernel/CLIDRegistry.h"
#include "AthenaKernel/ITPCnvBase.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "PersistentDataModel/DataHeader.h"
#include "RootAuxDynIO/RootAuxDynDefs.h"
#include "SGTools/DataProxy.h"
#include "SGTools/ProxyMap.h"
#include "SGTools/SGIFolder.h"
#include "SGTools/TransientAddress.h"
#include "SGTools/transientKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteHandle.h"
#include "xAODCore/AuxCompression.h"
#include "xAODCore/AuxSelection.h"

// Local include files
#include "AltDataBucket.h"

// Standard Constructor
AthenaOutputStream::AthenaOutputStream(const std::string& name, ISvcLocator* pSvcLocator)
      : base_class(name, pSvcLocator),
        m_currentStore(&m_dataStore),
        m_p2BWritten(std::format("SG::Folder/{}_TopFolder", name), this),
        m_compressionDecoderHigh(std::format("SG::Folder/{}_compressed_high", name), this),
        m_compressionDecoderLow(std::format("SG::Folder/{}_compressed_low", name), this),
        m_transient(std::format("SG::Folder/{}_transient", name), this),
        m_streamer(std::format("AthenaOutputStreamTool/{}Tool", name), this)
{
   // Ensure the service locater is good
   assert(pSvcLocator);

   // This property depends on the name that's known at construction time
   // Therefore, do it the old fashioned way
   declareProperty("WritingTool", m_streamer);

   // Associate action handlers with the AcceptAlgs,
   // RequireAlgs & VetoAlgs properties
   m_itemList.declareUpdateHandler(&AthenaOutputStream::itemListHandler, this);
   m_compressionListHigh.declareUpdateHandler(&AthenaOutputStream::compressionListHandlerHigh, this);
   m_compressionListLow.declareUpdateHandler(&AthenaOutputStream::compressionListHandlerLow, this);
}

// Standard Destructor
AthenaOutputStream::~AthenaOutputStream() {
   // Clear the internal caches
   m_streamerMap.clear();
}

// Initialize data writer
StatusCode AthenaOutputStream::initialize() {
   ATH_MSG_DEBUG("In initialize");

   // Initialize the FilteredAlgorithm base
   ATH_CHECK( this->FilteredAlgorithm::initialize() );

   // Reset the number of events written
   m_events = 0;

   // Set up the SG services
   ATH_CHECK( m_dataStore.retrieve() );
   ATH_MSG_DEBUG(std::format("Found {} store.", m_dataStore.typeAndName()));
   if (!m_metadataItemList.value().empty()) {
      ATH_CHECK( m_metadataStore.retrieve() );
      ATH_MSG_DEBUG(std::format("Found {} store.", m_metadataStore.typeAndName()));
   }

   // Set up various services
   ATH_CHECK( m_pCLIDSvc.retrieve() );
   ATH_CHECK( m_dictLoader.retrieve() );
   ATH_CHECK( m_tpCnvSvc.retrieve() );
   ATH_CHECK( m_itemSvc.retrieve() );
   ATH_CHECK( m_outSeqSvc.retrieve() );

   // Get Output Stream tool for writing
   ATH_CHECK( m_streamer.retrieve() );
   ATH_CHECK( m_streamer->connectServices(m_dataStore.typeAndName(), m_persName, m_extendProvenanceRecord) );

   ATH_CHECK( m_helperTools.retrieve() );
   ATH_MSG_INFO("Found " << m_helperTools);
   ATH_MSG_INFO(std::format("Data output: {}", m_outputName.toString()));

   for (auto& tool : m_helperTools) {
     ATH_CHECK( tool->postInitialize() );
   }

   // Register this algorithm for 'I/O' events
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   ATH_CHECK( iomgr.retrieve() );
   ATH_CHECK( iomgr->io_register(this) );
   ATH_CHECK( iomgr->io_register(this, IIoComponentMgr::IoMode::WRITE, m_outputName) );
   ATH_CHECK( this->io_reinit() );

   // Add an explicit input dependency for everything in our item list
   // that we know from the configuration is in the transient store.
   // We don't want to add everything on the list, because configurations
   // often initialize this with a maximal static list of everything
   // that could possibly be written.
   {
     ATH_CHECK( m_transient.retrieve() );
     IProperty *pAsIProp = dynamic_cast<IProperty*> (&*m_transient);
     if (!pAsIProp) {
       ATH_MSG_FATAL ("Bad folder interface");
       return StatusCode::FAILURE;
     }
     ATH_CHECK (pAsIProp->setProperty("ItemList", m_transientItems.toString()));

     for (const SG::FolderItem& item : *m_p2BWritten) {
       // Load ROOT dictionaries now.
       loadDict (item.id());

       const std::string& k = item.key();
       if (k.find('*') != std::string::npos) continue;
       if (k.find('.') != std::string::npos) continue;
       for (const SG::FolderItem& titem : *m_transient) {
         if (titem.id() == item.id() && titem.key() == k) {
           DataObjID id (item.id(), std::format("{}+{}", m_dataStore.name(), k));
           this->addDependency (id, Gaudi::DataHandle::Reader);
           break;
         }
       }
     }
     m_transient->clear();
   }

   // Also load dictionaries for metadata classes.
   if (!m_metadataItemList.value().empty()) {
     IProperty *pAsIProp = dynamic_cast<IProperty*> (&*m_transient);
     if (!pAsIProp) {
       ATH_MSG_FATAL ("Bad folder interface");
       return StatusCode::FAILURE;
     }
     ATH_CHECK (pAsIProp->setProperty("ItemList", m_metadataItemList.toString()));
     for (const SG::FolderItem& item : *m_transient) {
       loadDict (item.id());
     }
     m_transient->clear();
   }

   // Also make sure we have the dictionary for Token.
   m_dictLoader->load_type ("Token");

   // Listen to event range incidents if incident name is configured
   ATH_CHECK( m_incidentSvc.retrieve() );
   if( !m_outSeqSvc->incidentName().empty() ) {
      // use priority 95 to make sure the Output Sequencer goes first (it has priority 100)
      m_incidentSvc->addListener(this, IncidentType::BeginProcessing, 95);
      m_incidentSvc->addListener(this, IncidentType::EndProcessing, 95);
   }

   // Check compression settings and print some information about the configuration
   // Both should be between [5, 23] and high compression should be < low compression
   if(m_compressionBitsHigh < 5 || m_compressionBitsHigh > 23) {
     ATH_MSG_INFO(std::format("Float compression mantissa bits for high compression "
                              "({}) is outside the allowed range of [5, 23].",
                              m_compressionBitsHigh.toString()));
     ATH_MSG_INFO("Setting it to the appropriate limit.");
     m_compressionBitsHigh = m_compressionBitsHigh < 5 ? 5 : 23;
   }
   if(m_compressionBitsLow < 5 || m_compressionBitsLow > 23) {
     ATH_MSG_INFO(std::format("Float compression mantissa bits for low compression "
                              "({}) is outside the allowed range of [5, 23].",
                              m_compressionBitsLow.toString()));
     ATH_MSG_INFO("Setting it to the appropriate limit.");
     m_compressionBitsLow = m_compressionBitsLow < 5 ? 5 : 23;
   }
   if(m_compressionBitsLow <= m_compressionBitsHigh) {
     ATH_MSG_ERROR(std::format("Float compression mantissa bits for low compression "
                               "({}) is lower than or equal to high compression "
                               "({})! Please check the configuration! ",
                               m_compressionBitsLow.toString(),
                               m_compressionBitsHigh.toString()));
     return StatusCode::FAILURE;
   }
   if(m_compressionListHigh.value().empty() && m_compressionListLow.value().empty()) {
     ATH_MSG_VERBOSE("Both high and low float compression lists are empty. Float compression will NOT be applied.");
   } else {
     ATH_MSG_INFO("Either high or low (or both) float compression lists are defined. Float compression will be applied.");
     ATH_MSG_INFO(std::format("High compression will use {} mantissa bits, and "
                              "low compression will use {} mantissa bits.",
                              m_compressionBitsHigh.toString(),
                              m_compressionBitsLow.toString()));
   }

   // Setup stream name
   if (m_streamName.empty()) {
      m_streamName.setValue(this->name());
   }

   // Set SG key for selected variable information.
   m_selVetoesKey = std::format("SelectionVetoes_{}", m_streamName.toString());
   ATH_CHECK( m_selVetoesKey.initialize() );

   m_compInfoKey = std::format("CompressionInfo_{}", m_streamName.toString());
   ATH_CHECK( m_compInfoKey.initialize() );

   ATH_MSG_DEBUG("End initialize");
   return StatusCode::SUCCESS;
}

// Handle incidents
void AthenaOutputStream::handle(const Incident& inc)
{
   ATH_MSG_DEBUG(std::format("handle() incident type: {}", inc.type()));
   // mutex shared with write() which is called from writeMetaData
   std::unique_lock<mutex_t>  lock(m_mutex);

   if( inc.type() == "MetaDataStop" )  {
      if( m_outSeqSvc->inUse() ) {
         if( m_outSeqSvc->inConcurrentEventsMode() ) {
            // EventService MT - write metadata and close all remaining substreams
            while( m_streamerMap.size() > 0 ) {
               finalizeRange( m_streamerMap.begin()->first );
            }
            return;
         }
         if( m_outSeqSvc->lastIncident() == "EndEvent" ) {
            // in r22 EndEvent comes before output writing
            // - queue metadata writing and disconnect for after Event write
            m_writeMetadataAndDisconnect = true;
            return;
         }
      }
      // not in Event Service
      writeMetaData();
   }
   else if( m_outSeqSvc->inUse() ) {
      // Handle Event Ranges for Event Service
      EventContext::ContextID_t slot = inc.context().slot();
      if( slot == EventContext::INVALID_CONTEXT_ID ) {
         throw GaudiException("Received Incident with invalid slot in ES mode", name(), StatusCode::FAILURE);
      }
      auto count_events_in_range = [&](const std::string& range) {
         return std::count_if(m_slotRangeMap.cbegin(), m_slotRangeMap.cend(),
                              [&](auto& el){return el.second == range;} );
      };
      if( inc.type() == IncidentType::BeginProcessing ) {
         // get the current/old range filename for this slot
         const std::string rangeFN = m_slotRangeMap[ slot ];
         // build the new range filename for this slot
         const std::string newRangeFN = m_outSeqSvc->buildSequenceFileName( m_outputName );
         if( !rangeFN.empty() and rangeFN != newRangeFN ) {
            ATH_MSG_INFO(std::format("Slot range change: '{}' -> '{}'", rangeFN, newRangeFN));
            ATH_MSG_DEBUG(std::format("There are {} slots in use",m_slotRangeMap.size()));
            for(const auto & range : m_slotRangeMap ) {
               ATH_MSG_DEBUG(std::format("Slot: {}  FN={}", range.first, range.second));
            }
            if( count_events_in_range(rangeFN) == 1 ) {
               finalizeRange( rangeFN );
            }
         }
         ATH_MSG_INFO(std::format("slot {} processing event in range: {}", slot, newRangeFN));
         m_slotRangeMap[ slot ] = newRangeFN;
         // remember the RangeID for this slot so we can write metadata *after* a range change
         m_rangeIDforRangeFN[ newRangeFN ] = m_outSeqSvc->currentRangeID();
      }
      else if( inc.type() == IncidentType::EndProcessing ) {
         ATH_MSG_DEBUG(std::format("There are {} slots in use", m_slotRangeMap.size()));
         for( const auto& range : m_slotRangeMap ) {
            ATH_MSG_DEBUG(std::format("Slot: {}  FN={}", range.first, range.second));
         }
         if( m_slotRangeMap.size() > 1 ) {
            // if there are multiple slots, we can detect if the range ended with this event
            // - except the last range, because there is no next range to clear the slot map
            const std::string rangeFN = m_slotRangeMap[ slot ];
            if( count_events_in_range(rangeFN) == 1 ) {
               finalizeRange( rangeFN );
               m_slotRangeMap[ slot ].clear();
            }
         }
      }
   }
   ATH_MSG_DEBUG(std::format("Leaving incident handler for {}", inc.type()));
}

// Note - this method works in any slot - MetaCont uses the filenames to find objects
void AthenaOutputStream::finalizeRange( const std::string & rangeFN )
{
   ATH_MSG_DEBUG(std::format("Writing MetaData to {}", rangeFN));
   // MN: not calling StopMetaData Incident here but directly writeMetaData() - OK for Sim, check others
   // metadata tools like CutFlowSvc are not able to handle this yet
   const std::string rememberID = m_outSeqSvc->setRangeID( m_rangeIDforRangeFN[ rangeFN ] );
   writeMetaData( rangeFN );
   m_outSeqSvc->setRangeID( rememberID );

   ATH_MSG_INFO(std::format("Finished writing Event Sequence to {}", rangeFN));
   auto strm_iter = m_streamerMap.find( rangeFN );
   strm_iter->second->finalizeOutput().ignore();
   strm_iter->second->finalize().ignore();
   m_streamerMap.erase( strm_iter );
   m_outSeqSvc->publishRangeReport( rangeFN );
}

// Method to write MetaData for this stream
// in ES mode the range substream is determined by the current Event slot
// called from the incident handler - returns void and throws GaudiExceptions on errors
void AthenaOutputStream::writeMetaData(const std::string& outputFN)
{
   // use main stream tool by default, or per outputFile in ES mode
   IAthenaOutputStreamTool* streamer = outputFN.empty()? &*m_streamer : m_streamerMap[outputFN].get();

   for (auto& tool : m_helperTools) {
      if (!tool->preFinalize().isSuccess()) {
         throw GaudiException("Cannot finalize helper tool", name(), StatusCode::FAILURE);
      }
   }
   if( m_metaDataSvc->prepareOutput(outputFN).isFailure() ) {
      throw GaudiException("Failed on MetaDataSvc prepareOutput", name(), StatusCode::FAILURE);
   }
   // lock all metadata to prevent updates during writing
   MetaDataSvc::ToolLockGuard   tool_guard( *m_metaDataSvc );

   // Prepare the WriteDataHeaderForms incident
   std::string DHFWriteIncidentfileName = m_outSeqSvc->buildSequenceFileName(m_outputName);
   // remove technology from the name
   size_t pos = DHFWriteIncidentfileName.find(':');
   if( pos != std::string::npos ) DHFWriteIncidentfileName = DHFWriteIncidentfileName.substr(pos+1);
   FileIncident incident(name(), "WriteDataHeaderForms", DHFWriteIncidentfileName);
   m_incidentSvc->fireIncident(incident);

   ATH_MSG_DEBUG("metadataItemList: " << m_metadataItemList.value() );
   if (!m_metadataItemList.value().empty()) {
      m_currentStore = &m_metadataStore;
      StatusCode status = streamer->connectServices(m_metadataStore.typeAndName(), m_persName, false);
      if (status.isFailure()) {
         throw GaudiException("Unable to connect metadata services", name(), StatusCode::FAILURE);
      }
      m_outputAttributes = "[OutputCollection=MetaDataHdr][PoolContainerPrefix=MetaData][AttributeListKey=]";
      m_p2BWritten->clear();
      IProperty *pAsIProp(nullptr);
      if ((m_p2BWritten.retrieve()).isFailure() ||
          nullptr == (pAsIProp = dynamic_cast<IProperty*>(&*m_p2BWritten)) ||
          (pAsIProp->setProperty("ItemList", m_metadataItemList.toString())).isFailure()) {
         throw GaudiException("Folder property [metadataItemList] not found", name(), StatusCode::FAILURE);
      }
      if (write().isFailure()) {
         throw GaudiException("Cannot write metadata", name(), StatusCode::FAILURE);
      }
      FileIncident incident(name(), "WriteDataHeaderForms", DHFWriteIncidentfileName + m_outputAttributes);
      m_incidentSvc->fireIncident(incident);

      m_outputAttributes.clear();
      m_currentStore = &m_dataStore;
      status = streamer->connectServices(m_dataStore.typeAndName(), m_persName, m_extendProvenanceRecord);
      if (status.isFailure()) {
         throw GaudiException("Unable to re-connect services", name(), StatusCode::FAILURE);
      }
      m_p2BWritten->clear();
      if ((pAsIProp->setProperty(m_itemList)).isFailure()) {
         throw GaudiException("Folder property [itemList] not found", name(), StatusCode::FAILURE);
      }
      ATH_MSG_DEBUG(std::format("Metadata items written: {}", m_metadataItemList.value().size()));
   }
}

// Terminate data writer
StatusCode AthenaOutputStream::finalize()
{
   bool failed = false;
   ATH_MSG_DEBUG("finalize: Optimize output");
   // Connect the output file to the service
   if (!m_streamer->finalizeOutput().isSuccess()) {
      failed = true;
   }
   ATH_MSG_DEBUG("finalize: end optimize output");
   // Release the tools
   if (!m_helperTools.release().isSuccess()) {
      failed = true;
   }
   if (!m_streamer.release().isSuccess()) {
      failed = true;
   }
   if (failed) {
      return(StatusCode::FAILURE);
   }
   // Clear the internal caches
   m_objects.clear();
   m_objects.shrink_to_fit();
   m_ownedObjects.clear();
   m_altObjects.clear();
   return(StatusCode::SUCCESS);
}

// Execute data writer
StatusCode AthenaOutputStream::execute() {
   bool failed = false;
   // Call tool preExecute prior to writing
   for (auto& tool : m_helperTools) {
      if (!tool->preExecute().isSuccess()) {
         failed = true;
      }
   }
   // Write the event if the event is accepted
   if (isEventAccepted()) {
      if (write().isFailure()) {
         failed = true;
      }
   }
   // Call tool postExecute after writing
   for (auto& tool : m_helperTools) {
      if(!tool->postExecute().isSuccess()) {
         failed = true;
      }
   }
   // See if we should write metadata and do if so
   if( m_writeMetadataAndDisconnect ) {
      writeMetaData();
      m_writeMetadataAndDisconnect = false;
      // finalize will disconnect output
      if( !finalize().isSuccess() ) {
         failed = true;
      }
   }
   if (failed) {
      return(StatusCode::FAILURE);
   }
   return(StatusCode::SUCCESS);
}

// The main method that performs the writing
StatusCode AthenaOutputStream::write() {
   bool failed = false;
   IAthenaOutputStreamTool* streamer = &*m_streamer;
   std::string outputFN;

   std::unique_lock<mutex_t>  lock(m_mutex);
   outputFN = m_outSeqSvc->buildSequenceFileName( m_outputName );

   // Handle Event Ranges
   if( m_outSeqSvc->inUse() and m_outSeqSvc->inConcurrentEventsMode() ) {
      ATH_MSG_DEBUG(std::format("Writing event sequence to {}", outputFN));
      streamer = m_streamerMap[ outputFN ].get();
      if( !streamer ) {
         // new range, needs a new streamer tool
         IAlgTool* st = AlgTool::Factory::create( m_streamer->type(), m_streamer->type(), m_streamer->name(), this ).release();
         st->addRef();
         streamer = dynamic_cast<IAthenaOutputStreamTool*>( st );
         IProperty *mstreamer_props = dynamic_cast<IProperty*> (&*m_streamer);
         IProperty *streamer_props = dynamic_cast<IProperty*> (&*streamer);
         for ( const auto& prop : mstreamer_props->getProperties() ) {
            ATH_CHECK( streamer_props->setProperty( *prop ) );
         }
         if( !streamer or streamer->initialize().isFailure()
             or streamer->connectServices(m_dataStore.typeAndName(), m_persName, m_extendProvenanceRecord).isFailure() ) {
            ATH_MSG_FATAL(std::format("Unable to initialize OutputStreamTool for {}", outputFN));
            return StatusCode::FAILURE;
         }
         m_streamerMap[ outputFN ].reset( streamer );
      }
   }

   // Clear any previously existing item list
   // and collect all objects that are asked to be written out
   clearSelection();
   ATH_CHECK( collectAllObjects() );

   // keep a local copy of the object lists so they are not overwritten when we release the lock
   IDataSelector objects = std::move( m_objects );
   IDataSelector altObjects = std::move( m_altObjects );
   std::vector<std::unique_ptr<DataObject> > ownedObjects = std::move( m_ownedObjects );

   // prepare before releasing lock because m_outputAttributes change in metadataStop
   const std::string connectStr = outputFN + m_outputAttributes;

   for (auto& tool : m_helperTools) {
     ATH_CHECK( tool->preStream() );
   }

   // MN: would be nice to release the Stream lock here
   // lock.unlock();

   // Connect the output file to the service
   if (!streamer->connectOutput(connectStr).isSuccess()) {
      ATH_MSG_FATAL("Could not connectOutput");
      return StatusCode::FAILURE;
   }
   ATH_MSG_DEBUG(std::format("connectOutput done for {}", outputFN));
   StatusCode currentStatus = streamer->streamObjects(objects, connectStr);
   // Do final check of streaming
   if (!currentStatus.isSuccess()) {
      if (!currentStatus.isRecoverable()) {
         ATH_MSG_FATAL("streamObjects failed.");
         failed = true;
      } else {
         ATH_MSG_DEBUG("streamObjects failed.");
      }
   }
   bool doCommit = false;
   if (!streamer->commitOutput(doCommit).isSuccess()) {
      ATH_MSG_FATAL("commitOutput failed.");
      failed = true;
   }
   if (failed) {
      return(StatusCode::FAILURE);
   }
   m_events++;
   return(StatusCode::SUCCESS);
}

// Clear collected object list
void AthenaOutputStream::clearSelection()     {
   m_objects.clear();
   m_ownedObjects.clear();
   m_altObjects.clear();
}

// Collect objects
StatusCode AthenaOutputStream::collectAllObjects() {
   if (m_itemListFromTool) {
      if (!m_streamer->getInputItemList(&*m_p2BWritten).isSuccess()) {
         ATH_MSG_WARNING("collectAllObjects() could not get ItemList from Tool.");
      }
   }

   // This holds the vetoes for the AuxID selection
   auto vetoes = std::make_unique<SG::SelectionVetoes>();
   // This holds the lossy float compression information
   auto compInfo = std::make_unique<SG::CompressionInfo>();

   m_p2BWritten->updateItemList(true);
   // Collect all objects that need to be persistified:
   for (const auto& i : *m_p2BWritten) {
      ATH_CHECK( addItemObjects(i, *vetoes, *compInfo) );
   }

   // If there were any variable selections, record the information in SG.
   if (!vetoes->empty()) {
     ATH_CHECK( SG::makeHandle (m_selVetoesKey).record (std::move (vetoes)) );
   }

   // Store the lossy float compression information in the SG.
   if (!compInfo->empty()) {
     ATH_CHECK( SG::makeHandle (m_compInfoKey).record (std::move (compInfo)) );
   }

   return StatusCode::SUCCESS;
}

// Build a list of objects we're going to write out
// This function also builds the list of vetoed AuxIDs
// and the lossy float compression lists.
StatusCode AthenaOutputStream::addItemObjects(const SG::FolderItem& item,
                                              SG::SelectionVetoes& vetoes,
                                              SG::CompressionInfo& compInfo)
{
   // anything after a dot is a list of dynamic Aux attributes, separated by dots
   size_t dotpos = item.key().find('.');
   std::string item_key, aux_attr;
   if( dotpos != std::string::npos ) {
      item_key = item.key().substr(0, dotpos+1);
      aux_attr = item.key().substr(dotpos+1);
   } else {
      item_key = item.key();
   }
   CLID item_id = item.id();
   ATH_MSG_DEBUG(std::format("addItemObjects({},\"{}\") called", item_id, item_key));
   ATH_MSG_DEBUG(std::format("           Key:{}", item_key));
   if( aux_attr.size() ) {
      ATH_MSG_DEBUG(std::format("      Aux Attr:{}", aux_attr));
   }

   // Here we build the list of attributes for the lossy float compression
   // Note that we do not allow m_compressionBitsHigh >= m_compressionBitsLow
   // Otherwise is, in any case, a logical error and they'd potentially overwrite each other
   std::map< unsigned int, std::set< std::string > > comp_attr_map;
   comp_attr_map[ m_compressionBitsHigh ] = buildCompressionSet( m_compressionDecoderHigh, item_id, item_key );
   comp_attr_map[ m_compressionBitsLow  ] = buildCompressionSet( m_compressionDecoderLow, item_id, item_key );

   // Print some debugging information regarding the lossy float compression configuration
   for( const auto& it : comp_attr_map ) {
     ATH_MSG_DEBUG(std::format("     Comp Attr {} with {} mantissa bits.", it.second.size(), it.first));
     if ( it.second.size() > 0 ) {
       for( const auto& attr : it.second ) {
          ATH_MSG_DEBUG(std::format("       >> {}", attr));
       }
     }
   }

   // For MetaData objects of type T that are kept in MetaContainers get the MetaCont<T> ID
   const CLID remapped_item_id = m_metaDataSvc->remapMetaContCLID( item_id );
   SG::ConstProxyIterator iter, end;
   SG::ProxyMap map;
   bool gotProxies = false;
   // Look for the clid in storegate
   SG::DataProxy* match = (*m_currentStore)->proxy(remapped_item_id, item_key, true);
   if (match != nullptr) {
      map.insert({item_key, match});
      iter = map.begin();
      end = map.end();
      gotProxies = true;
   }
   // Look for the clid in storegate
   if (!gotProxies && ((*m_currentStore)->proxyRange(remapped_item_id, iter, end)).isSuccess()) {
      gotProxies = true;
   }
   if (gotProxies) {
      bool added = false, removed = false;
      // Now loop over any found proxies
      for (; iter != end; ++iter) {
         SG::DataProxy* itemProxy(iter->second);
         std::string proxyName = itemProxy->name();
         std::string stream;
         if( m_currentStore == &m_metadataStore ) {
            // only check metadata keys
            stream = m_metaDataSvc->removeStreamFromKey(proxyName);  // can modify proxyName
         }
         // Does this key match the proxy key name - allow for wildcarding and aliases
         bool keyMatch = ( item_key == "*" ||
                           item_key == proxyName ||
                           itemProxy->hasAlias(item_key) );
         if (!keyMatch) {
            // For item list we currently allow wildcards ('*'), which has limited use, e.g.:
            // xAOD::CutBookkeeperAuxContainer#IncompleteCutBookkeepers*Aux.
            // Here we look for those few cases...
            // Escape some regex metacharacters first
            std::string item_key_escaped = std::regex_replace(item_key, std::regex(R"([${}|(){}\[\]+?])"), R"(\\$&)");
            // Replace '*' with '.*'
            item_key_escaped = std::regex_replace(item_key_escaped, std::regex(R"(\*)"), ".*");
            // Now see if proxyName matches the pattern in the item list
            const std::regex pattern(item_key_escaped);
            keyMatch = std::regex_match(proxyName, pattern);
            ATH_MSG_DEBUG(std::format("Result of checking {} against {} to see if it matches,"
                                      " original pattern being {}, is {}",
                                      proxyName, item_key_escaped, item_key, keyMatch));
         }

         // Now check if this item is marked for another output stream, if so we reject it
         // We also reject keys that are marked transient at this point
         bool xkeyMatch = false;
         if( (!stream.empty() and stream != m_outputName) || SG::isTransientKey(proxyName) ) {
            // reject keys that are marked for a different output stream
            ATH_MSG_DEBUG(std::format("Rejecting key: {} in output: {}", itemProxy->name(), m_outputName.toString()));
            xkeyMatch = true;
         }

         // All right, it passes key match find in itemList, but not in excludeList
         if (keyMatch && !xkeyMatch) {
            if (m_forceRead && itemProxy->isValid()) {
               if (nullptr == itemProxy->accessData()) {
                  ATH_MSG_ERROR(std::format(" Could not get data object for id {},\"{}\"", remapped_item_id, proxyName));
               }
            }
            if (nullptr != itemProxy->object()) {
               if( std::find(m_objects.begin(), m_objects.end(), itemProxy->object()) == m_objects.end() &&
                   std::find(m_altObjects.begin(), m_altObjects.end(), itemProxy->object()) == m_altObjects.end() )
               {
                  if( item_id != remapped_item_id ) {
                     // For MetaCont<T>: -
                     // create a temporary DataObject for an entry in the  container to pass to CnvSvc
                     DataBucketBase* dbb = static_cast<DataBucketBase*>( itemProxy->object() );
                     const MetaContBase* metaCont = static_cast<MetaContBase*>( dbb->cast( ClassID_traits<MetaContBase>::ID() ) );
                     void* obj = metaCont? metaCont->getAsVoid( m_outSeqSvc->currentRangeID() ) : nullptr;
                     if( obj ) {
                        auto altbucket = std::make_unique<AltDataBucket>(
                           obj, item_id, *CLIDRegistry::CLIDToTypeinfo(item_id), proxyName );
                        m_objects.push_back( altbucket.get() );
                        m_ownedObjects.push_back( std::move(altbucket) );
                        m_altObjects.push_back( itemProxy->object() ); // only for duplicate prevention
                     } else {
                        ATH_MSG_ERROR(std::format("Failed to retrieve object from MetaCont with key={}, for EventRangeID={}",
                                      item_key, m_outSeqSvc->currentRangeID()));
                        return StatusCode::FAILURE;
                     }
                  } else if (item.exact()) {
                   // If the exact flag is set, make a new DataObject
                   // holding the object as the requested type.
                   DataBucketBase* dbb = dynamic_cast<DataBucketBase*> (itemProxy->object());
                   if (!dbb) std::abort();
                   void* ptr = dbb->cast (item_id);
                   if (!ptr) {
                     // Hard cast
                     ptr = dbb->object();
                   }
                   auto altbucket =
                     std::make_unique<AltDataBucket>
                       (ptr, item_id,
                        *CLIDRegistry::CLIDToTypeinfo (item_id),
                        *itemProxy);
                   m_objects.push_back(altbucket.get());
                   m_ownedObjects.push_back (std::move(altbucket));
                   m_altObjects.push_back (itemProxy->object());
                 }
                 else
                   m_objects.push_back(itemProxy->object());
                 ATH_MSG_DEBUG(std::format(" Added object {},\"{}\"", item_id, proxyName));
               }

               // Build ItemListSvc string
               std::string tn;
               std::stringstream tns;
               if (!m_pCLIDSvc->getTypeNameOfID(item_id, tn).isSuccess()) {
                  ATH_MSG_ERROR(std::format(" Could not get type name for id {},\"{}\"", item_id, proxyName));
                  tns << item_id << '_' << proxyName;
               } else {
                  tn += '_' + proxyName;
                  tns << tn;
               }

               /// Handle variable selections.
               /// Both variable selection and lossy float compression
               /// are limited to event data for the time being
               if ((*m_currentStore)->storeID() == StoreID::EVENT_STORE &&
                   item_key.find( RootAuxDynIO::AUX_POSTFIX ) == ( item_key.size() - 4 )) {

                  const SG::IConstAuxStore* auxstore( nullptr );
                  try {
                    SG::fromStorable( itemProxy->object(), auxstore, true );
                  } catch( const std::exception& ) {
                    ATH_MSG_DEBUG(std::format("Error in casting object with CLID {} to SG::IConstAuxStore*", itemProxy->clID()));
                    auxstore = nullptr;
                  }

                  if (auxstore) {
                    handleVariableSelection (*auxstore, *itemProxy,
                                             tns.str(), aux_attr,
                                             vetoes);

                    // Here comes the compression logic using ThinningInfo
                    // Get a hold of all AuxIDs for this store (static, dynamic etc.)
                    const SG::auxid_set_t allVars = auxstore->getAuxIDs();

                    // Get a handle on the compression information for this store
                    std::string key = item_key;
                    key.erase (key.size()-4, 4);

                    // Build the compression list, retrieve the relevant AuxIDs and
                    // store it in the relevant map that is going to be inserted into
                    // the ThinningCache later on by the ThinningCacheTool
                    xAOD::AuxCompression compression;
                    compression.setCompressedAuxIDs( comp_attr_map );
                    for( const auto& it : compression.getCompressedAuxIDs( allVars ) ) {
                      if( it.second.size() > 0 ) { // insert only if the set is non-empty
                        compInfo[ key ][ it.first ] = it.second;
                        ATH_MSG_DEBUG(std::format("Container {} has {} variables that'll be "
                                      "lossy float compressed with {} mantissa bits",
                                      key, it.second.size(), it.first));
                      }
                    } // End of loop over variables to be lossy float compressed
                  } // End of lossy float compression logic

               }

               added = true;
               if (m_itemSvc->addStreamItem(this->name(),tns.str()).isFailure()) {
                  ATH_MSG_WARNING(std::format("Unable to record item {} in Svc", tns.str()));
               }
            }
         } else if (keyMatch && xkeyMatch) {
            removed = true;
         }
      } // proxy loop
      if (!added && !removed) {
         ATH_MSG_DEBUG(std::format(" No object matching {},\"{}\" found", item_id, item_key));
      } else if (removed) {
         ATH_MSG_DEBUG(std::format(" Object being excluded based on property setting {},\"{}\". Skipping",
                                   item_id, item_key));
      }
   } else {
      ATH_MSG_DEBUG(std::format(" Failed to receive proxy iterators from StoreGate for {},\"{}\". Skipping",
                                item_id, item_key));
   }
   return StatusCode::SUCCESS;
}

/// Here we build the list of attributes for the float compression
/// CompressionList follows the same logic as the ItemList
/// We find the matching keys, read the string after "Aux.",
/// tokenize by "." and build an std::set of these to be
/// communicated to ThinningInfo elsewhere in the code.
std::set<std::string>
AthenaOutputStream::buildCompressionSet (const ToolHandle<SG::IFolder>& handle,
                                         const CLID& item_id,
                                         const std::string& item_key) const
{
  // Create an empty result
  std::set<std::string> result;

  // Check the item is indeed Aux.
  if(item_key.find("Aux.") == std::string::npos) {
    return result;
  }

  // First the high compression list
  for (const auto& iter : *handle) {
    // First match the IDs for early rejection.
    if (iter.id() != item_id) {
      continue;
    }
    // Then find the compression item key and the compression list string
    size_t seppos = iter.key().find('.');
    std::string comp_item_key{""}, comp_str{""};
    if(seppos != std::string::npos) {
      comp_item_key = iter.key().substr(0, seppos+1);
      comp_str = iter.key().substr(seppos+1);
    } else {
      comp_item_key = iter.key();
    }
    // Proceed only if the keys match and the
    // compression list string is not empty
    if (!comp_str.empty() && comp_item_key == item_key) {
      std::stringstream ss(comp_str);
      std::string attr;
      while( std::getline(ss, attr, '.') ) {
         result.insert(attr);
      }
    }
  }

  // All done, return the result
  return result;
}

/// Here we build the vetoed AuxIDs
void AthenaOutputStream::handleVariableSelection (const SG::IConstAuxStore& auxstore,
                                                  SG::DataProxy& itemProxy,
                                                  const std::string& tns,
                                                  const std::string& aux_attr,
                                                  SG::SelectionVetoes& vetoes) const
{
  // Collect dynamic Aux selection (parse the line, attributes separated by dot)
  std::set<std::string> attributes;
  if( aux_attr.size() ) {
    std::stringstream ss(aux_attr);
    std::string attr;
    while( std::getline(ss, attr, '.') ) {
      attributes.insert(attr);
      std::stringstream temp;
      temp << tns << attr;
      if (m_itemSvc->addStreamItem(this->name(),temp.str()).isFailure()) {
        ATH_MSG_WARNING(std::format("Unable to record item {} in Svc", temp.str()));
      }
    }
  }

  // Return early if there's no selection.
  if (attributes.empty()) {
    return;
  }

  std::string key = itemProxy.name();
  if (key.size() >= 4 && key.compare (key.size()-4, 4, "Aux.")==0)
  {
    key.erase (key.size()-4, 4);
  }

  // Find the entry for the selection.
  SG::auxid_set_t& vset = vetoes[key];

  // Form the veto mask for this object.
  xAOD::AuxSelection sel;
  sel.selectAux (attributes);

  // Get all the AuxIDs that we know of and the selected ones
  SG::auxid_set_t all = auxstore.getAuxIDs();
  SG::auxid_set_t selected = sel.getSelectedAuxIDs( all );

  // Loop over all and build a list of vetoed AuxIDs from non selected ones
  for( const SG::auxid_t auxid : all ) {
    if ( !selected.test( auxid ) ) {
      vset.insert( auxid );
    }
  }
}

// Here comes the list handlers...
void AthenaOutputStream::itemListHandler(Gaudi::Details::PropertyBase& /* theProp */) {
   // Assuming concrete SG::Folder also has an itemList property
   IProperty *pAsIProp(nullptr);
   if ((m_p2BWritten.retrieve()).isFailure() ||
           nullptr == (pAsIProp = dynamic_cast<IProperty*>(&*m_p2BWritten)) ||
           (pAsIProp->setProperty(m_itemList)).isFailure()) {
      throw GaudiException("Folder property [itemList] not found", name(), StatusCode::FAILURE);
   }
}

void AthenaOutputStream::compressionListHandlerHigh(Gaudi::Details::PropertyBase& /* theProp */) {
   IProperty *pAsIProp(nullptr);
   if ((m_compressionDecoderHigh.retrieve()).isFailure() ||
           nullptr == (pAsIProp = dynamic_cast<IProperty*>(&*m_compressionDecoderHigh)) ||
           (pAsIProp->setProperty("ItemList", m_compressionListHigh.toString())).isFailure()) {
      throw GaudiException("Folder property [ItemList] not found", name(), StatusCode::FAILURE);
   }
}

void AthenaOutputStream::compressionListHandlerLow(Gaudi::Details::PropertyBase& /* theProp */) {
   IProperty *pAsIProp(nullptr);
   if ((m_compressionDecoderLow.retrieve()).isFailure() ||
           nullptr == (pAsIProp = dynamic_cast<IProperty*>(&*m_compressionDecoderLow)) ||
           (pAsIProp->setProperty("ItemList", m_compressionListLow.toString())).isFailure()) {
      throw GaudiException("Folder property [ItemList] not found", name(), StatusCode::FAILURE);
   }
}

// Reinitialize the internal state of the algorithm
// for I/O purposes (e.g. upon @c fork(2))
StatusCode AthenaOutputStream::io_reinit() {
   ATH_MSG_INFO("I/O reinitialization...");
   m_incidentSvc->removeListener(this, "MetaDataStop"); // Remove any existing listener to avoid handling the incident multiple times
   m_incidentSvc->addListener(this, "MetaDataStop", 50);
   for (auto& tool : m_helperTools) {
      if (!tool->postInitialize().isSuccess()) {
          ATH_MSG_ERROR("Cannot initialize helper tool");
      }
   }
   return StatusCode::SUCCESS;
}

// Finalize the internal state of the algorithm
// for I/O purposes (e.g. upon @c fork(2))
StatusCode AthenaOutputStream::io_finalize() {
   ATH_MSG_INFO("I/O finalization...");
   for (auto& tool : m_helperTools) {
      if (!tool->preFinalize().isSuccess()) {
          ATH_MSG_ERROR("Cannot finalize helper tool");
      }
   }
   const Incident metaDataStopIncident(name(), "MetaDataStop");
   this->handle(metaDataStopIncident);
   m_incidentSvc->removeListener(this, "MetaDataStop");
   if (m_dataStore->clearStore().isFailure()) {
      ATH_MSG_WARNING("Cannot clear the DataStore");
   }
   return StatusCode::SUCCESS;
}

/// Helper function to load dictionaries (both transient and persistent)
/// for a given type.
/// We want to to this explicitly during initialization to avoid sporadic
/// failures seen loading dictionaries while multiple threads are running.
/// See ATEAM-697 and ATEAM-749.
void AthenaOutputStream::loadDict (CLID clid)
{
  m_dictLoader->load_type (clid);

  // Also load the persistent class dictionary, if applicable.
  std::unique_ptr<ITPCnvBase> tpcnv = m_tpCnvSvc->t2p_cnv_unique (clid);
  if (tpcnv) {
    m_dictLoader->load_type (tpcnv->persistentTInfo());
  }
}
