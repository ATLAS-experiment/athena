/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// IOVDbSvc.cxx
// Re-implementation of Athena IOVDbSvc
// Richard Hawkijngs, started 23/11/08
// based on earlier code by RD Schaffer, Antoine Perus and RH

#include "IOVDbParser.h"
#include "IOVDbSvc.h"
#include "CoralCrestManager.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IProperty.h"

#include "AthenaKernel/IOVRange.h"
#include "CxxUtils/checker_macros.h"
#include "DBLock/DBLock.h"
#include "EventInfoUtils/EventIDFromStore.h"
#include "IOVDbDataModel/IOVMetaDataContainer.h"
#include "StoreGate/StoreClearedIncident.h"

#include <algorithm>
#include <list>
#include <ranges>
#include <utility>

#include "CrestApi/CrestLogger.h"

namespace {

// Wrap a cool IDatabase with a DBLock.
class LockedDatabase
  : public cool::IDatabase
{
public:
  LockedDatabase (cool::IDatabasePtr dbptr,
                  const Athena::DBLock& dblock)
    : m_dbptr (std::move(dbptr)), m_dblock (dblock) {}

  virtual ~LockedDatabase() = default;

  virtual const cool::DatabaseId& databaseId() const override
  { return m_dbptr->databaseId(); }

  virtual const cool::IRecord& databaseAttributes() const override
  { return m_dbptr->databaseAttributes(); }

  virtual cool::IFolderSetPtr createFolderSet
  ( const std::string& fullPath,
    const std::string& description = "",
    bool createParents = false ) override
  { return m_dbptr->createFolderSet (fullPath, description, createParents); }

  virtual bool existsFolderSet( const std::string& folderSetName ) override
  { return m_dbptr->existsFolderSet (folderSetName); }

  virtual cool::IFolderSetPtr getFolderSet( const std::string& fullPath ) override
  { return m_dbptr->getFolderSet (fullPath); }

  virtual cool::IFolderPtr createFolder
  ( const std::string& fullPath,
    const cool::IFolderSpecification& folderSpec,
    const std::string& description = "",
    bool createParents = false ) override
  { return m_dbptr->createFolder (fullPath, folderSpec, description, createParents); }

  virtual bool existsFolder( const std::string& fullPath ) override
  { return m_dbptr->existsFolder (fullPath); }

  virtual cool::IFolderPtr getFolder( const std::string& fullPath ) override
  { return m_dbptr->getFolder (fullPath); }

  virtual const std::vector<std::string> listAllNodes( bool ascending = true ) override
  { return m_dbptr->listAllNodes (ascending); }

  virtual bool dropNode( const std::string& fullPath ) override
  { return m_dbptr->dropNode (fullPath); }

  virtual bool existsTag( const std::string& tagName ) const override
  { return m_dbptr->existsTag (tagName); }

  virtual cool::IHvsNode::Type tagNameScope( const std::string& tagName ) const override
  { return m_dbptr->tagNameScope (tagName); }

  virtual const std::vector<std::string>
  taggedNodes( const std::string& tagName ) const override
  { return m_dbptr->taggedNodes (tagName); }

  virtual bool isOpen() const override
  { return m_dbptr->isOpen(); }

  virtual void openDatabase() override
  { return m_dbptr->openDatabase(); }

  virtual void closeDatabase() override
  { return m_dbptr->closeDatabase(); }
    
  virtual const std::string& databaseName() const override
  { return m_dbptr->databaseName(); }

#ifdef COOL400TX
  /// Start a new transaction and enter manual transaction mode
  virtual ITransactionPtr startTransaction() override
  { return m_dbptr->startTransaction(); }
#endif


private:
  cool::IDatabasePtr m_dbptr;
  Athena::DBLock m_dblock;
};

} // anonymous namespace


int IOVDbSvc::poolSvcContext()
{
   if( m_poolSvcContext < 0 ) {
      // Get context for POOL conditions files, and created an initial connection
      if (m_par_managePoolConnections) {
         m_poolSvcContext=m_h_poolSvc->getInputContext("Conditions", m_par_maxNumPoolFiles);
      } else {
         m_poolSvcContext=m_h_poolSvc->getInputContext("Conditions");
      }
      if( m_h_poolSvc->connect(Io::READ, m_poolSvcContext).isSuccess() ) {
         ATH_MSG_INFO( "Opened read transaction for POOL PersistencySvc");
      } else {
         // We only emit info for failure to connect (for the moment? RDS 01/2008)
         ATH_MSG_INFO( "Cannot connect to POOL PersistencySvc" );
      }
   }
   return m_poolSvcContext;
}


