/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "HltEventLoopMgr.h"
#include "TrigCOOLUpdateHelper.h"
#include "TrigRDBManager.h"

// Trigger includes
#include "TrigKernel/HltExceptions.h"
#include "TrigSteeringEvent/HLTResultMT.h"

// Athena includes
#include "AthenaInterprocess/Incidents.h"
#include "AthenaKernel/AthStatusCode.h"
#include "ByteStreamData/ByteStreamMetadata.h"
#include "ByteStreamData/ByteStreamMetadataContainer.h"
#include "EventInfoUtils/EventInfoFromxAOD.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/SGHiveMgrSvc.h"

// Gaudi includes
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/ThreadLocalContext.h"

// TDAQ includes
#include "eformat/StreamTag.h"

// ROOT includes
#include "TROOT.h"
#include "TSystem.h"

// System includes
#include <filesystem>
#include <format>
#include <sstream>
#include <string>
#include <time.h>

// =============================================================================
// Helper macros, typedefs and constants
// =============================================================================
namespace {
  bool isTimedOut(const std::unordered_map<std::string_view,StatusCode>& algErrors) {
    for (const auto& [key, sc] : algErrors) {
      if (sc == Athena::Status::TIMEOUT) return true;
    }
    return false;
  }
  /// Workaround for classes which implement operator<< but don't have a method to convert to string
  template <typename T> std::string toString(const T& x) {
    std::ostringstream ss;
    ss << x;
    return ss.str();
  }
}
using namespace boost::property_tree;

// =============================================================================
// Standard constructor
// =============================================================================
HltEventLoopMgr::HltEventLoopMgr(const std::string& name, ISvcLocator* svcLoc)
: base_class(name, svcLoc) {}

// =============================================================================
// Standard destructor
// =============================================================================
HltEventLoopMgr::~HltEventLoopMgr() noexcept
{
  // tbb:task_group destructor throws if wait() was never called
  m_parallelIOTaskGroup.wait();
}

// =============================================================================
// Reimplementation of AthService::initalize (IStateful interface)
// =============================================================================
StatusCode HltEventLoopMgr::initialize()
{
  // Do not auto-retrieve tools (see Gaudi!1124)
  m_autoRetrieveTools = false;
  m_checkToolDeps = false;

  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  ATH_MSG_INFO(" ---> HltEventLoopMgr = " << name() << " initialize");

  //----------------------------------------------------------------------------
  // Setup properties
  //----------------------------------------------------------------------------

  // Set the timeout value (cast float to int)
  m_softTimeoutValue = std::chrono::milliseconds(static_cast<int>(m_hardTimeout.value() * m_softTimeoutFraction.value()));

  // Read DataFlow configuration properties
  updateDFProps();

  // print properties
  ATH_MSG_INFO(" ---> ApplicationName           = " << m_applicationName);
  ATH_MSG_INFO(" ---> HardTimeout               = " << m_hardTimeout.value());
  ATH_MSG_INFO(" ---> SoftTimeoutFraction       = " << m_softTimeoutFraction.value());
  ATH_MSG_INFO(" ---> SoftTimeoutValue          = " << m_softTimeoutValue.count());
  ATH_MSG_INFO(" ---> TimeoutThreadIntervalMs   = " << m_timeoutThreadIntervalMs.value());
  ATH_MSG_INFO(" ---> TraceOnTimeout            = " << m_traceOnTimeout.value());
  ATH_MSG_INFO(" ---> MaxFrameworkErrors        = " << m_maxFrameworkErrors.value());
  ATH_MSG_INFO(" ---> FwkErrorDebugStreamName   = " << m_fwkErrorDebugStreamName.value());
  ATH_MSG_INFO(" ---> AlgErrorDebugStreamName   = " << m_algErrorDebugStreamName.value());
  ATH_MSG_INFO(" ---> TimeoutDebugStreamName    = " << m_timeoutDebugStreamName.value());
  ATH_MSG_INFO(" ---> TruncationDebugStreamName = " << m_truncationDebugStreamName.value());
  ATH_MSG_INFO(" ---> SORPath                   = " << m_sorPath.value());
  ATH_MSG_INFO(" ---> setMagFieldFromPtree      = " << m_setMagFieldFromPtree.value());
  ATH_MSG_INFO(" ---> execAtStart               = " << m_execAtStart.value());
  ATH_MSG_INFO(" ---> forceRunNumber            = " << m_forceRunNumber.value());
  ATH_MSG_INFO(" ---> forceLumiblock            = " << m_forceLumiblock.value());
  ATH_MSG_INFO(" ---> forceStartOfRunTime       = " << m_forceSOR_ns.value());
  ATH_MSG_INFO(" ---> RewriteLVL1               = " << m_rewriteLVL1.value());
  ATH_MSG_INFO(" ---> EventContextWHKey         = " << m_eventContextWHKey.key());
  ATH_MSG_INFO(" ---> EventInfoRHKey            = " << m_eventInfoRHKey.key());

  ATH_CHECK( m_jobOptionsSvc.retrieve() );
  const std::string& slots = m_jobOptionsSvc->get("EventDataSvc.NSlots");
  if (!slots.empty())
    ATH_MSG_INFO(" ---> NumConcurrentEvents       = " << slots);
  else
    ATH_MSG_WARNING("Failed to retrieve the job property EventDataSvc.NSlots");
  const std::string& threads = m_jobOptionsSvc->get("AvalancheSchedulerSvc.ThreadPoolSize");
  if (!threads.empty())
    ATH_MSG_INFO(" ---> NumThreads                = " << threads);
  else
    ATH_MSG_WARNING("Failed to retrieve the job property AvalancheSchedulerSvc.ThreadPoolSize");

  const std::string& procs = m_jobOptionsSvc->get("DataFlowConfig.DF_NumberOfWorkers");
  if (!procs.empty()) {
    ATH_MSG_INFO(" ---> NumProcs                  = " << procs);
    try {
      SG::HiveMgrSvc::setNumProcs(std::stoi(procs));
    }
    catch (const std::logic_error& ex) {
      ATH_MSG_ERROR("Cannot convert " << procs << "to integer: " << ex.what());
      return StatusCode::FAILURE;
    }
  }
  else
    ATH_MSG_WARNING("Failed to retrieve the job property DataFlowconfig.DF_NumberOfWorkers");

  if (m_maxParallelIOTasks.value() <= 0) {
    ATH_CHECK(m_maxParallelIOTasks.fromString(threads));
  }
  ATH_MSG_INFO(" ---> MaxParallelIOTasks        = " << m_maxParallelIOTasks.value());
  ATH_MSG_INFO(" ---> MaxIOWakeUpIntervalMs     = " << m_maxIOWakeUpIntervalMs.value());

  //----------------------------------------------------------------------------
  // Setup all Hive services for multithreaded event processing with the exception of SchedulerSvc,
  // which has to be initialised after forking because it opens new threads
  //----------------------------------------------------------------------------
  m_whiteboard = serviceLocator()->service(m_whiteboardName);
  if( !m_whiteboard.isValid() )  {
    ATH_MSG_FATAL("Error retrieving " << m_whiteboardName << " interface IHiveWhiteBoard");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Initialised " << m_whiteboardName << " interface IHiveWhiteBoard");

  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
  if( !m_algResourcePool.isValid() ) {
    ATH_MSG_FATAL("Error retrieving AlgResourcePool");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("initialised AlgResourcePool");

  m_aess = serviceLocator()->service("AlgExecStateSvc");
  if( !m_aess.isValid() ) {
    ATH_MSG_FATAL("Error retrieving AlgExecStateSvc");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("initialised AlgExecStateSvc");

  //----------------------------------------------------------------------------
  // Initialise services
  //----------------------------------------------------------------------------
  ATH_CHECK(m_incidentSvc.retrieve());
  ATH_CHECK(m_evtStore.retrieve());
  ATH_CHECK(m_detectorStore.retrieve());
  ATH_CHECK(m_inputMetaDataStore.retrieve());
  ATH_CHECK(m_evtSelector.retrieve());
  ATH_CHECK(m_evtSelector->createContext(m_evtSelContext)); // create an EvtSelectorContext
  ATH_CHECK(m_outputCnvSvc.retrieve());
  ATH_CHECK(m_ioCompMgr.retrieve());
  if (m_monitorScheduler) {
    ATH_CHECK(m_schedulerMonSvc.retrieve());
  }

  //----------------------------------------------------------------------------
  // Initialise tools
  //----------------------------------------------------------------------------
  // COOL helper
  ATH_CHECK(m_coolHelper.retrieve());
  // HLT result builder
  ATH_CHECK(m_hltResultMaker.retrieve());
  // Monitoring tools
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
  ATH_CHECK(m_errorMonTool.retrieve());

  //----------------------------------------------------------------------------
  // Initialise data handle keys
  //----------------------------------------------------------------------------
  // EventContext WriteHandle
  ATH_CHECK(m_eventContextWHKey.initialize());
  // EventInfo ReadHandle
  ATH_CHECK(m_eventInfoRHKey.initialize());
  // HLTResultMT ReadHandle (created dynamically from the result builder property)
  m_hltResultRHKey = m_hltResultMaker->resultName();
  ATH_CHECK(m_hltResultRHKey.initialize());
  // L1TriggerResult and RoIBResult ReadHandles for RewriteLVL1
  ATH_CHECK(m_l1TriggerResultRHKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_roibResultRHKey.initialize(SG::AllowEmpty));

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Reimplementation of AthService::stop (IStateful interface)
// =============================================================================
StatusCode HltEventLoopMgr::stop()
{
  // Need to reinitialize IO in the mother process
  if (m_workerID==0) {
    ATH_CHECK(m_ioCompMgr->io_reinitialize());
  }

  return StatusCode::SUCCESS;
}

// =============================================================================
// Reimplementation of AthService::finalize (IStateful interface)
// =============================================================================
StatusCode HltEventLoopMgr::finalize()
{
  ATH_MSG_INFO(" ---> HltEventLoopMgr/" << name() << " finalize ");
  // Usually (but not necessarily) corresponds to the number of processed events +1
  ATH_MSG_INFO("Total number of EventContext objects created " << m_localEventNumber);

  // Release all handles
  auto releaseAndCheck = [&](auto& handle, std::string_view handleType) {
    if (handle.release().isFailure())
      ATH_MSG_WARNING("finalize(): Failed to release " << handleType << " " << handle.typeAndName());
  };
  auto releaseService = [&](auto&&... args) { (releaseAndCheck(args,"service"), ...); };
  auto releaseTool = [&](auto&&... args) { (releaseAndCheck(args,"tool"), ...); };
  auto releaseSmartIF = [](auto&&... args) { (args.reset(), ...); };

  releaseService(m_incidentSvc,
                 m_evtStore,
                 m_detectorStore,
                 m_inputMetaDataStore,
                 m_evtSelector,
                 m_outputCnvSvc,
                 m_schedulerMonSvc);

  releaseTool(m_coolHelper,
              m_hltResultMaker,
              m_monTool);

  releaseSmartIF(m_whiteboard,
                 m_algResourcePool,
                 m_aess,
                 m_schedulerSvc);

  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of ITrigEventLoopMgr::prepareForStart
// =============================================================================
StatusCode HltEventLoopMgr::prepareForStart(const ptree& pt)
{
  try {
    const auto& rparams = pt.get_child("RunParams");
    m_sorHelper = std::make_unique<TrigSORFromPtreeHelper>(msgSvc(), m_detectorStore, m_sorPath, rparams);
  }
  catch(ptree_bad_path& e) {
    ATH_MSG_ERROR("Bad ptree path: \"" << e.path<ptree::path_type>().dump() << "\" - " << e.what());
    return StatusCode::FAILURE;
  }

  // Override run/timestamp if needed
  if (m_forceRunNumber > 0) {
    m_sorHelper->setRunNumber(m_forceRunNumber);
    ATH_MSG_WARNING("Run number overwrite:" << m_forceRunNumber);
  }
  if (m_forceSOR_ns > 0) {
    m_sorHelper->setSORtime_ns(m_forceSOR_ns);
    ATH_MSG_WARNING("SOR time overwrite:" << m_forceSOR_ns);
  }

  // Set our "run context"
  m_currentRunCtx.setEventID( m_sorHelper->eventID() );
  m_currentRunCtx.setExtension(Atlas::ExtendedEventContext(m_evtStore->hiveProxyDict(),
                                                           m_currentRunCtx.eventID().run_number()));

  // Some algorithms expect a valid context during start()
  ATH_MSG_DEBUG("Setting context for start transition: " << m_currentRunCtx.eventID());
  Gaudi::Hive::setCurrentContext(m_currentRunCtx);

  try {
    ATH_CHECK( clearTemporaryStores() );                 // do the necessary resets
    ATH_CHECK( m_sorHelper->fillSOR(m_currentRunCtx) );  // update SOR in det store

    const auto& soral = getSorAttrList();
    updateMetadataStore(soral);  // update metadata store
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Exception: " << e.what());
  }

  ATH_CHECK( updateMagField(pt) );  // update magnetic field

  return StatusCode::SUCCESS;
}


// =============================================================================
// Implementation of ITrigEventLoopMgr::prepareForRun
// =============================================================================
StatusCode HltEventLoopMgr::prepareForRun(const ptree& /*pt*/)
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  try
  {
    // Reset the AlgExecStateSvc (important in case there was a stop/start)
    m_aess->reset(m_currentRunCtx);

    // Fire BeginRun incident
    m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginRun, m_currentRunCtx));

    // Initialize COOL helper (needs to be done after IOVDbSvc has loaded all folders)
    ATH_CHECK(m_coolHelper->readFolderInfo());

    // Run optional algs/sequences (e.g. CondAlgs ATR-26138)
    ATH_CHECK(execAtStart(m_currentRunCtx));

    // close any open files (e.g. THistSvc)
    ATH_CHECK(m_ioCompMgr->io_finalize());

    // Verify that there are no other open ROOT files (e.g. from dual-use tools).
    if ( !gROOT->GetListOfFiles()->IsEmpty() ) {
      std::unordered_map<std::string, size_t> dups;
      for (const auto f : *gROOT->GetListOfFiles()) {
        ++dups[f->GetName()];
      }
      // Exception for THistSvc files as those will remain open
      auto histsvc = serviceLocator()->service("THistSvc", false).as<IIoComponent>();
      for (const std::string& histfile : m_ioCompMgr->io_retrieve(histsvc.get())) {
        dups.erase(histfile);
      }
      if (!dups.empty()) {
        msg() << MSG::ERROR << "The following ROOT files (with #instances) have not been closed yet: ";
        for (const auto& [n,c] : dups) msg() << n << "(x" << c << ") ";
        msg() << endmsg;
      }
    }

    // close open DB connections
    ATH_CHECK(TrigRDBManager::closeDBConnections(msg()));

    // Assert that scheduler has not been initialised before forking
    SmartIF<IService> svc = serviceLocator()->service(m_schedulerName, /*createIf=*/ false);
    if (svc.isValid()) {
      ATH_MSG_FATAL("Misconfiguration - Scheduler was initialised before forking!");
      return StatusCode::FAILURE;
    }

    ATH_MSG_VERBOSE("end of " << __FUNCTION__);
    return StatusCode::SUCCESS;
  }
  catch(const std::runtime_error& e)
  {
    ATH_MSG_ERROR("Runtime error: " << e.what());
  }

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::FAILURE;
}


// =============================================================================
StatusCode HltEventLoopMgr::execAtStart(const EventContext& ctx) const
{
  IAlgManager* algMgr = Gaudi::svcLocator()->as<IAlgManager>();

  StatusCode sc;
  for (const std::string& name : m_execAtStart) {
    SmartIF<IAlgorithm>& alg = algMgr->algorithm(name, /*createIf*/false);
    if ( alg ) {
      ATH_MSG_INFO("Executing " << alg->name() << "...");
      sc &= alg->sysExecute(ctx);
    }
    else ATH_MSG_WARNING("Cannot find algorithm or sequence " << name);
  }
  return sc;
}


// =============================================================================
// Implementation of ITrigEventLoopMgr::hltUpdateAfterFork
// =============================================================================
StatusCode HltEventLoopMgr::hltUpdateAfterFork(const ptree& /*pt*/)
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  updateDFProps();
  ATH_MSG_INFO("Post-fork initialization for " << m_applicationName);

  ATH_MSG_DEBUG("Initialising the scheduler after forking");
  m_schedulerSvc = serviceLocator()->service(m_schedulerName, /*createIf=*/ true);
  if ( !m_schedulerSvc.isValid()){
    ATH_MSG_FATAL("Error retrieving " << m_schedulerName << " interface ISchedulerSvc");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Initialised " << m_schedulerName << " interface ISchedulerSvc");

  ATH_MSG_DEBUG("Trying a stop-start of CoreDumpSvc");
  SmartIF<IService> svc = serviceLocator()->service("CoreDumpSvc", /*createIf=*/ false);
  if (svc.isValid()) {
    StatusCode sc = svc->stop();
    sc &= svc->start();
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Could not perform stop/start for CoreDumpSvc");
    }
    else {
      ATH_MSG_DEBUG("Done a stop-start of CoreDumpSvc");
    }
  }
  else {
    ATH_MSG_WARNING("Could not retrieve CoreDumpSvc");
  }

  // Make sure output files, i.e. histograms are written to their own directory.
  // Nothing happens if the online TrigMonTHistSvc is used as there are no output files.
  SmartIF<IIoComponent> histsvc = serviceLocator()->service("THistSvc", /*createIf=*/ false).as<IIoComponent>();
  if ( !m_ioCompMgr->io_retrieve(histsvc.get()).empty() ) {
    std::filesystem::path worker_dir = std::filesystem::absolute("athenaHLT_workers");
    std::ostringstream oss;
    oss << "athenaHLT-" << std::setfill('0') << std::setw(2) << m_workerID;
    worker_dir /= oss.str();
    // Delete worker directory if it exists already
    if ( std::filesystem::exists(worker_dir) ) {
      if ( std::filesystem::remove_all(worker_dir) == 0 ) {
        ATH_MSG_FATAL("Cannot delete previous worker directory " << worker_dir);
        return StatusCode::FAILURE;
      }
    }
    if ( !std::filesystem::create_directories(worker_dir) ) {
      ATH_MSG_FATAL("Cannot create worker directory " << worker_dir);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Writing worker output files to " << worker_dir);
    ATH_CHECK(m_ioCompMgr->io_update_all(worker_dir.string()));
  }
  ATH_CHECK(m_ioCompMgr->io_reinitialize());

  const size_t numSlots = m_whiteboard->getNumberOfStores();
  m_freeSlots = numSlots;

  // Initialise vector of time points for event timeout monitoring
  m_eventTimerStartPoint.clear();
  m_eventTimerStartPoint.resize(numSlots, std::chrono::steady_clock::now());
  m_isSlotProcessing.resize(numSlots, false);

  // Initialise vector of time points for free slots monitoring
  m_freeSlotStartPoint.clear();
  m_freeSlotStartPoint.resize(numSlots, std::chrono::steady_clock::now());

  // Initialise the queues used in parallel I/O steering
  m_parallelIOQueue.set_capacity(static_cast<decltype(m_parallelIOQueue)::size_type>(m_maxParallelIOTasks.value()));
  m_finishedEventsQueue.set_capacity(static_cast<decltype(m_finishedEventsQueue)::size_type>(numSlots));

  // Fire incident to update listeners after forking
  m_incidentSvc->fireIncident(AthenaInterprocess::UpdateAfterFork(m_workerID, m_workerPID, name(), m_currentRunCtx));

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEventProcessor::executeRun
// =============================================================================
StatusCode HltEventLoopMgr::executeRun(int maxevt)
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  if (m_monitorScheduler) ATH_CHECK(m_schedulerMonSvc->startMonitoring());

  StatusCode sc = StatusCode::SUCCESS;
  try {
    sc = nextEvent(maxevt);
    if (sc.isFailure()) ATH_MSG_FATAL("Event loop failed");
  }
  catch (const std::exception& e) {
    ATH_MSG_FATAL("Event loop failed, std::exception caught: " << e.what());
    sc = StatusCode::FAILURE;
  }
  catch (...) {
    ATH_MSG_FATAL("Event loop failed, unknown exception caught");
    sc = StatusCode::FAILURE;
  }

  if (m_monitorScheduler) ATH_CHECK(m_schedulerMonSvc->stopMonitoring());

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return sc;
}

// =============================================================================
// Implementation of IEventProcessor::nextEvent
// maxevt is not used - we always want to process all events delivered
// =============================================================================
StatusCode HltEventLoopMgr::nextEvent(int /*maxevt*/)
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  // Start the event timer thread
  ATH_MSG_DEBUG("Starting the timeout thread");
  m_timeoutThread = std::make_unique<HLT::LoopThread>([this]{return eventTimerCallback();}, m_timeoutThreadIntervalMs.value());
  m_timeoutThread->start();

  // Start the event loop
  ATH_MSG_INFO("Starting loop on events");
  std::unique_lock<std::mutex> lock{m_loopStatus.loopEndedMutex};
  m_inputThread = std::make_unique<HLT::LoopThread>([this]{return inputThreadCallback();}, m_maxIOWakeUpIntervalMs.value());
  m_outputThread = std::make_unique<HLT::LoopThread>([this]{return outputThreadCallback();}, m_maxIOWakeUpIntervalMs.value());
  m_outputThread->start();
  m_inputThread->start();

  // Wait for event loop to end. The condition means the main input and output threads flagged they have
  // nothing else to do and will exit asynchronously (later than the wait here ends)
  ATH_MSG_DEBUG("Event loop started, the main thread is going to sleep until it finishes");
  m_loopStatus.loopEndedCond.wait(lock, [this](){return m_loopStatus.loopEnded.load();});
  ATH_MSG_INFO("All events processed, finalising the event loop");

  // Wait for the I/O TBB tasks and main I/O threads to finish. Note the TBB tasks need to finish first
  // because they may notify the condition variables in the main I/O threads. The lifetime of the condition
  // variables must span beyond any I/O TBB task.
  ATH_MSG_DEBUG("Waiting for all I/O tasks and threads to return");
  m_parallelIOTaskGroup.wait();
  m_inputThread->wait();
  m_outputThread->wait();

  // Stop the event timer thread
  ATH_MSG_DEBUG("All I/O threads and tasks finished. Stopping the timeout thread");
  m_timeoutThread->stop();
  m_timeoutThread->wait();
  ATH_MSG_DEBUG("The timeout thread finished");

  ATH_MSG_INFO("Finished loop on events");

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEventProcessor::stopRun (obsolete for online runnning)
// =============================================================================
StatusCode HltEventLoopMgr::stopRun() {
  ATH_MSG_FATAL("Misconfiguration - the method HltEventLoopMgr::stopRun() cannot be used online");
  return StatusCode::FAILURE;
}

// =============================================================================
// Implementation of IEventProcessor::createEventContext
// =============================================================================
EventContext HltEventLoopMgr::createEventContext() {
  size_t eventNumber = ++m_localEventNumber;
  auto slot = m_whiteboard->allocateStore(eventNumber); // returns npos on failure
  if (slot == std::string::npos) {
    // return an invalid EventContext
    return EventContext();
  }
  return EventContext{ eventNumber, slot };
}

// =============================================================================
// Implementation of IEventProcessor::executeEvent
// =============================================================================
StatusCode HltEventLoopMgr::executeEvent(EventContext &&ctx)
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  resetTimeout(Athena::Timeout::instance(ctx));

  // Monitor slot idle time (between scheduler popFinishedEvent and pushNewEvent)
  // Note this is time of a scheduler slot being free, not equal to the time of a whiteboard slot being free
  const auto slotIdleTime = std::chrono::steady_clock::now() - m_freeSlotStartPoint[ctx.slot()];
  Monitored::Scalar<int64_t> monSlotIdleTime("SlotIdleTime", std::chrono::duration_cast<std::chrono::milliseconds>(slotIdleTime).count());
  Monitored::Group(m_monTool, monSlotIdleTime);

  // Now add event to the scheduler
  ATH_MSG_DEBUG("Adding event " <<  ctx.evt() << ", slot " << ctx.slot() << " to the scheduler");
  StatusCode addEventStatus = m_schedulerSvc->pushNewEvent( new EventContext{std::move(ctx)} );

  // If this fails, we need to wait for something to complete
  if (addEventStatus.isFailure()){
    ATH_MSG_ERROR("Failed adding event to the scheduler");
    return StatusCode::FAILURE;
  }

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
void HltEventLoopMgr::updateDFProps()
{
  auto getDFProp = [&](const std::string& name, std::string& value, bool required = true) {
                     if (m_jobOptionsSvc->has("DataFlowConfig."+name)) {
                       value = m_jobOptionsSvc->get("DataFlowConfig."+name);
                       ATH_MSG_INFO(" ---> Read from DataFlow configuration: " << name << " = " << value);
                     } else {
                       msg() << (required ? MSG::WARNING : MSG::INFO)
                             << "Could not set Property " << name << " from DataFlow" << endmsg;
                     }
                   };

  getDFProp( "DF_ApplicationName", m_applicationName );
  std::string wid, wpid;
  getDFProp( "DF_WorkerId", wid, false );
  getDFProp( "DF_Pid", wpid, false );
  if (!wid.empty()) m_workerID = std::stoi(wid);
  if (!wpid.empty()) m_workerPID = std::stoi(wpid);
}

// =============================================================================
void HltEventLoopMgr::updateMetadataStore(const coral::AttributeList & sor_attrlist) const
{
  auto metadatacont = std::make_unique<ByteStreamMetadataContainer>();
  metadatacont->push_back(std::make_unique<ByteStreamMetadata>(
    sor_attrlist["RunNumber"].data<unsigned int>(),
    0,
    0,
    sor_attrlist["RecordingEnabled"].data<bool>(),
    0,
    sor_attrlist["DetectorMaskSnd"].data<unsigned long long>(),
    sor_attrlist["DetectorMaskFst"].data<unsigned long long>(),
    0,
    0,
    "",
    "",
    "",
    0,
    std::vector<std::string>()
  ));
  // Record ByteStreamMetadataContainer in MetaData Store
  if(m_inputMetaDataStore->record(std::move(metadatacont),"ByteStreamMetadata").isFailure()) {
    ATH_MSG_WARNING("Unable to record MetaData in InputMetaDataStore");
  }
  else {
    ATH_MSG_DEBUG("Recorded MetaData in InputMetaDataStore");
  }
}

//=========================================================================
StatusCode HltEventLoopMgr::updateMagField(const ptree& pt) const
{
  if (m_setMagFieldFromPtree) {
    try {
      auto tor_cur = pt.get<float>("Magnets.ToroidsCurrent.value");
      auto sol_cur = pt.get<float>("Magnets.SolenoidCurrent.value");

      // Set current on conditions alg
      IAlgManager* algMgr = Gaudi::svcLocator()->as<IAlgManager>();

      SmartIF<IAlgorithm>& fieldAlg = algMgr->algorithm("AtlasFieldMapCondAlg", /*createIf*/false);
      if ( fieldAlg ) {
        ATH_MSG_INFO("Setting field currents on AtlasFieldMapCondAlg");
        ATH_CHECK( Gaudi::Utils::setProperty(fieldAlg, "MapSoleCurrent", sol_cur) );
        ATH_CHECK( Gaudi::Utils::setProperty(fieldAlg, "MapToroCurrent", tor_cur) );
      }
      else ATH_MSG_WARNING("Cannot retrieve AtlasFieldMapCondAlg");

      ATH_MSG_INFO("*****************************************");
      ATH_MSG_INFO("  Auto-configuration of magnetic field:  ");
      ATH_MSG_INFO("    solenoid current from IS = " << sol_cur);
      ATH_MSG_INFO("     torroid current from IS = " << tor_cur);
      ATH_MSG_INFO("*****************************************");
    }
    catch(ptree_bad_path& e) {
      ATH_MSG_ERROR( "Cannot read magnet currents from ptree: " << e.what() );
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}


// =============================================================================
StatusCode HltEventLoopMgr::clearTemporaryStores()
{
  //----------------------------------------------------------------------------
  // Clear the event store, if used in the event loop
  //----------------------------------------------------------------------------
  ATH_CHECK(m_evtStore->clearStore());
  ATH_MSG_DEBUG("Cleared the EventStore");

  //----------------------------------------------------------------------------
  // Clear the InputMetaDataStore
  //----------------------------------------------------------------------------
  ATH_CHECK(m_inputMetaDataStore->clearStore());
  ATH_MSG_DEBUG("Cleared the InputMetaDataStore");

  return StatusCode::SUCCESS;
}

// =============================================================================
const coral::AttributeList& HltEventLoopMgr::getSorAttrList() const
{
  auto sor = m_detectorStore->retrieve<const TrigSORFromPtreeHelper::SOR>(m_sorPath);
  if (sor==nullptr) {
    throw std::runtime_error("Cannot retrieve " + m_sorPath);
  }
  if(sor->size() != 1)
  {
    // This branch should never be entered (the CondAttrListCollection
    // corresponding to the SOR should contain one single AttrList). Since
    // that's required by code ahead but not checked at compile time, we
    // explicitly guard against any potential future mistake with this check
    throw std::runtime_error("SOR record should have one and one only attribute list, but it has " + std::to_string(sor->size()));
  }

  const auto & soral = sor->begin()->second;
  printSORAttrList(soral);
  return soral;
}

// =============================================================================
void HltEventLoopMgr::printSORAttrList(const coral::AttributeList& atr) const
{
  unsigned long long sorTime_ns(atr["SORTime"].data<unsigned long long>());

  // Human readable format of SOR time
  time_t sorTime_sec = sorTime_ns / std::nano::den;
  struct tm buf;

  ATH_MSG_INFO("SOR parameters:");
  ATH_MSG_INFO("   RunNumber             = " << atr["RunNumber"].data<unsigned int>());
  ATH_MSG_INFO("   SORTime [ns]          = " << sorTime_ns <<
               " (" << std::put_time(localtime_r(&sorTime_sec, &buf), "%F %T") << ") ");

  auto dmfst = atr["DetectorMaskFst"].data<unsigned long long>();
  auto dmsnd = atr["DetectorMaskSnd"].data<unsigned long long>();
  ATH_MSG_INFO("   DetectorMaskFst       = 0x" << std::format("{:016x}", dmfst));
  ATH_MSG_INFO("   DetectorMaskSnd       = 0x" << std::format("{:016x}", dmsnd));
  ATH_MSG_INFO("   Complete DetectorMask = 0x" << std::format("{:016x}", dmfst)
                                               << std::format("{:016x}", dmsnd));

  ATH_MSG_INFO("   RunType               = " << atr["RunType"].data<std::string>());
  ATH_MSG_INFO("   RecordingEnabled      = " << (atr["RecordingEnabled"].data<bool>() ? "true" : "false"));
}

// =============================================================================
StatusCode HltEventLoopMgr::failedEvent(HLT::OnlineErrorCode errorCode, const EventContext& eventContext)
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__ << " with errorCode = " << errorCode
                  << ", context = " << eventContext << " eventID = " << eventContext.eventID());

  // Used by MsgSvc (and possibly others but not relevant here)
  Gaudi::Hive::setCurrentContext(eventContext);

  auto returnFailureAndStopEventLoop = [this]() -> StatusCode {
    ATH_MSG_INFO("Stopping event loop due to failure");
    // Change the loop exit code to FAILURE
    m_loopStatus.exitCode = StatusCode::FAILURE;

    // Flag eventsAvailable=false which will result in I/O threads to finish all the ongoing event processing
    // and then stop. We cannot flag loopEnded=true here yet, because it would finish the I/O threads while
    // there might be still events being processed and they would crash when finished.
    m_loopStatus.eventsAvailable = false;

    // Inform the caller the failure could not be handled cleanly and the event loop will stop
    return StatusCode::FAILURE;
  };

  //----------------------------------------------------------------------------
  // Handle framework errors by printing an informative message and breaking the loop
  //----------------------------------------------------------------------------
  if (errorCode==HLT::OnlineErrorCode::BEFORE_NEXT_EVENT) {
    ATH_MSG_ERROR("Failure occurred with OnlineErrorCode=" << errorCode
      << " meaning there was a framework error before requesting a new event. No output will be produced for this event"
      << " and the event loop will exit after all ongoing processing is finished.");
    return returnFailureAndStopEventLoop();
  }
  if (errorCode==HLT::OnlineErrorCode::CANNOT_RETRIEVE_EVENT) {
    ATH_MSG_ERROR("Failure occurred with OnlineErrorCode=" << errorCode
      << " meaning a new event could not be correctly read. No output will be produced for this event."
      << " The event loop will exit after all ongoing processing is finished.");
    return returnFailureAndStopEventLoop();
  }
  if (errorCode==HLT::OnlineErrorCode::AFTER_RESULT_SENT) {
    ATH_MSG_ERROR("Failure occurred with OnlineErrorCode=" << errorCode
      << " meaning there was a framework error after HLT result was already sent out."
      << " The event loop will exit after all ongoing processing is finished.");
    return returnFailureAndStopEventLoop();
  }
  if (errorCode==HLT::OnlineErrorCode::CANNOT_ACCESS_SLOT) {
    ATH_MSG_ERROR("Failed to access the slot for the processed event, cannot produce output. OnlineErrorCode="
      << errorCode << ". The event loop will exit after all ongoing processing is finished unless the failed event"
      << " reaches a hard timeout sooner and this process is killed.");
    return returnFailureAndStopEventLoop();
  }
  if (errorCode==HLT::OnlineErrorCode::SCHEDULING_FAILURE) {
    // Here we cannot be certain if the scheduler started processing the event or not. If yes, the output thread
    // will finalise the event as normal. If not, the event will eventually reach a hard timeout and this process
    // is killed, or we exit the process without ever producing output for this event (needs to be handled upstream).
    ATH_MSG_ERROR("Failure occurred with OnlineErrorCode=" << errorCode
      << ". Cannot determine if the event processing started or not and whether a decision for this event will be"
      << " produced. The event loop will exit after all ongoing processing is finished, which may include or"
      << " not include the problematic event.");
    return returnFailureAndStopEventLoop();
  }
  if (errorCode==HLT::OnlineErrorCode::SCHEDULER_POP_FAILURE) {
    ATH_MSG_ERROR("Failure occurred with OnlineErrorCode=" << errorCode
      << " meaning the Scheduler returned FAILURE when asked to give a finished event. Will keep trying to"
      << " pop further events if there are any still in the scheduler, but this may keep repeating until"
      << " this process is killed by hard timeout or other means. If all ongoing processing manages to finish"
      << " then the event loop will exit.");
    return returnFailureAndStopEventLoop();
  }
  if (!eventContext.valid()) {
    ATH_MSG_ERROR("Failure occurred with an invalid EventContext. Likely there was a framework error before"
      << " requesting a new event or after sending the result of a finished event. OnlineErrorCode=" << errorCode
      << ". The event loop will exit after all ongoing processing is finished.");
    return returnFailureAndStopEventLoop();
  }

  //----------------------------------------------------------------------------
  // Make sure we are using the right store
  //----------------------------------------------------------------------------
  if (m_whiteboard->selectStore(eventContext.slot()).isFailure()) {
    return failedEvent(HLT::OnlineErrorCode::CANNOT_ACCESS_SLOT,eventContext);
  }

  //----------------------------------------------------------------------------
  // Define a debug stream tag for the HLT result
  //----------------------------------------------------------------------------
  std::string debugStreamName;
  switch (errorCode) {
    case HLT::OnlineErrorCode::PROCESSING_FAILURE:
      debugStreamName = m_algErrorDebugStreamName.value();
      break;
    case HLT::OnlineErrorCode::TIMEOUT:
      debugStreamName = m_timeoutDebugStreamName.value();
      break;
    case HLT::OnlineErrorCode::RESULT_TRUNCATION:
      debugStreamName = m_truncationDebugStreamName.value();
      break;
    default:
      debugStreamName = m_fwkErrorDebugStreamName.value();
      break;
  }
  eformat::helper::StreamTag debugStreamTag{debugStreamName, eformat::DEBUG_TAG, true};

  //----------------------------------------------------------------------------
  // Create an HLT result for the failed event (copy one if it exists and contains serialised data)
  //----------------------------------------------------------------------------
  std::unique_ptr<HLT::HLTResultMT> hltResultPtr;
  StatusCode buildResultCode{StatusCode::SUCCESS};
  auto hltResultRH = SG::makeHandle(m_hltResultRHKey,eventContext);
  if (hltResultRH.isValid() && !hltResultRH->getSerialisedData().empty()) {
    // There is already an existing result, create a copy with the error code and stream tag
    hltResultPtr = std::make_unique<HLT::HLTResultMT>(*hltResultRH);
    hltResultPtr->addErrorCode(errorCode);
    buildResultCode &= hltResultPtr->addStreamTag(debugStreamTag);
  } else {
    // Create a result if not available, pre-fill with error code an stream tag, then try to fill event data
    hltResultPtr = std::make_unique<HLT::HLTResultMT>();
    hltResultPtr->addErrorCode(errorCode);
    buildResultCode &= hltResultPtr->addStreamTag(debugStreamTag);
    // Fill the result unless we already failed doing this before
    if (errorCode != HLT::OnlineErrorCode::NO_HLT_RESULT) {
      buildResultCode &= m_hltResultMaker->fillResult(*hltResultPtr,eventContext);
    }
  }

  // Try to record the result in th event store
  SG::WriteHandleKey<HLT::HLTResultMT> hltResultWHK(m_hltResultRHKey.key()+"_FailedEvent");
  buildResultCode &= hltResultWHK.initialize();
  auto hltResultWH = SG::makeHandle(hltResultWHK,eventContext);
  if (buildResultCode.isFailure() || hltResultWH.record(std::move(hltResultPtr)).isFailure()) {
    if (errorCode == HLT::OnlineErrorCode::NO_HLT_RESULT) {
      // Avoid infinite loop
      ATH_MSG_ERROR("Second failure to build or record the HLT Result in event store while handling a failed event. "
                    << "Cannot force-accept this event from HLT side, will rely on data collector to do this. "
                    << "The event loop will exit after all ongoing processing is finished.");
      return returnFailureAndStopEventLoop();
    }
    ATH_MSG_ERROR("Failed to build or record the HLT Result in event store while handling a failed event. "
                  << "Trying again with skipped filling of the result contents (except debug stream tag).");
    return failedEvent(HLT::OnlineErrorCode::NO_HLT_RESULT,eventContext);
  }

  //----------------------------------------------------------------------------
  // Monitor event processing time for the failed (force-accepted) event
  //----------------------------------------------------------------------------
  auto eventTime = std::chrono::steady_clock::now() - m_eventTimerStartPoint[eventContext.slot()];
  int64_t eventTimeMillisec = std::chrono::duration_cast<std::chrono::milliseconds>(eventTime).count();
  auto monTimeAny = Monitored::Scalar<int64_t>("TotalTime", eventTimeMillisec);
  auto monTimeAcc = Monitored::Scalar<int64_t>("TotalTimeAccepted", eventTimeMillisec);
  Monitored::Group(m_monTool, monTimeAny, monTimeAcc);

  //----------------------------------------------------------------------------
  // Try to build and send the output
  //----------------------------------------------------------------------------
  if (m_outputCnvSvc->connectOutput("").isFailure()) {
    ATH_MSG_ERROR("The output conversion service failed in connectOutput() while handling a failed event. "
                  << "Cannot force-accept this event from HLT side, will rely on data collector to do this. "
                  << "The event loop will exit after all ongoing processing is finished.");
    return returnFailureAndStopEventLoop();
  }

  DataObject* hltResultDO = m_evtStore->accessData(hltResultWH.clid(),hltResultWH.key());
  if (hltResultDO == nullptr) {
    if (errorCode == HLT::OnlineErrorCode::NO_HLT_RESULT) {
      // Avoid infinite loop
      ATH_MSG_ERROR("Second failure to build or record the HLT Result in event store while handling a failed event. "
                    << "Cannot force-accept this event from HLT side, will rely on data collector to do this. "
                    << "The event loop will exit after all ongoing processing is finished.");
      return returnFailureAndStopEventLoop();
    }
    ATH_MSG_ERROR("Failed to retrieve DataObject for the HLT result object while handling a failed event. "
                  << "Trying again with skipped filling of the result contents (except debug stream tag).");
    return failedEvent(HLT::OnlineErrorCode::NO_HLT_RESULT,eventContext);
  }

  IOpaqueAddress* addr = nullptr;
  if (m_outputCnvSvc->createRep(hltResultDO,addr).isFailure() || addr == nullptr) {
    ATH_MSG_ERROR("Conversion of HLT result object to the output format failed while handling a failed event. "
                  << "Cannot force-accept this event from HLT side, will rely on data collector to do this. "
                  << "The event loop will exit after all ongoing processing is finished.");
    delete addr;
    return returnFailureAndStopEventLoop();
  }

  if (m_outputCnvSvc->commitOutput("",true).isFailure()) {
    ATH_MSG_ERROR("The output conversion service failed in commitOutput() while handling a failed event. "
                  << "Cannot force-accept this event from HLT side, will rely on data collector to do this. "
                  << "The event loop will exit after all ongoing processing is finished.");
    delete addr;
    return returnFailureAndStopEventLoop();
  }

  // The output has been sent out, the ByteStreamAddress can be deleted
  delete addr;

  //------------------------------------------------------------------------
  // Reset the timeout flag and the timer, and mark the slot as idle
  //------------------------------------------------------------------------
  resetEventTimer(eventContext, /*processing=*/ false);

  //----------------------------------------------------------------------------
  // Clear the event data slot
  //----------------------------------------------------------------------------
  // Need to copy the event context because it's managed by the event store and clearWBSlot deletes it
  const EventContext eventContextCopy = eventContext;
  if (clearWBSlot(eventContext.slot()).isFailure())
    return failedEvent(HLT::OnlineErrorCode::AFTER_RESULT_SENT,eventContextCopy);

  // Only now after store clearing we can allow the slot to be filled again,
  // so we increment m_freeSlots and notify the input thread
  ++m_freeSlots;
  if (!m_loopStatus.loopEnded && m_inputThread!=nullptr) {
    m_inputThread->cond().notify_all();
  }

  //----------------------------------------------------------------------------
  // Finish handling the failed event
  //----------------------------------------------------------------------------

  // Unless this is an event data or algorithm processing failure, increment the number of framework failures
  if (!HLT::isEventProcessingErrorCode(errorCode)) {
    if ( m_maxFrameworkErrors.value()>=0 && ((++m_nFrameworkErrors)>m_maxFrameworkErrors.value()) ) {
      ATH_MSG_ERROR("Failure with OnlineErrorCode=" << errorCode
        << " was successfully handled, but the number of tolerable framework errors for this HltEventLoopMgr instance,"
        << " which is " << m_maxFrameworkErrors.value() << ", was exceeded. Current local event number is "
        << eventContextCopy.evt() << ", slot " << eventContextCopy.slot()
        << ". The event loop will exit after all ongoing processing is finished.");
      return returnFailureAndStopEventLoop();
    }
  }

  // Even if handling the failed event succeeded, print an error message with failed event details
  ATH_MSG_ERROR("Failed event with OnlineErrorCode=" << errorCode
    << " Current local event number is " << eventContextCopy.evt() << ", slot " << eventContextCopy.slot());

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS; // error handling succeeded, event loop may continue
}

// =============================================================================
void HltEventLoopMgr::eventTimerCallback()
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);
  auto now=std::chrono::steady_clock::now();
  for (size_t i=0; i<m_eventTimerStartPoint.size(); ++i) {
    // iterate over all slots and check for timeout
    if (!m_isSlotProcessing.at(i)) continue;
    if (now > m_eventTimerStartPoint.at(i) + m_softTimeoutValue) {
      EventContext ctx(0,i); // we only need the slot number for Athena::Timeout instance
      // don't duplicate the actions if the timeout was already reached
      if (!Athena::Timeout::instance(ctx).reached()) {
        ATH_MSG_ERROR("Soft timeout in slot " << i << ". Processing time exceeded the limit of " << m_softTimeoutValue.count() << " ms");
        setTimeout(Athena::Timeout::instance(ctx));
        // Generate stack trace and scheduler dump only once, on the first timeout
        if (m_traceOnTimeout.value() && !m_timeoutTraceGenerated) {
          m_schedulerSvc->dumpState();
          ATH_MSG_INFO("Generating stack trace due to the soft timeout");
          m_timeoutTraceGenerated = true;
          gSystem->StackTrace();
        }
      }
    }
  }
  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
}