StatusCode IOVDbSvc::initialize() {
  // subscribe to events
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc",name());
  ATH_CHECK( incSvc.retrieve() );
  const long int pri = 100;
  incSvc->addListener( this, IncidentType::BeginEvent, pri );
  incSvc->addListener( this, "StoreCleared", pri );   // for SP Athena
  incSvc->addListener( this, IncidentType::EndProcessing, pri );  // for MT Athena

  // Register this service for 'I/O' events
  ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
  ATH_CHECK( iomgr.retrieve() );
  ATH_CHECK( iomgr->io_register(this) );

  // print warnings/info depending on state of job options
  if (!m_par_manageConnections)
    ATH_MSG_INFO( "COOL connection management disabled - connections kept open throughout job" );
  if (!m_par_managePoolConnections)
    ATH_MSG_INFO( "POOL file connection management disabled - files kept open throught job" );
  if (m_par_maxNumPoolFiles > 0)
    ATH_MSG_INFO( "Only " << m_par_maxNumPoolFiles.value() <<  " POOL conditions files will be open at once" );
  if (m_par_forceRunNumber > 0 || m_par_forceLumiblockNumber > 0)
    ATH_MSG_WARNING( "Global run/LB number forced to be [" <<
                     m_par_forceRunNumber.value() << "," << m_par_forceLumiblockNumber.value() <<  "]" );
  if (m_par_forceTimestamp > 0)
    ATH_MSG_WARNING( "Global timestamp forced to be " << m_par_forceTimestamp.value() );
  if (m_par_cacheRun > 0)
    ATH_MSG_INFO( "Run-LB data will be cached in groups of " << m_par_cacheRun.value() << " runs" );
  if (m_par_cacheTime > 0)
    ATH_MSG_INFO( "Timestamp data will be cached in groups of " << m_par_cacheTime.value() << " seconds" );
  if (m_par_cacheAlign > 0) 
    ATH_MSG_INFO( "Cache alignment will be done in " << m_par_cacheAlign.value() << " slices" );
  if (m_par_onlineMode) 
    ATH_MSG_INFO(  "Online mode ignoring potential missing channels outside cache" );
  if (m_par_checklock)
    ATH_MSG_INFO( "Tags will be required to be locked");

  // make sure iovTime is undefined
  m_iovTime.reset();

  // extract information from EventSelector for run/LB/time overrides
  ATH_CHECK( checkEventSel() );

  // initialise default connection
  if (!m_par_defaultConnection.empty()) {
    // default connection is readonly if no : in name (i.e. logical conn)
    bool readonly=(m_par_defaultConnection.find(':')==std::string::npos);
    m_connections.push_back(std::make_unique<IOVDbConn>(m_par_defaultConnection,readonly,msg()));
  }

  // set time of timestampslop in nanoseconds
  m_iovslop=static_cast<cool::ValidityKey>(m_par_timeStampSlop*1.E9);

  // check for global tag in jobopt, which will override anything in input file
  if (!m_par_globalTag.empty()) {
    m_globalTag=m_par_globalTag;
    ATH_MSG_INFO( "Global tag: " << m_par_globalTag.value() << " set from joboptions" );
  }

  // setup folders and process tag overrides
  ATH_CHECK( setupFolders() );

  // Set state to initialize
  m_state=IOVDbSvc::INITIALIZATION;
  ATH_MSG_INFO( "Initialised with " << m_connections.size() << 
                " connections and " << m_foldermap.size() << " folders" );

  if (m_outputToFile)    ATH_MSG_INFO("Db dump to file activated");
  if (m_crestCoolToFile) ATH_MSG_INFO("Crest or Cool dump to file activated");

  ATH_MSG_INFO( "Service IOVDbSvc initialised successfully" );

  ATH_CHECK( checkConfigConsistency() );
  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::io_reinit() {
   ATH_MSG_DEBUG("I/O reinitialization...");
   // PoolSvc clears all connections on IO_reinit - forget the stored contextId
   m_poolSvcContext = -1;
   return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::io_finalize() {
   ATH_MSG_DEBUG("I/O finalization...");
   return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::finalize() {
  // summarise and delete folders, adding total read from COOL
  unsigned long long nread = 0;
  float readtime = 0;

  // accumulate a map of readtime by connection
  std::map<IOVDbConn*, float> ctmap;
  for (const auto & [name, folder] : m_foldermap) {
    folder->summary();
    nread += folder->bytesRead();
    readtime += folder->readTime();
    const auto& [citr, inserted] = ctmap.try_emplace(folder->conn(), 0);
    citr->second += folder->readTime();
  }
  m_foldermap.clear();
  ATH_MSG_INFO(  "Total payload read from IOVDb: " << nread << " bytes in (( " << std::fixed << std::setw(9) << std::setprecision(2) <<
    readtime << " ))s" );

  // close and delete connections, printing time in each one
  for (const auto & conn : m_connections) {
    float fread = 0;
    const auto citr = ctmap.find(conn.get());
    if (citr != ctmap.end()) fread = citr->second;
    conn->setInactive();
    conn->summary(fread);
  }
  m_connections.clear();

  return StatusCode::SUCCESS;
}


cool::IDatabasePtr IOVDbSvc::getDatabase(bool readOnly) {
  // get default database connection
  cool::IDatabasePtr dbconn;
  if (m_par_defaultConnection.empty() || m_connections.empty()) {
    ATH_MSG_INFO( "No default COOL database connection is available");
    dbconn.reset();
  } else {
    Athena::DBLock dblock;
    if (m_connections[0]->isReadOnly()!=readOnly) {
      ATH_MSG_INFO("Changing state of default connection to readonly=" << readOnly );
      m_connections[0]->setReadOnly(readOnly);
    }
    dbconn = std::make_shared<LockedDatabase> (m_connections[0]->getCoolDb(),
                                               dblock);
  }
  return dbconn;
}


StatusCode IOVDbSvc::preLoadAddresses(StoreID::type storeID,tadList& tlist) {
  // Read information for folders and setup TADs
  if (storeID!=StoreID::DETECTOR_STORE) return StatusCode::SUCCESS;
  // Preloading of addresses should be done ONLY for detector store
  ATH_MSG_DEBUG( "preLoadAddress: storeID -> " << storeID );

  Athena::DBLock dblock;

  // check File Level Meta Data of input, see if any requested folders are available there
  SG::ConstIterator<IOVMetaDataContainer> cont;
  SG::ConstIterator<IOVMetaDataContainer> contEnd;
  if (m_h_metaDataStore->retrieve(cont,contEnd).isSuccess()) {
    unsigned int ncontainers=0;
    unsigned int nused=0;
    for (;cont!=contEnd; ++cont) {
      ++ncontainers;
      const std::string& fname=cont->folderName();
      // check if this folder is in list requested by IOVDbSvc
      for (const auto & [name, folder] : m_foldermap) {
        // take data from FLMD only if tag override is NOT set
        if (folder->folderName()==fname && !(folder->tagOverride())) {
          ATH_MSG_INFO( "Folder " << fname << " will be taken from file metadata" );
          folder->useFileMetaData();
          folder->setFolderDescription( cont->folderDescription() );
          ++nused;
          break;
        }
      }
    }
    ATH_MSG_INFO( "Found " << ncontainers <<  " metadata containers in input file, " << nused << " will be used");
  } else {
    ATH_MSG_DEBUG( "Could not retrieve IOVMetaDataContainer objects from MetaDataStore" );
  }

  // Remove folders which should only be read from file meta data, but
  // were not found in the MetaDataStore

  // Note: we cannot iterate and perform erase within the iteration
  // because the iterator becomes invalid. So first collect the keys
  // to erase in a first pass and then erase them.
  std::vector<std::string> keysToDelete;
  for (const auto & [name, folder] : m_foldermap)  {
    if (folder->fromMetaDataOnly() && !folder->readMeta()) {
      ATH_MSG_INFO( "preLoadAddresses: Removing folder " << folder->folderName() <<
        ". It should only be in the file meta data and was not found." );
      keysToDelete.push_back(name);
    }
  }
  
  for (auto & thisKey : keysToDelete) {
    const auto fitr = m_foldermap.find(thisKey);
    if (fitr != m_foldermap.end()) {
      fitr->second->conn()->decUsage();
      m_foldermap.erase(fitr);
    } else {
      ATH_MSG_ERROR( "preLoadAddresses: Could not find folder " << thisKey << " for removal" );
    }
  }
  

  // loop over all folders, grouped by connection
  // do metadata folders on first connection (default connection)
  bool doMeta=true;
  // do not close COOL connection until next one has been opened, this enables
  // connection sharing in CORAL, so all COOL connections will use the same
  // CORAL one (althugh they will each be given a separate session)
  IOVDbConn* oldconn=nullptr;
  for (const auto & pThisConnection : m_connections) {
    if (pThisConnection->nFolders()>0 || doMeta) {
      // loop over all folders using this connection
      for (const auto & [name, folder] : m_foldermap) {
        if (folder->conn()==pThisConnection.get() || (folder->conn()==nullptr && doMeta)) {
          std::unique_ptr<SG::TransientAddress> tad =
            folder->preLoadFolder( &(*m_h_tagInfoMgr), m_par_cacheRun, m_par_cacheTime);
          if (oldconn!=pThisConnection.get()) {
            // close old connection if appropriate
            if (m_par_manageConnections && oldconn!=nullptr) oldconn->setInactive();
            oldconn=pThisConnection.get();
          }
          if (tad==nullptr) {
            ATH_MSG_ERROR( "preLoadFolder failed for folder " << folder->folderName() );
            return StatusCode::FAILURE;
          }
          // for write-metadata folder, request data preload
          if (folder->writeMeta()) {
            if (m_h_IOVSvc->preLoadDataTAD(tad.get(),folder->eventStore()).isFailure()) {
              ATH_MSG_ERROR( "Could not request IOVSvc to preload metadata for " << folder->folderName() );
              return StatusCode::FAILURE;
            }
          } else {
            // for other folders, just preload TAD (not data)
            if (m_h_IOVSvc->preLoadTAD(tad.get(), folder->eventStore()).isFailure()) {
              ATH_MSG_ERROR( "Could not request IOVSvc to preload metadata for " << folder->folderName() );
              return StatusCode::FAILURE;
            }
          }
          // Add TAD to Storegate
          tlist.push_back(tad.release());
          // check for IOV override
          folder->setIOVOverride(m_par_forceRunNumber, m_par_forceLumiblockNumber, m_par_forceTimestamp);
        }
      }
    }
    doMeta=false;
  }
  // close last connection
  if (oldconn!=nullptr and m_par_manageConnections) oldconn->setInactive();

  // some folder keys may have changed during preloadFolder due to use of
  // <key> specification in folder description string
  // build a new foldermap with the updated keys
  FolderMap newmap;
  for (auto & [name, folder] : m_foldermap) {
    newmap[folder->key()]=std::move(folder);
  }
  m_foldermap=std::move(newmap);

  // fill global and explicit folder tags into TagInfo
  if (fillTagInfo().isFailure())
    ATH_MSG_ERROR("Could not fill TagInfo object from preLoadAddresses" );

  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::loadAddresses(StoreID::type /*storeID*/, tadList& /*list*/ ) {
  // this method does nothing
  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::updateAddress(StoreID::type storeID, SG::TransientAddress* tad,
                                   const EventContext& /*ctx*/)
{
  // Provide TAD and associated range, actually reading the conditions data

  // Read information for folders and setup TADs
  if (storeID!=StoreID::DETECTOR_STORE) return StatusCode::FAILURE;
  Gaudi::Guards::AuditorGuard auditor(std::string("UpdateAddr::")+(tad->name().empty() ? "anonymous" : tad->name()),
                                      auditorSvc(), "preLoadProxy");

  IOVTime iovTime{m_iovTime};
  IOVRange range;
  std::unique_ptr<IOpaqueAddress> address;

  // first check if this key is managed by IOVDbSvc
  // return FAILURE if not - this allows other AddressProviders to be 
  // asked for the TAD
  const std::string& key=tad->name();
  const auto fitr=m_foldermap.find(key);
  if (fitr==m_foldermap.end()) {
    ATH_MSG_VERBOSE( 
        "updateAddress cannot find description for TAD " << key );
    return StatusCode::FAILURE;
  }
  const auto& folder=fitr->second;
  if (folder->clid()!=tad->clID()) {
    ATH_MSG_VERBOSE( "CLID for TAD " << key << " is " << tad->clID()
             << " but expecting " << folder->clid() );
    
    return StatusCode::FAILURE;
  }

  // IOVDbSvc will satisfy the request, using already found folder
  // now determine the current IOVTime
  if (m_state==IOVDbSvc::INITIALIZATION && !m_iovTime.isValid()) {
    ATH_MSG_DEBUG( "updateAddress: in initialisation phase and no iovTime defined" );
    return::StatusCode::SUCCESS;
  }
  if (m_state==IOVDbSvc::EVENT_LOOP) {
    // determine iovTime from eventID in the event context
    const EventIDBase* evid = EventIDFromStore( m_h_sgSvc );
    if( evid ) {
      iovTime.setRunEvent( evid->run_number(), evid->lumi_block()) ;
      // save both seconds and ns offset for timestamp
      uint64_t nsTime = evid->time_stamp() *1000000000LL;
      nsTime += evid->time_stamp_ns_offset();
      iovTime.setTimestamp(nsTime);
      m_iovTime = iovTime;
      ATH_MSG_DEBUG( "updateAddress - using iovTime from EventInfo: " << iovTime);
    } else {
      // failed to get event info - just return success
      ATH_MSG_DEBUG( "Could not get event - initialise phase");
      return StatusCode::SUCCESS;
    }
  } else {
    ATH_MSG_DEBUG("updateAddress: using iovTime from init/beginRun: " << iovTime);
  }



  // obtain the validity key for this folder (includes overrides)
  cool::ValidityKey vkey=folder->iovTime(iovTime);
  {
     // The dblock is currently abused to also protect the cache in the IOVDbFolders.
     // This global lock may give rise to deadlocks between the dblock and the internal lock
     // of the SGImplSvc. The deadlock may arise if the order of the initial call to IOVDbSvc
     // and SGImplSvc are different, because the two services call each other.
     // A problem was observed when SG::DataProxy::isValidAddress first called IOVDbSvc::updateAddress
     // which called IOVSvc::setRange then SGImplSvc::proxy, and at the same time
     // StoreGateSvc::contains called first SGImplSvc::proxy which then called IOVDbSvc::updateAddress.
     // This problem is mitigated by limiting the scope of the dblock here.
     Athena::DBLock dblock;
     ATH_MSG_DEBUG("Validity key "<<vkey);
     if (folder->source() == "CREST") {
        if (!folder->readMeta() && !folder->cacheValid((vkey))){
          fitr->second->loadCache(vkey, m_par_cacheAlign,m_globalTag,m_par_onlineMode);
      }
    } else { //COOL reading 
     if (!folder->readMeta() && !folder->cacheValid(vkey)) {
        // mark this folder as not-dropped so cache-read will succeed
        folder->setDropped(false);
        // reload cache for this folder (and all others sharing this DB connection)
        ATH_MSG_DEBUG( "Triggering cache load for folder " << folder->folderName());
        if (loadCaches(folder->conn()).isFailure()) {
           ATH_MSG_ERROR( "Cache load failed for at least one folder from " << folder->conn()->name()
                          << ". You may see errors from other folders sharing the same connection." );
           return StatusCode::FAILURE;
        }
     }
    }//end cool part 
     // data should now be in cache
     // setup address and range
     {
        Gaudi::Guards::AuditorGuard auditor(std::string("FldrSetup:")+(tad->name().empty() ? "anonymous" : tad->name()),
                                            auditorSvc(), "preLoadProxy");
        if (!folder->getAddress(vkey,&(*m_h_persSvc),poolSvcContext(),address,
                                range,m_poolPayloadRequested)) {
           ATH_MSG_ERROR( "getAddress failed for folder " << folder->folderName() );
           return StatusCode::FAILURE;
        }
     }
     // reduce minimum IOV of timestamp folders to avoid 'thrashing'
     // due to events slightly out of order in HLT
     if (folder->timeStamp()) {
        cool::ValidityKey start=range.start().timestamp();
        if (start>m_iovslop) start-=m_iovslop;
        range=IOVRange(IOVTime(start),range.stop());
     }
  }

  // Pass range onto IOVSvc
  if (m_h_IOVSvc->setRange(tad->clID(),tad->name(),
                           range,folder->eventStore()).isFailure()) {
    ATH_MSG_ERROR( "setRange failed for folder " << folder->folderName() );
    return StatusCode::FAILURE;
  }
  tad->setAddress(address.release());
  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::getRange( const CLID&        clid, 
                               const std::string& dbKey,
                               const IOVTime&     time,
                               IOVRange&          range,
                               std::string&       tag,
                               std::unique_ptr<IOpaqueAddress>&   address) {

  Athena::DBLock dblock;

  ATH_MSG_DEBUG( "getRange  clid: " << clid << " key: \""<< dbKey << "\"  t: " << time );

  const auto fitr = m_foldermap.find(dbKey);
  if (fitr==m_foldermap.end()) {
    ATH_MSG_VERBOSE("getRange cannot find description for dbKey " << dbKey );
    return StatusCode::FAILURE;
  }
  const auto& folder = fitr->second;
  if (folder->clid()!=clid) {
    ATH_MSG_VERBOSE( "supplied CLID for " << dbKey << " is "
             << clid
             << " but expecting " << folder->clid() );
    
    return StatusCode::FAILURE;
  }

  tag = folder->key();

  // obtain the validity key for this folder (includes overrides)
  cool::ValidityKey vkey = folder->iovTime(time);
  if (folder->source() == "CREST") {
      if (!folder->readMeta() && !folder->cacheValid((vkey))){
        fitr->second->loadCache(vkey, m_par_cacheAlign,m_globalTag,m_par_onlineMode);
      }
  } else {
    if (!folder->readMeta() && !folder->cacheValid(vkey)) {
      // mark this folder as not-dropped so cache-read will succeed
      folder->setDropped(false);
      // reload cache for this folder (and all others sharing this DB
      // connection)
      ATH_MSG_DEBUG("Triggering cache load for folder " << folder->folderName());
      if (loadCaches(folder->conn(), &time).isFailure()) {
        ATH_MSG_ERROR("Cache load failed for at least one folder from " << folder->conn()->name()
                                                                        << ". You may see errors from other folders sharing the "
                                                                           "same connection.");
        return StatusCode::FAILURE;
      }
    }
  }
  // data should now be in cache
  address.reset();
  // setup address and range
  {
    Gaudi::Guards::AuditorGuard auditor(std::string("FldrSetup:")+(dbKey.empty() ? "anonymous" : dbKey),
                                        auditorSvc(), "preLoadProxy");
    if (!folder->getAddress(vkey,&(*m_h_persSvc),poolSvcContext(),address,
                            range,m_poolPayloadRequested)) {
      ATH_MSG_ERROR("getAddress failed for folder " <<folder->folderName() );
      return StatusCode::FAILURE;
    }
  }

  // Special handling for extensible folders:
  if (folder->extensible()) {
    // Set the end time to just past the current event or lumiblock.
    IOVTime extStop = range.stop();
    if (folder->timeStamp()) {
      extStop.setTimestamp (time.timestamp() + 1);
    }
    else {
      extStop.setRETime (time.re_time() + 1);
    }
    range = IOVRange (range.start(), extStop);
  }

  // Special handling for IOV override: set the infinite validity range
  if (folder->iovOverridden()) {
    if (folder->timeStamp()) {
      range = IOVRange ( IOVTime(IOVTime::MINTIMESTAMP),
                         IOVTime(IOVTime::MAXTIMESTAMP) );
    }
    else {
      range = IOVRange ( IOVTime(IOVTime::MINRUN,IOVTime::MINEVENT),
                         IOVTime(IOVTime::MAXRUN,IOVTime::MAXEVENT) );
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::setRange( const CLID&        /*clid*/,
                               const std::string& /*dbKey*/,
                               const IOVRange&    /*range*/,
                               const std::string& /*storeName*/ ) {
  // this method does nothing
  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::signalBeginRun(const IOVTime& beginRunTime,
                                    const EventContext& ctx)
{
  Athena::DBLock dblock;

  // Begin run - set state and save time for later use
  m_state=IOVDbSvc::BEGIN_RUN;
  // Is this a different run compared to the previous call?
  bool newRun = m_iovTime.isValid() && (m_iovTime.run() != beginRunTime.run());
  m_iovTime=beginRunTime;
  // For a MC event, the run number we need to use to look up the conditions
  // may be different from that of the event itself.  Override the run
  // number with the conditions run number from the event context,
  // if it is defined.
  EventIDBase::number_type conditionsRun =
    Atlas::getExtendedEventContext(ctx).conditionsRun();
  if (conditionsRun != EventIDBase::UNDEFNUM) {
    m_iovTime.setRunEvent (conditionsRun, m_iovTime.event());
  }

  ATH_MSG_DEBUG( "signalBeginRun> begin run time " << m_iovTime);
  if (!m_par_onlineMode) {
    return StatusCode::SUCCESS;
  }

  // ONLINE mode: allow adding of new calibration constants between runs
  if (!newRun) {
    ATH_MSG_DEBUG( "Same run as previous signalBeginRun call. Skipping re-loading of folders..." );
    return StatusCode::SUCCESS;
  }

  // all other stuff is event based so happens after this.
  // this is before first event of each run
  ATH_MSG_DEBUG( "In online mode will recheck ... " );
  ATH_MSG_DEBUG( "First reload PoolCataloge ... " );
  m_h_poolSvc->startCatalog();
  m_h_poolSvc->commitCatalog();
  static const std::string preLoadProxyStr{"preLoadProxy"};
  for (const auto & pThisConnection : m_connections){
    // only access connections which are actually in use - avoids waking up
    // the default DB connection if it is not being used
    if (pThisConnection->nFolders()>0) {
      //request for database activates connection
      cool::IDatabasePtr dbconn=pThisConnection->getCoolDb();
      if (dbconn.get()==nullptr) {
        ATH_MSG_FATAL( "Conditions database connection " <<  pThisConnection->name() << " cannot be opened - STOP" );
        return StatusCode::FAILURE;
      }
      for (const auto & [name, folder]: m_foldermap) {
        if (folder->conn()!=pThisConnection.get()) continue;
        folder->printCache();
        cool::ValidityKey vkey=folder->iovTime(m_iovTime);
        {
          Gaudi::Guards::AuditorGuard auditor(std::string("FldrCache:")+folder->folderName(), auditorSvc(), preLoadProxyStr);
          if (!folder->loadCacheIfDbChanged(vkey, m_globalTag, dbconn, m_h_IOVSvc)) {
            ATH_MSG_ERROR( "Problem RELOADING: " << folder->folderName());
            return StatusCode::FAILURE;
          }
        }
        folder->printCache();
      }
    }
    if (m_par_manageConnections) pThisConnection->setInactive();
  }
  return StatusCode::SUCCESS;
}


void IOVDbSvc::signalEndProxyPreload() {
  // this method does nothing
}


void IOVDbSvc::postConditionsLoad() {
   // Close any open POOL files after loding Conditions
   ATH_MSG_DEBUG( "postConditionsLoad: m_par_managePoolConnections=" << m_par_managePoolConnections
                  << "  m_poolPayloadRequested=" << m_poolPayloadRequested );

   if (m_par_managePoolConnections && m_poolPayloadRequested) {
      // reset POOL connection to close all open conditions POOL files
      m_par_managePoolConnections.set(false);
      m_poolPayloadRequested=false;
      if( m_poolSvcContext ) {
         if (m_h_poolSvc->disconnect(m_poolSvcContext).isSuccess()) {
            ATH_MSG_DEBUG( "Successfully closed input POOL connections");
         } else {
            ATH_MSG_WARNING( "Unable to close input POOL connections" );
         }
         // reopen transaction
         if (m_h_poolSvc->connect(Io::READ, m_poolSvcContext).isSuccess()) {
            ATH_MSG_DEBUG("Reopend read transaction for POOL conditions input files" );
         } else {
            ATH_MSG_WARNING("Cannot reopen read transaction for POOL conditions input files");
         }
      }
   }
}


void IOVDbSvc::handle( const Incident& inc) {
  // Handle incidents:
  // BeginEvent to set IOVDbSvc state to EVENT_LOOP
  // StoreCleared/EndProcessing to close any open POOL files
  ATH_MSG_VERBOSE( "entering handle(), incident type " << inc.type() << " from " << inc.source() );
  if (inc.type()=="BeginEvent") {
    m_state=IOVDbSvc::EVENT_LOOP;
  } else {
    Athena::DBLock dblock;

    const StoreClearedIncident* sinc = dynamic_cast<const StoreClearedIncident*>(&inc);
    if( (inc.type()=="StoreCleared" && sinc!=nullptr && sinc->store()==&*m_h_sgSvc
         && m_state>=IOVDbSvc::EVENT_LOOP)
        or inc.type()==IncidentType::EndProcessing )
    {
       m_state=IOVDbSvc::FINALIZE_ALG;
       postConditionsLoad();
    }
  }
}


StatusCode IOVDbSvc::processTagInfo() {
  // Processing of taginfo
  // Set GlobalTag and any folder-specific overrides if given

  // dump out contents of TagInfo
  ATH_MSG_DEBUG( "Tags from input TagInfo:");
  if( msg().level()>=MSG::DEBUG ) m_h_tagInfoMgr->printTags(msg());
  
  // check IOVDbSvc GlobalTag, if not already set
  if (m_globalTag.empty()) {
    m_globalTag = m_h_tagInfoMgr->findTag("IOVDbGlobalTag");
    if (!m_globalTag.empty()) ATH_MSG_INFO( "Global tag: " << m_globalTag<< " set from input file" );
    ATH_CHECK( checkConfigConsistency() );
  }

  // now check for tag overrides for specific folders
  const ITagInfoMgr::NameTagPairVec& nameTagPairs = m_h_tagInfoMgr->getInputTags();
  for (const auto & [theTagName, theTag]: nameTagPairs) {
    // assume tags relating to conditions folders start with /
    if (not theTagName.starts_with('/')) continue;
    // check for folder(s) with this name in (key, ptr) pair
    for (const auto & [name, folder]: m_foldermap) {
      const std::string& ifname=folder->folderName();
      if (ifname!=theTagName) continue;
      // use an override from TagInfo only if there is not an explicit jo tag,
      // and folder meta-data is not used, and there is no <noover/> spec,
      // and no global tag set in job options
      if (folder->joTag().empty() && !folder->readMeta() && !folder->noOverride() && m_par_globalTag.empty()) {
        folder->setTagOverride(theTag,false);
        ATH_MSG_INFO( "TagInfo override for tag " << theTag << " in folder " << ifname );
      } else if (folder->joTag()!=theTag) {
        const std::string_view tagTypeString=(folder->joTag().empty()) ? "hierarchical" : "jobOption";
        ATH_MSG_INFO( "Ignoring inputfile TagInfo request for tag " << theTag << " in folder " << ifname<<" in favour of "<<tagTypeString);
      }
    }
  }
  return StatusCode::SUCCESS;
}


std::vector<std::string> 
IOVDbSvc::getKeyList() {
  // return a list of all the StoreGate keys being managed by IOVDbSvc
  auto keys = std::views::keys(m_foldermap);
  return {keys.begin(), keys.end()};
}


bool IOVDbSvc::getKeyInfo(const std::string& key, IIOVDbSvc::KeyInfo& info) {
  // return information about given SG key
  // first attempt to find the folder object for this key
  const auto itr = m_foldermap.find(key);
  if (itr!=m_foldermap.end()) {
    const IOVDbFolder* f = itr->second.get();
    info.folderName = f->folderName();
    info.tag = f->resolvedTag();
    info.range = f->currentRange();
    info.retrieved = f->retrieved();
    info.bytesRead = f->bytesRead();
    info.readTime = f->readTime();
    info.extensible = f->extensible();
    return true;
  } else {
    info.retrieved = false;
    return false;
  }
}


bool IOVDbSvc::dropObject(const std::string& key, const bool resetCache) {
  // find the folder corresponding to this object
  const auto itr = m_foldermap.find(key);
  if (itr!=m_foldermap.end()) {
    IOVDbFolder* folder=itr->second.get();
    CLID clid=folder->clid();
    SG::DataProxy* proxy=m_h_detStore->proxy(clid,key);
    if (proxy!=nullptr) {
      m_h_detStore->clearProxyPayload(proxy);
      ATH_MSG_DEBUG("Dropped payload for key " << key );
      folder->setDropped(true);
      if (resetCache) {
        folder->resetCache();
        ATH_MSG_DEBUG( "Cache reset done for folder " << folder->folderName() );
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


/************************/
// private methods of IOVDbSvc

StatusCode IOVDbSvc::checkEventSel() {
  // check if EventSelector is being used to override run numbers
  // if so, we can set IOV time already to allow conditons retrieval
  // in the initialise phase, needed for setting up simulation

  ServiceHandle<Gaudi::Interfaces::IOptionsSvc> joSvc("JobOptionsSvc",name());
  ATH_CHECK( joSvc.retrieve() );

  if (!joSvc->has("EventSelector.OverrideRunNumber")) {
    // do not return FAILURE if the EventSelector cannot be found, or it has
    // no override property, can e.g. happen in online running
    ATH_MSG_DEBUG( "No EventSelector.OverrideRunNumber property found" );
    return StatusCode::SUCCESS;
  }

  BooleanProperty overrideRunNumber("OverrideRunNumber",false);
  ATH_CHECK( overrideRunNumber.fromString(joSvc->get("EventSelector.OverrideRunNumber")) );
  if (overrideRunNumber) {
    // if flag is set, extract Run,LB and time
    ATH_MSG_INFO(  "Setting run/LB/time from EventSelector override in initialize" );
    uint32_t run,lumib;
    uint64_t time;
    bool allGood=true;
    if (m_par_forceRunNumber!=0 || m_par_forceLumiblockNumber!=0) {
      ATH_MSG_WARNING( "forceRunNumber property also set" );
    }
    IntegerProperty iprop1("RunNumber",0);
    if (iprop1.fromString(joSvc->get("EventSelector.RunNumber","INVALID"))) {
      run=iprop1;
    } else {
      ATH_MSG_ERROR( "Unable to get RunNumber from EventSelector");
      allGood=false;
    }
    IntegerProperty iprop2("FirstLB",0);
    if (iprop2.fromString(joSvc->get("EventSelector.FirstLB","INVALID"))) {
      lumib=iprop2;
    } else {
      ATH_MSG_ERROR( "Unable to get FirstLB from EventSelector");
      allGood=false;
    }
    IntegerProperty iprop3("InitialTimeStamp",0);
    if (iprop3.fromString(joSvc->get("EventSelector.InitialTimeStamp","INVALID"))) {
      time=iprop3;
    } else {
      ATH_MSG_ERROR("Unable to get InitialTimeStamp from EventSelector" );
      allGood=false;
    }
    if (allGood) {
      m_iovTime.setRunEvent(run,lumib);
      uint64_t nsTime=time*1000000000LL;
      m_iovTime.setTimestamp(nsTime);
      ATH_MSG_INFO( "run/LB/time set to [" << run << "," << lumib << " : " << nsTime << "]" );
    } else {
      ATH_MSG_ERROR( "run/LB/Time NOT changed" );
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::setupFolders() {
  // read the Folders joboptions and setup the folder list
  // no wildcards are allowed

  // getting the pairs: folder name - CREST tag name:
  if (m_par_source == "CREST"){
    auto mLevel = static_cast<std::underlying_type_t<MSG::Level>>(msg().level());
    Crest::LogLevel cLevel = static_cast<Crest::LogLevel>(mLevel);
    Crest::Logger::setLogLevel(cLevel);	  
    m_cresttagmap.clear();
    m_cresttagmap = CoralCrestManager::getGlobalTagMap(m_par_defaultConnection,m_par_globalTag);
    if (m_cresttagmap.empty()) {
      ATH_MSG_FATAL("Got empty tag-map. GlobalTag "<< m_par_globalTag.value() << " does not exist.");
      return StatusCode::FAILURE;
    }
  }
  
  //1. Loop through folders
  std::list<IOVDbParser> allFolderdata;
  for (const auto & thisFolder : m_par_folders) {
    ATH_MSG_DEBUG( "Setup folder " << thisFolder );
    IOVDbParser folderdata(thisFolder,msg());
    if (!folderdata.isValid()) {
      ATH_MSG_FATAL("setupFolders: Folder setup string is invalid: " <<thisFolder);
      return StatusCode::FAILURE;
    }
    
    allFolderdata.push_back(std::move(folderdata));
  }

  //2. Loop through overwrites:
  // syntax for entries is <prefix>folderpath</prefix> <tag>value</tag>
  // folderpath matches from left of folderName
  // but if partial match, next character must be / so override for /Fred/Ji
  // matches /Fred/Ji/A and /Fred/Ji but not /Fred/Jim

  for (const auto & thisOverrideTag : m_par_overrideTags) {
    IOVDbParser keys(thisOverrideTag,msg());
    if (not keys.isValid()){
      ATH_MSG_ERROR("An override tag was invalid: " << thisOverrideTag);
      return StatusCode::FAILURE;
    }
    std::string prefix;
    if (!keys.getKey("prefix","",prefix)) { // || !keys.getKey("tag","",tag)) {
      ATH_MSG_ERROR( "Problem in overrideTag specification " <<thisOverrideTag );
      return StatusCode::FAILURE;
    }

    for (auto& folderdata : allFolderdata) {
      const std::string& ifname=folderdata.folderName();
      if (ifname.starts_with(prefix) &&
          (ifname.size()==prefix.size() || ifname[prefix.size()]=='/')) {
        //Match! 
        folderdata.applyOverrides(keys,msg());
      }
    }
  }

  //3. Remove any duplicates:
  std::list<IOVDbParser>::iterator it1=allFolderdata.begin(); 
  std::list<IOVDbParser>::iterator it_e=allFolderdata.end(); 
  for (;it1!=it_e;++it1) {
    const IOVDbParser& folder1=*it1;
    std::list<IOVDbParser>::iterator it2=it1;
    ++it2;
    while(it2!=it_e) {
      const IOVDbParser& folder2=*it2;
      if (folder1==folder2) {
        it2=allFolderdata.erase(it2); //FIXME: Smarter distinction/reporting about same folder but different keys.
        ATH_MSG_DEBUG( "Removing duplicate folder " << folder1.folderName());
      } else {
        ++it2;
        //Catch suspicous cases:
        if (folder1.folderName()==folder2.folderName()) {
          ATH_MSG_WARNING( "Folder name appears twice: " << folder1.folderName() );
          ATH_MSG_WARNING( folder1 << " vs " << folder2 );
        }
      }
    }//end inner loop
  }//end outer loop

  //4.Set up folder map with cleaned folder list

  bool crestError=false;
  for (const auto& folderdata : allFolderdata) {
    // find the connection specification first
    // default is to use the 'default' connection
    IOVDbConn* conn=nullptr;
    std::string connstr;
    if (folderdata.getKey("db","",connstr)) {
      // an explicit database name is specified
      // check if it is already present in the existing connections
      for (const auto & pThisConnection : m_connections) {
        if (pThisConnection->name()==connstr) {
          // found existing connection - use that
          conn=pThisConnection.get();
          break;
        }
      }
      if (conn==nullptr) {
        // create new read-onlyconnection
        conn = m_connections.emplace_back(std::make_unique<IOVDbConn>(connstr,true,msg())).get();
      }
    } else {
      // no connection specified - use default if available
      if (!m_par_defaultConnection.empty()) {
        conn=m_connections[0].get();
      } else {
        ATH_MSG_FATAL( "Folder request " << folderdata.folderName() << 
          " gives no DB connection information and no default set" );
        return StatusCode::FAILURE;
      }
    }
    
    // create the new folder, but only if a folder for this SG key has not
    // already been requested

    std::string crestTag;
    if (m_par_source == "CREST"){
      crestTag = m_cresttagmap[folderdata.folderName()];
      if(crestTag.empty() && folderdata.folderName() != "/TagInfo") {
        ATH_MSG_FATAL( "GlobalTag "<< m_par_globalTag << " does not contain folder "
                       << folderdata.folderName());
        crestError=true;
        continue;
      }
    }
    
    auto folder=std::make_unique<IOVDbFolder>(conn,folderdata,msg(),&(*m_h_clidSvc), &(*m_h_metaDataTool),
                                              m_par_checklock, m_outputToFile, m_par_source,
                                              m_par_defaultConnection, crestTag, m_crestCoolToFile);
    const std::string& key=folder->key();
    if (m_foldermap.find(key)==m_foldermap.end()) {  //This check is too weak. For POOL-based folders, the SG key is in the folder description (not known at this point).
      m_foldermap[key]=std::move(folder);
      conn->incUsage();
    } else {
      ATH_MSG_ERROR( "Duplicate request for folder " << 
        folder->folderName() << 
        " associated to already requested Storegate key " << key );
      // clean up this duplicate request
    }
  }// end loop over folders

  if(crestError)
    return StatusCode::FAILURE;

  // check for folders to be written to metadata
  for (const auto & folderToWrite : m_par_foldersToWrite) {
    // match wildcard * at end of string only (i.e. /A/* matches /A/B, /A/C/D)
    std::string_view match=folderToWrite;
    std::string::size_type idx=folderToWrite.find('*');
    if (idx!=std::string::npos) {
      match=std::string_view(folderToWrite).substr(0,idx);
    }
    for (const auto & [name, folder] : m_foldermap) {
      if (folder->folderName().starts_with(match)) {
        folder->setWriteMeta();
        ATH_MSG_INFO( "Folder " << folder->folderName() << " will be written to file metadata" );
      }
    }//end loop over FolderMap
  }//end loop over  m_par_foldersToWrite
  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::fillTagInfo() {
  if (!m_par_globalTag.empty()) {
    ATH_MSG_DEBUG( "Adding GlobalTag " << m_par_globalTag << " into TagInfo" );
    ATH_CHECK( m_h_tagInfoMgr->addTag("IOVDbGlobalTag",m_par_globalTag) );
  }
  // add all explicit tags specified in folders
  // can be from Folders or tagOverrides properties
  for (const auto & [name, folder] : m_foldermap) {
    if (!folder->joTag().empty()) {
      ATH_MSG_DEBUG( "Adding folder " << folder->folderName() <<" tag " << folder->joTag() << " into TagInfo" );
      if (m_h_tagInfoMgr->addTag(folder->folderName(),folder->joTag()).isFailure())
        return StatusCode::FAILURE;
    }
    // check to see if any input TagInfo folder overrides should be removed
    // this anticipates the decisions which will be made in processTagInfo
    // Here we do not have access to the TagInfo object, but can put remove
    // requests in for all folders if the global tag is set, or if there is
    // an explict joboption tag, nooverride spec, or data comes from metadata
    if (!m_par_globalTag.empty() || !folder->joTag().empty() || folder->noOverride() ||
        folder->readMeta()) {
      if (m_h_tagInfoMgr->removeTagFromInput(folder->folderName()).isFailure()) {
        ATH_MSG_WARNING( "Could not add TagInfo remove request for "
               << folder->folderName() );
      } else {
        ATH_MSG_INFO( "Added taginfo remove for " << folder->folderName() );
      }
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode IOVDbSvc::loadCaches(IOVDbConn* conn, const IOVTime* time) {
  // load the caches for all folders using the given connection
  // so connection use is optimised

  Gaudi::Guards::AuditorGuard auditor(std::string("loadCachesOverhead:")+conn->name(), auditorSvc(), "preLoadProxy");

  ATH_MSG_DEBUG( "loadCaches: Begin for connection " << conn->name());
  // if global abort already set, load nothing
  if (m_abort) return StatusCode::FAILURE;
  bool access=false;
  StatusCode sc=StatusCode::SUCCESS;
  for (const auto & [name, folder] : m_foldermap) {
    if (folder->conn()!=conn) continue;
    cool::ValidityKey vkey=folder->iovTime(time==nullptr ? m_iovTime : *time);
    // protect against out of range times (timestamp -1 happened in FDR2)
    if (vkey>cool::ValidityKeyMax) {
      ATH_MSG_WARNING( "Requested validity key " << vkey << " is out of range, reset to 0" );
      vkey=0;
    }
    if (!folder->cacheValid(vkey) && !folder->dropped()) {
      access=true;
      {
        Gaudi::Guards::AuditorGuard auditor(std::string("FldrCache:")+folder->folderName(), auditorSvc(), "preLoadProxy");
        if (!folder->loadCache(vkey,m_par_cacheAlign,m_globalTag,m_par_onlineMode)) {
          ATH_MSG_ERROR( "Cache load (prefetch) failed for folder " << folder->folderName() );
          // remember the failure, but also load other folders on this connection
          // while it is open
          sc=StatusCode::FAILURE;
        }
      }
    }
  }
  // disconnect from database if we connected
  if (access && m_par_manageConnections) conn->setInactive();
  // if connection aborted, set overall abort so we do not waste time trying
  // to read data from other schema
  if (conn->aborted()) {
    m_abort=true;
    throw GaudiException("Connection " + conn->name() + " was aborted",
                         "IOVDbSvc::loadCache()", StatusCode::FAILURE);
  }
  return sc;
}


StatusCode IOVDbSvc::checkConfigConsistency() const {
  // check consistency of global tag and database instance, if set
  // catch most common user misconfigurations
  // this is only done here as need global tag to be set even if read from file
  // @TODO should this not be done during initialize

  if (!m_par_dbinst.empty() && !m_globalTag.empty() && m_par_source!="CREST") {
    const std::string_view tagstub = std::string_view(m_globalTag).substr(0,7);
    ATH_MSG_DEBUG( "Checking " << m_par_dbinst << " against " <<tagstub );

    if ( ((m_par_dbinst=="COMP200" || m_par_dbinst=="CONDBR2") &&
          (tagstub!="COMCOND" && tagstub!="CONDBR2")) ||
         (m_par_dbinst=="OFLP200" && (tagstub!="OFLCOND" && tagstub!="CMCCOND")) ) {

      ATH_MSG_FATAL( "Likely incorrect conditions DB configuration! " <<
                     "Attached to database instance " << m_par_dbinst <<
                     " but global tag begins " << tagstub );
      ATH_MSG_FATAL( "See Atlas/CoolTroubles wiki for details,"
                     " or set IOVDbSvc.DBInstance=\"\" to disable check" );
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}