// =============================================================================
void HltEventLoopMgr::resetEventTimer(const EventContext& eventContext, bool processing) {
  if (!eventContext.valid()) {return;}
  {
    std::unique_lock<std::mutex> lock(m_timeoutThread->mutex());
    m_eventTimerStartPoint[eventContext.slot()] = std::chrono::steady_clock::now();
    m_isSlotProcessing[eventContext.slot()] = processing;
    resetTimeout(Athena::Timeout::instance(eventContext));
  }
  m_timeoutThread->cond().notify_all();
}

// =============================================================================
StatusCode HltEventLoopMgr::clearWBSlot(size_t evtSlot) const
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);
  auto monTime = Monitored::Timer<std::chrono::duration<float, std::milli>>("TIME_clearStore");
  StatusCode sc = m_whiteboard->clearStore(evtSlot);
  Monitored::Group(m_monTool, monTime);
  if( !sc.isSuccess() )  {
    ATH_MSG_WARNING("Clear of event data store failed");
  }
  ATH_MSG_VERBOSE("end of " << __FUNCTION__ << ", returning m_whiteboard->freeStore(evtSlot=" << evtSlot << ")");
  return m_whiteboard->freeStore(evtSlot);
}

// =============================================================================
void HltEventLoopMgr::inputThreadCallback() {
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);
  if (m_loopStatus.loopEnded) {
    ATH_MSG_VERBOSE("Event loop ended, stopping the input thread and returning from " << __FUNCTION__);
    m_inputThread->stop();
    // Notify output thread which may be still waiting for events
    m_outputThread->cond().notify_all();
    return;
  }

  // Early exit conditions
  if (!m_loopStatus.eventsAvailable) {
    ATH_MSG_VERBOSE("No more events, flagging the event loop as finished, stopping the input thread"
                    << " and returning from " << __FUNCTION__);
    m_inputThread->stop();
    // Notify output thread which may be still waiting for events
    m_outputThread->cond().notify_all();
    return;
  }
  const size_t numSlotsToFill = m_freeSlots.load();
  if (numSlotsToFill==0) {
    ATH_MSG_VERBOSE("No free slots, returning from " << __FUNCTION__);
    return;
  }
  m_freeSlots -= numSlotsToFill;

  // Read in and start processing another event
  ATH_MSG_DEBUG("Free slots = " << numSlotsToFill << ". Reading new event(s) to fill the slot(s).");

  // Fill all free slots with new events
  for (size_t i=0; i<numSlotsToFill; ++i) {
    auto task = [mgr=this](){
      StatusCode sc = StatusCode::SUCCESS;
      try {
        sc = mgr->startNextEvent();
      }
      catch (const std::exception& e) {
        mgr->error() << "Exception caught in startNextEvent: " << e.what() << endmsg;
        sc = StatusCode::FAILURE;
      }
      catch (...) {
        mgr->error() << "Exception caught in startNextEvent" << endmsg;
        sc = StatusCode::FAILURE;
      }
      if (sc.isFailure()) {
        mgr->error() << "startNextEvent failed, stopping the event loop" << endmsg;
        mgr->m_loopStatus.exitCode = StatusCode::FAILURE;
        mgr->m_loopStatus.eventsAvailable = false;
        return;
      }
      // Pop one item from parallel I/O queue to decrement its size - it doesn't matter which item
      // is popped, we only use the queue size to limit the number of tasks running in parallel
      bool popIOQueue{false};
      mgr->m_parallelIOQueue.pop(popIOQueue);
    };

    // Push one item to the parallel I/O queue to increment its size - the value doesn't matter,
    // we only use the queue size and benefit from the blocking push call here to limit the number
    // of tasks running in parallel. Once we can push to the queue, we can schedule the task.
    m_parallelIOQueue.push(true);
    m_parallelIOTaskGroup.run(std::move(task));
  }
  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
}

// =============================================================================
void HltEventLoopMgr::outputThreadCallback() {
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);
  const size_t nslots = m_isSlotProcessing.size(); // size is fixed in hltUpdateAfterFork after configuring scheduler
  if (m_schedulerSvc->freeSlots() == nslots) {
    if (m_loopStatus.eventsAvailable) {
      ATH_MSG_DEBUG("There are currently no events being processed by the Scheduler, returning from " << __FUNCTION__);
    } else if (m_freeSlots == nslots) {
      ATH_MSG_DEBUG("No more events to process and scheduler is empty, stopping the event loop and output thread");
      if (!m_loopStatus.loopEnded && m_outputThread!=nullptr) {
        m_outputThread->stop();
      }
      // Notify input thread which may be still waiting for free slots
      if (!m_loopStatus.loopEnded && m_inputThread!=nullptr) {
        m_inputThread->cond().notify_all();
      }
      // Notify the main thread that the loop ended - this is the only place able to do this!
      m_loopStatus.loopEnded = true;
      m_loopStatus.loopEndedCond.notify_all();
    }
    else{
      ATH_MSG_DEBUG("No more events, but processing is still ongoing, returning from " << __FUNCTION__);
    }
    return;
  }

  //----------------------------------------------------------------------------
  // Pop events from the Scheduler
  //----------------------------------------------------------------------------
  std::vector<EventContext*> finishedEvtContexts;
  EventContext* finishedEvtContext(nullptr);
  const auto popStartTime = std::chrono::steady_clock::now();

  // Pop one event from the scheduler (blocking call)
  ATH_MSG_DEBUG("Waiting for a finished event from the Scheduler");
  if (m_schedulerSvc->popFinishedEvent(finishedEvtContext).isFailure()) {
    failedEvent(HLT::OnlineErrorCode::SCHEDULER_POP_FAILURE, EventContext()).ignore();
    delete finishedEvtContext;
    return;
  }
  ATH_MSG_DEBUG("Scheduler returned a finished event: " << finishedEvtContext);
  finishedEvtContexts.push_back(finishedEvtContext);

  // See if more events are available (non-blocking call)
  while (m_schedulerSvc->tryPopFinishedEvent(finishedEvtContext).isSuccess()){
    ATH_MSG_DEBUG("Scheduler returned a finished event: " << *finishedEvtContext);
    finishedEvtContexts.push_back(finishedEvtContext);
  }
  const auto popSpentTime = std::chrono::steady_clock::now() - popStartTime;
  const auto popSpentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(popSpentTime).count();
  Monitored::Scalar<int64_t> monPopSchedulerTime{"PopSchedulerTime", popSpentTimeMs};
  Monitored::Scalar<size_t> monPopSchedulerNumEvt{"PopSchedulerNumEvt", finishedEvtContexts.size()};
  Monitored::Group{m_monTool, monPopSchedulerNumEvt, monPopSchedulerTime};

  //----------------------------------------------------------------------------
  // Post-process the finished events
  //----------------------------------------------------------------------------
  const size_t nFinishedEvents = finishedEvtContexts.size();
  ATH_MSG_DEBUG("Number of finished events to post-process: " << nFinishedEvents);

  // Push all post-processing tasks to TBB
  for (EventContext* thisFinishedEvtContext : finishedEvtContexts) {
    // Reset free slot timer for monitoring
    if (thisFinishedEvtContext != nullptr) {
      m_freeSlotStartPoint[thisFinishedEvtContext->slot()] = std::chrono::steady_clock::now();
    }

    // Create and enqueue the task
    m_finishedEventsQueue.push(thisFinishedEvtContext);
    auto task = [mgr=this](){
      StatusCode sc = StatusCode::SUCCESS;
      try {
        sc = mgr->processFinishedEvent();
      }
      catch (const std::exception& e) {
        mgr->error() << "Exception caught in processFinishedEvent: " << e.what() << endmsg;
        sc = StatusCode::FAILURE;
      }
      catch (...) {
        mgr->error() << "Exception caught in processFinishedEvent" << endmsg;
        sc = StatusCode::FAILURE;
      }

      if (sc.isFailure()) {
        mgr->error() << "processFinishedEvent failed, stopping the event loop" << endmsg;
        mgr->m_loopStatus.exitCode = StatusCode::FAILURE;
        mgr->m_loopStatus.eventsAvailable = false;
      }

      // Pop one item from parallel I/O queue to decrement its size - it doesn't matter which item
      // is popped, we only use the queue size to limit the number of tasks running in parallel
      bool popIOQueue{false};
      mgr->m_parallelIOQueue.pop(popIOQueue);

      // Wake up the output thread if it's sleeping - this prevents a deadlock after the last event
      // when input thread already finished and is no longer waking up the output thread. Spurious wake-ups
      // during the event loop from this notification should have negligible effect on CPU load.
      mgr->m_outputThread->cond().notify_one();
    };

    // Push one item to the parallel I/O queue to increment its size - the value doesn't matter,
    // we only use the queue size and benefit from the blocking push call here to limit the number
    // of tasks running in parallel. Once we can push to the queue, we can schedule the task.
    m_parallelIOQueue.push(true);
    m_parallelIOTaskGroup.run(std::move(task));
  }

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
}

// =============================================================================
StatusCode HltEventLoopMgr::startNextEvent()
{
  StatusCode sc = StatusCode::SUCCESS;
  auto check = [this, &sc](std::string&& errmsg, HLT::OnlineErrorCode errcode, const EventContext& eventContext) {
    if (sc.isSuccess()) {return false;}
    ATH_MSG_ERROR(errmsg);
    sc = failedEvent(errcode, eventContext);
    Gaudi::Hive::setCurrentContext(EventContext());
    return true;
  };

  //------------------------------------------------------------------------
  // Allocate event slot and create new EventContext
  //------------------------------------------------------------------------

  // Create an EventContext, allocating and selecting a whiteboard slot
  std::unique_ptr<EventContext> eventContextPtr = std::make_unique<EventContext>(createEventContext());

  sc = eventContextPtr->valid() ? StatusCode(StatusCode::SUCCESS) : StatusCode(StatusCode::FAILURE);
  if (check("Failed to allocate slot for a new event", HLT::OnlineErrorCode::BEFORE_NEXT_EVENT, *eventContextPtr)) {
    return sc;
  }

  sc = m_whiteboard->selectStore(eventContextPtr->slot());
  if (check("Failed to select event store slot number " + std::to_string(eventContextPtr->slot()),
            HLT::OnlineErrorCode::BEFORE_NEXT_EVENT, *eventContextPtr)) {
    return sc;
  }

  // We can completely avoid using ThreadLocalContext if we store the EventContext in the event store. Any
  // service/tool method which does not allow to pass EventContext as argument, can const-retrieve it from the
  // event store rather than using ThreadLocalContext.

  // We link the current store in the extension of the EventContext we just created. Only then we create
  // a WriteHandle for the EventContext using the EventContext itself. The handle will use the linked hiveProxyDict
  // to record the context in the current store.
  eventContextPtr->setExtension(Atlas::ExtendedEventContext(m_evtStore->hiveProxyDict(),
                                                            m_currentRunCtx.eventID().run_number()));
  auto eventContext = SG::makeHandle(m_eventContextWHKey,*eventContextPtr);
  sc = eventContext.record(std::move(eventContextPtr));
  if (check("Failed to record new EventContext in the event store",
            HLT::OnlineErrorCode::BEFORE_NEXT_EVENT, *eventContext)) {
    return sc;
  }

  // Reset the AlgExecStateSvc
  m_aess->reset(*eventContext);

  ATH_MSG_DEBUG("Created new EventContext with number: " << eventContext->evt()
                << ", slot: " << eventContext->slot());

  // This ThreadLocalContext call is a not-so-nice behind-the-scenes way to inform some services about the current
  // context. If possible, services should use EventContext from the event store as recorded above. We have to set
  // the ThreadLocalContext here because some services still use it.
  Gaudi::Hive::setCurrentContext(*eventContext);

  //------------------------------------------------------------------------
  // Create a new address for EventInfo to facilitate automatic conversion from input data
  //------------------------------------------------------------------------
  IOpaqueAddress* addr = nullptr;
  sc = m_evtSelector->createAddress(*m_evtSelContext, addr);
  if (check("Event selector failed to create an IOpaqueAddress",
            HLT::OnlineErrorCode::BEFORE_NEXT_EVENT, *eventContext)) {
    return sc;
  }

  //------------------------------------------------------------------------
  // Get the next event
  //------------------------------------------------------------------------
  try {
    bool noEventsTemporarily{false};
    do {
      try {
        noEventsTemporarily = false;
        sc = m_evtSelector->next(*m_evtSelContext);
      } catch (const hltonl::Exception::NoEventsTemporarily& e) {
        ATH_MSG_DEBUG("No new input events available temporarily, requesting again");
        noEventsTemporarily = true;
      }
    } while (noEventsTemporarily);
  }
  catch (const hltonl::Exception::NoMoreEvents& e) {
    sc = StatusCode::SUCCESS;
    m_loopStatus.eventsAvailable = false;
    sc = clearWBSlot(eventContext->slot());
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Failed to clear the whiteboard slot " << eventContext->slot()
                      << " after NoMoreEvents detected");
    }
    // Increment m_freeSlots after clearing the store and notify the input thread
    ++m_freeSlots;
    if (!m_loopStatus.loopEnded && m_inputThread!=nullptr) {
      m_inputThread->cond().notify_all();
    }
    return StatusCode::SUCCESS;
  }
  catch (const hltonl::Exception::MissingCTPFragment& e) {
    sc = StatusCode::FAILURE;
    if (check(e.what(), HLT::OnlineErrorCode::MISSING_CTP_FRAGMENT, *eventContext)) {
      return sc;
    }
  }
  catch (const hltonl::Exception::BadCTPFragment& e) {
    sc = StatusCode::FAILURE;
    if (check(e.what(), HLT::OnlineErrorCode::BAD_CTP_FRAGMENT, *eventContext)) {
      return sc;
    }
  }
  catch (const std::exception& e) {
    ATH_MSG_ERROR("Failed to get next event from the event source, std::exception caught: " << e.what());
    sc = StatusCode::FAILURE;
  }
  catch (...) {
    ATH_MSG_ERROR("Failed to get next event from the event source, unknown exception caught");
    sc = StatusCode::FAILURE;
  }
  if (check("Failed to get the next event",
            HLT::OnlineErrorCode::CANNOT_RETRIEVE_EVENT, *eventContext)) {
    return sc;
  }

  //------------------------------------------------------------------------
  // Reset the timeout flag and the timer, and mark the slot as busy
  //------------------------------------------------------------------------
  resetEventTimer(*eventContext, /*processing=*/ true);

  //------------------------------------------------------------------------
  // Load event proxies and get event info
  //------------------------------------------------------------------------
  sc = m_evtStore->loadEventProxies();
  if (check("Failed to load event proxies", HLT::OnlineErrorCode::NO_EVENT_INFO, *eventContext)) {
    return sc;
  }

  auto eventInfo = SG::makeHandle(m_eventInfoRHKey,*eventContext);
  sc = eventInfo.isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE;
  if (check("Failed to retrieve EventInfo", HLT::OnlineErrorCode::NO_EVENT_INFO, *eventContext)) {
    return sc;
  }

  ATH_MSG_DEBUG("Retrieved event info for the new event " << *eventInfo);

  // Set EventID for the EventContext
  EventID eid = eventIDFromxAOD(eventInfo.cptr());
  // Override run/LB/timestamp if needed
  if (m_forceRunNumber > 0) {
    eid.set_run_number(m_forceRunNumber);
  }
  if (m_forceLumiblock > 0) {
    eid.set_lumi_block(m_forceLumiblock);
  }
  if (m_forceSOR_ns > 0) {
    eid.set_time_stamp(m_forceSOR_ns / std::nano::den);
    eid.set_time_stamp_ns_offset(m_forceSOR_ns % std::nano::den);
  }
  eventContext->setEventID(eid);

  // Update thread-local EventContext after setting EventID
  Gaudi::Hive::setCurrentContext(*eventContext);

  //-----------------------------------------------------------------------
  // COOL updates for LB changes
  //-----------------------------------------------------------------------

  // Check if this is a new LB
  EventIDBase::number_type oldMaxLB{0}, newMaxLB{0};
  bool updatedLB{false};
  do {
    oldMaxLB = m_loopStatus.maxLB.load();
    newMaxLB = std::max(oldMaxLB, eventContext->eventID().lumi_block());
    updatedLB = newMaxLB > oldMaxLB;
  } while (updatedLB && !m_loopStatus.maxLB.compare_exchange_strong(oldMaxLB, newMaxLB));
  m_loopStatus.maxLB.compare_exchange_strong(oldMaxLB, newMaxLB);

  // Wait in case a COOL update is ongoing to avoid executeEvent
  // reading conditions data while they are being updated.
  {
    std::unique_lock<std::mutex> lock(m_loopStatus.coolUpdateMutex);
    m_loopStatus.coolUpdateCond.wait(lock, [&]{return !m_loopStatus.coolUpdateOngoing;});
  }

  // Do COOL updates (if needed) and notify other threads about it
  if (updatedLB) {
    {
      std::lock_guard<std::mutex> lock(m_loopStatus.coolUpdateMutex);
      m_loopStatus.coolUpdateOngoing = true;
      sc = m_coolHelper->hltCoolUpdate(*eventContext);
      if (check("Failure during COOL update", HLT::OnlineErrorCode::COOL_UPDATE, *eventContext)) {
        m_loopStatus.coolUpdateOngoing = false;
        return sc;
      }
      m_loopStatus.coolUpdateOngoing = false;
    }
    m_loopStatus.coolUpdateCond.notify_all();
  }

  //------------------------------------------------------------------------
  // Process the event
  //------------------------------------------------------------------------
  // We need to make a copy of eventContext, as executeEvent uses move semantics and eventContext is already owned
  // by the event store. The copy we create here is pushed to the scheduler and retrieved back in drainScheduler
  // where we have to delete it.
  sc = executeEvent(EventContext(*eventContext));
  if (check("Failed to schedule event processing",
            HLT::OnlineErrorCode::SCHEDULING_FAILURE, *eventContext)) {
    return sc;
  }
  // Notify the output thread to start waiting for a finished event
  m_outputThread->cond().notify_one();

  //------------------------------------------------------------------------
  // Set ThreadLocalContext to an invalid context
  //------------------------------------------------------------------------
  // We have passed the event to the scheduler and we are entering back a context-less environment
  Gaudi::Hive::setCurrentContext( EventContext() );

  return sc;
}

// =============================================================================
StatusCode HltEventLoopMgr::processFinishedEvent()
{
  EventContext* eventContext{nullptr};
  m_finishedEventsQueue.pop(eventContext);

  StatusCode sc = StatusCode::SUCCESS;
  auto check = [this, &sc, &eventContext](std::string&& errmsg, HLT::OnlineErrorCode errcode) {
    if (sc.isSuccess()) {return false;}
    ATH_MSG_ERROR(errmsg);
    const EventContext& eventContextRef = (eventContext==nullptr) ? EventContext() : *eventContext;
    sc = failedEvent(errcode, eventContextRef);
    Gaudi::Hive::setCurrentContext(EventContext());
    delete eventContext;
    return true;
  };

  //--------------------------------------------------------------------------
  // Basic checks, select slot, retrieve event info
  //--------------------------------------------------------------------------
  // Check if the EventContext object exists
  if (eventContext == nullptr) {
    sc = StatusCode::FAILURE;
    if (check("Detected nullptr EventContext while finalising a processed event",
              HLT::OnlineErrorCode::CANNOT_ACCESS_SLOT)) {
      return sc;
    }
  }

  // Set ThreadLocalContext to the currently processed finished context
  Gaudi::Hive::setCurrentContext(eventContext);

  // Check the event processing status
  if (m_aess->eventStatus(*eventContext) != EventStatus::Success) {
    sc = StatusCode::FAILURE;
    auto algErrors = m_errorMonTool->algExecErrors(*eventContext);
    const HLT::OnlineErrorCode errCode = isTimedOut(algErrors) ?
                                         HLT::OnlineErrorCode::TIMEOUT : HLT::OnlineErrorCode::PROCESSING_FAILURE;
    if (check("Processing event with context " + toString(*eventContext) + \
              " failed with status " + toString(m_aess->eventStatus(*eventContext)),
              errCode)) {
      return sc;
    }
  }

  // Select the whiteboard slot
  sc = m_whiteboard->selectStore(eventContext->slot());
  if (check("Failed to select event store slot " + std::to_string(eventContext->slot()),
            HLT::OnlineErrorCode::CANNOT_ACCESS_SLOT)) {
    return sc;
  }

  // Fire EndProcessing incident - some services may depend on this
  m_incidentSvc->fireIncident(Incident(name(), IncidentType::EndProcessing, *eventContext));

  //--------------------------------------------------------------------------
  // HLT output handling
  //--------------------------------------------------------------------------
  // Call the result builder to record HLTResultMT in SG
  sc = m_hltResultMaker->makeResult(*eventContext);
  if (check("Failed to create the HLT result object", HLT::OnlineErrorCode::NO_HLT_RESULT)) {return sc;}

  // Connect output (create the output container) - the argument is currently not used
  sc = m_outputCnvSvc->connectOutput("");
  if (check("Conversion service failed to connectOutput", HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {return sc;}

  // Retrieve the HLT result and the corresponding DataObject
  auto hltResult = SG::makeHandle(m_hltResultRHKey,*eventContext);
  if (!hltResult.isValid()) {sc = StatusCode::FAILURE;}
  if (check("Failed to retrieve the HLT result", HLT::OnlineErrorCode::NO_HLT_RESULT)) {return sc;}

  DataObject* hltResultDO = m_evtStore->accessData(hltResult.clid(),hltResult.key());
  if (hltResultDO == nullptr) {sc = StatusCode::FAILURE;}
  if (check("Failed to retrieve the HLTResult DataObject", HLT::OnlineErrorCode::NO_HLT_RESULT)) {return sc;}

  // Check for result truncation
  if (!hltResult->getTruncatedModuleIds().empty() && hltResult->severeTruncation()) {sc = StatusCode::FAILURE;}
  if (check("HLT result truncation", HLT::OnlineErrorCode::RESULT_TRUNCATION)) {return sc;}

  // Convert the HLT result to the output data format
  IOpaqueAddress* addr = nullptr;
  sc = m_outputCnvSvc->createRep(hltResultDO,addr);
  if (sc.isFailure()) {delete addr;}
  if (check("Conversion service failed to convert HLTResult", HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {return sc;}

  // Retrieve and convert the L1 result to the output data format
  IOpaqueAddress* l1addr = nullptr;
  IOpaqueAddress* l1addrLegacy = nullptr;
  if (m_rewriteLVL1) {
    // Run-3 L1 simulation result
    if (not m_l1TriggerResultRHKey.empty()) {
      auto l1TriggerResult = SG::makeHandle(m_l1TriggerResultRHKey, *eventContext);
      if (!l1TriggerResult.isValid()) {sc = StatusCode::FAILURE;}
      if (check("Failed to retrieve the L1 Trigger Result for RewriteLVL1",
                HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {
        return sc;
      }

      DataObject* l1TriggerResultDO = m_evtStore->accessData(l1TriggerResult.clid(),l1TriggerResult.key());
      if (l1TriggerResultDO == nullptr) {sc = StatusCode::FAILURE;}
      if (check("Failed to retrieve the L1 Trigger Result DataObject for RewriteLVL1",
                HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {
        return sc;
      }

      sc = m_outputCnvSvc->createRep(l1TriggerResultDO,l1addr);
      if (sc.isFailure()) {delete l1addr;}
      if (check("Conversion service failed to convert L1 Trigger Result for RewriteLVL1",
                HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {
        return sc;
      }
    }
    // Legacy (Run-2) L1 simulation result
    if (not m_roibResultRHKey.empty()) {
      auto roibResult = SG::makeHandle(m_roibResultRHKey, *eventContext);
      if (!roibResult.isValid()) {sc = StatusCode::FAILURE;}
      if (check("Failed to retrieve the RoIBResult for RewriteLVL1",
                HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {
        return sc;
      }

      DataObject* roibResultDO = m_evtStore->accessData(roibResult.clid(),roibResult.key());
      if (roibResultDO == nullptr) {sc = StatusCode::FAILURE;}
      if (check("Failed to retrieve the RoIBResult DataObject for RewriteLVL1",
                HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {
        return sc;
      }

      sc = m_outputCnvSvc->createRep(roibResultDO,l1addrLegacy);
      if (sc.isFailure()) {delete l1addrLegacy;}
      if (check("Conversion service failed to convert RoIBResult for RewriteLVL1",
                HLT::OnlineErrorCode::OUTPUT_BUILD_FAILURE)) {
        return sc;
      }
    }
  }

  // Save event processing time before sending output
  bool eventAccepted = !hltResult->getStreamTags().empty();
  auto eventTime = std::chrono::steady_clock::now() - m_eventTimerStartPoint[eventContext->slot()];
  int64_t eventTimeMillisec = std::chrono::duration_cast<std::chrono::milliseconds>(eventTime).count();

  // Commit output (write/send the output data) - the arguments are currently not used
  sc = m_outputCnvSvc->commitOutput("",true);
  if (sc.isFailure()) {delete addr;}
  if (check("Conversion service failed to commitOutput", HLT::OnlineErrorCode::OUTPUT_SEND_FAILURE)) {return sc;}

  // The output has been sent out, the ByteStreamAddress can be deleted
  delete addr;
  delete l1addr;
  delete l1addrLegacy;

  //------------------------------------------------------------------------
  // Reset the timeout flag and the timer, and mark the slot as idle
  //------------------------------------------------------------------------
  resetEventTimer(*eventContext, /*processing=*/ false);

  //--------------------------------------------------------------------------
  // Clear the slot
  //--------------------------------------------------------------------------
  ATH_MSG_DEBUG("Clearing slot " << eventContext->slot()
                << " (event " << eventContext->evt() << ") of the whiteboard");

  sc = clearWBSlot(eventContext->slot());
  if (check("Whiteboard slot " + std::to_string(eventContext->slot()) + " could not be properly cleared",
            HLT::OnlineErrorCode::AFTER_RESULT_SENT)) {
    return sc;
  }

  ATH_MSG_DEBUG("Finished processing " << (eventAccepted ? "accepted" : "rejected")
                << " event with context " << *eventContext
                << " which took " << eventTimeMillisec << " ms");

  // Only now after store clearing we can allow the slot to be filled again,
  // so we increment m_freeSlots and notify the input thread
  ++m_freeSlots;
  if (!m_loopStatus.loopEnded && m_inputThread!=nullptr) {
    m_inputThread->cond().notify_all();
  }

  // Fill the time monitoring histograms
  auto monTimeAny = Monitored::Scalar<int64_t>("TotalTime", eventTimeMillisec);
  auto monTimeAcc = Monitored::Scalar<int64_t>(eventAccepted ? "TotalTimeAccepted" : "TotalTimeRejected", eventTimeMillisec);
  Monitored::Group(m_monTool, monTimeAny, monTimeAcc);

  // Set ThreadLocalContext to an invalid context as we entering a context-less environment
  Gaudi::Hive::setCurrentContext( EventContext() );

  // Delete the EventContext which was created when calling executeEvent( EventContext(*eventContext) )
  delete eventContext;

  return StatusCode::SUCCESS;
}
