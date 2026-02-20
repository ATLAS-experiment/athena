/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "WebdaqHistSvc.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaInterprocess/Incidents.h"

#include "CxxUtils/checker_macros.h"
#include "AthenaMonitoringKernel/OHLockedHist.h"

#include "hltinterface/IInfoRegister.h"
#include "webdaq/webdaq-root.hpp"
#include "webdaq/webdaq.hpp"

#include "TError.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TObject.h"
#include "TROOT.h"
#include "TTree.h"
#include <TBufferJSON.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <cstdlib> 

WebdaqHistSvc::WebdaqHistSvc(const std::string& name, ISvcLocator* svc) : base_class(name, svc)
{}

/**************************************************************************************/

StatusCode WebdaqHistSvc::initialize ATLAS_NOT_THREAD_SAFE()
{
  // Protect against multiple instances of TROOT
  if (0 == gROOT) {
    static TROOT root("root", "ROOT I/O");
  }
  else {
    ATH_MSG_VERBOSE("ROOT already initialized, debug = " << gDebug);
  }

  gErrorIgnoreLevel = kBreak; // ignore warnings see TError.h in ROOT base src

  // compile regexes
  m_excludeTypeRegex = boost::regex(m_excludeType.value());
  m_includeTypeRegex = boost::regex(m_includeType.value());
  m_excludeNameRegex = boost::regex(m_excludeName.value());
  m_includeNameRegex = boost::regex(m_includeName.value());
  m_PublicationIncludeNameRegex = boost::regex(m_PublicationIncludeName.value());
  m_fastPublicationIncludeNameRegex = boost::regex(m_fastPublicationIncludeName.value());

  // Retrieve and set OH mutex
  ATH_MSG_INFO("Enabling use of OH histogram mutex");
  static std::mutex mutex; 
  oh_lock_histogram_mutex::set_histogram_mutex(mutex);
  ATH_CHECK( m_jobOptionsSvc.retrieve() );
  
  //Retireve enviroment variables
  const char* tdaq_partition_cstr = std::getenv("TDAQ_PARTITION");
  if (tdaq_partition_cstr != nullptr) {
    m_partition = std::string(tdaq_partition_cstr);
    ATH_MSG_INFO("Partition: " << m_partition);
  } else {
    ATH_MSG_ERROR("TDAQ_PARTITION environment variable not set");
    return StatusCode::FAILURE;
  }
  const char* tdaqWebdaqBase_cstr = std::getenv("TDAQ_WEBDAQ_BASE");
  if (tdaqWebdaqBase_cstr != nullptr) {
    m_tdaqWebdaqBase = std::string(tdaqWebdaqBase_cstr);
    ATH_MSG_INFO("TDAQ_WEBDAQ_BASE value: " << m_tdaqWebdaqBase);
  } else {
    ATH_MSG_ERROR("TDAQ_WEBDAQ_BASE environment variable not set! Is needed for the OH publication through webdaq");
    return StatusCode::FAILURE;
  }
  const char* tdaq_oh_server = std::getenv("TDAQ_OH_SERVER");
  if (tdaq_oh_server != nullptr) {
    m_tdaqOHServerName = std::string(tdaq_oh_server);
  } else {
    m_tdaqOHServerName = m_OHServerName.value();
  } 
  ATH_MSG_INFO("TDAQ_OH_SERVER value: " << m_tdaqOHServerName);
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
  ATH_CHECK( incSvc.retrieve() );
  incSvc->addListener(this, AthenaInterprocess::UpdateAfterFork::type());
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

void WebdaqHistSvc::handle(const Incident& incident)
{
  if (incident.type() == AthenaInterprocess::UpdateAfterFork::type()) {
    ATH_MSG_INFO("Going to initialize the monitoring Thread"); 
    m_thread = std::thread( &WebdaqHistSvc::monitoringTask, this, m_numSlots, m_intervalSeconds, std::ref(m_histoMapUpdated), m_PublicationIncludeNameRegex);
    m_threadFast = std::thread( &WebdaqHistSvc::monitoringTask, this, m_numSlotsFast, m_intervalSecondsFast, std::ref(m_histoMapUpdatedFast), m_fastPublicationIncludeNameRegex);
  }
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::stop()
{
  /// Set the stop flag for the task thread to true
  ATH_MSG_DEBUG("Stopping monitoring task");
  m_stopFlag = true; 
  // Wait for the task to finish
  if (m_thread.joinable()) {
    ATH_MSG_DEBUG("Going to join the monitoring thread");
    try {
        m_thread.join();
    } 
    catch (const std::exception& e) {
      ATH_MSG_ERROR("Failed to join the monitoring thread: " << e.what());
      return StatusCode::FAILURE;
    }
  }
  if (m_threadFast.joinable()) {
    ATH_MSG_DEBUG("Going to join the fast monitoring thread");
    try {
        m_threadFast.join();
    } 
    catch (const std::exception& e) {
      ATH_MSG_ERROR("Failed to join the fast monitoring thread: " << e.what());
      return StatusCode::FAILURE;
    }
  }
  ATH_MSG_DEBUG("Clearing list of histograms");
  m_hists.clear();
  m_histoMapUpdated = true;
  m_histoMapUpdatedFast = true;
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::finalize()
{
  ATH_MSG_INFO("finalize");
  // Reset OH mutex
  oh_lock_histogram_mutex::reset_histogram_mutex();

  return StatusCode::SUCCESS;
}

/**************************************************************************************/

template <typename T>
StatusCode WebdaqHistSvc::regHist_i(std::unique_ptr<T> hist_unique, const std::string& id,
                                      bool shared, THistID*& phid)
{
  ATH_MSG_DEBUG("Registering histogram " << id);

  phid = nullptr;
  if (not isObjectAllowed(id, hist_unique.get())) {
    return StatusCode::FAILURE;
  }

  if (hist_unique->Class()->InheritsFrom(TH1::Class())) {
    
    tbb::concurrent_hash_map<std::string, THistID>::accessor accessor;
    if (m_hists.find(accessor, id)) {
      ATH_MSG_ERROR("Histogram with name " << id << " already registered");
      return StatusCode::FAILURE; 
    } 
    // Element not found, attempt to insert
    if (!m_hists.insert(accessor, id)) {
      ATH_MSG_ERROR("Failed to insert histogram with name " << id);
      return StatusCode::FAILURE;
    }
    T* hist = hist_unique.release();
    m_histoMapUpdated = true;
    m_histoMapUpdatedFast = true;
    accessor->second = THistID(id, hist);
    //finished
    if (shared) accessor->second.mutex = std::make_unique<std::mutex>();
    phid = &accessor->second;
    ATH_MSG_DEBUG((shared ? "Shared histogram " : "Histogram ")
          << hist->GetName() << " registered under " << id << " " << name());
  } else {
    ATH_MSG_ERROR("Cannot register " << hist_unique->ClassName()
                                     << " because it does not inherit from TH1");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
    
/**************************************************************************************/

template <typename T>
LockedHandle<T> WebdaqHistSvc::regShared_i(const std::string& id, std::unique_ptr<T> hist)
{
  LockedHandle<T> lh(nullptr, nullptr);
  
  tbb::concurrent_hash_map<std::string, THistID>::accessor accessor;
  // Check if the histogram is already registered
  if (!m_hists.find(accessor, id)) {
    // No histogram under that id yet
    T* phist = hist.get();
    THistID* phid = nullptr;
    if (regHist_i(std::move(hist), id, true, phid).isSuccess()) {
      if (phid) lh.set(phist, phid->mutex.get());
    }
  }
  else 
  {
    // Histogram already registered under that id
    if (accessor->second.mutex == nullptr) {
      ATH_MSG_ERROR("regShared: previously registered histogram \"" << id
                                                                    << "\" was not marked shared");
    }
    T* phist = dynamic_cast<T*>(accessor->second.obj);
    if (phist == nullptr) {
      ATH_MSG_ERROR("regShared: unable to dcast retrieved shared hist \""
                    << id << "\" of type " << accessor->second.obj->IsA()->GetName()
                    << " to requested type " << System::typeinfoName(typeid(T)));
    }
    else {
      lh.set(phist, accessor->second.mutex.get());
      //hist is automatically deleted at end of method
    }
  }
  return lh;
}


/**************************************************************************************/

template <typename T>
T* WebdaqHistSvc::getHist_i(const std::string& id, const size_t& /*ind*/, bool quiet) const
{
  ATH_MSG_DEBUG("Getting histogram " << id);

  tbb::concurrent_hash_map<std::string, THistID>::const_accessor accessor;
  if (!m_hists.find(accessor, id) or accessor.empty()) {
    if (!quiet) ATH_MSG_ERROR("could not locate Hist with id \"" << id << "\"");
    return nullptr;
  }

  T* phist = dynamic_cast<T*>(accessor->second.obj);
  if (phist == nullptr) {
    ATH_MSG_ERROR("getHist: unable to dcast retrieved shared hist \""
                  << id << "\" of type " << accessor->second.obj->IsA()->GetName() << " to requested type "
                  << System::typeinfoName(typeid(T)));
    return nullptr;
  }
  return phist;
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::getTHists_i(const std::string& dir, TList& tl) const
{
  for (auto it = m_hists.begin(); it != m_hists.end(); ++it) {
    const std::string& id = it->first;
    const THistID& h = it->second;
    if (id.starts_with(dir)) { // histogram booking path starts from the dir
      tl.Add(h.obj);
    }
  }
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

template <typename T>
LockedHandle<T> WebdaqHistSvc::getShared_i(const std::string& id) const
{
  tbb::concurrent_hash_map<std::string, THistID>::const_accessor accessor;
  if (m_hists.find(accessor, id)) {
    if (accessor->second.mutex == nullptr) {
      ATH_MSG_ERROR("getShared: found Hist with id \"" << id
                                                       << "\", but it's not marked as shared");
      return {};
    }  
    T* phist = dynamic_cast<T*>(accessor->second.obj);
    if (phist == nullptr) {
      ATH_MSG_ERROR("getShared: unable to dcast retrieved shared hist \""
                    << id << "\" of type " << accessor->second.obj->IsA()->GetName()
                    << " to requested type " << System::typeinfoName(typeid(T)));
      return {};
    }
    return LockedHandle<T>(phist, accessor->second.mutex.get());
  }
  ATH_MSG_ERROR("getShared: cannot find histogram with id \"" << id << "\"");
  return {};
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::deReg(TObject* optr)
{
  // Find the relevant histogram and deregister it
  for (auto it = m_hists.begin(); it != m_hists.end(); ++it) {
    if (it->second.obj == optr) {
      ATH_MSG_DEBUG("Found histogram " << optr << " booked under " << it->first
                                       << " and will deregister it");
      return deReg(it->first);
    }
  }
  ATH_MSG_ERROR("Histogram with pointer " << optr << " not found in the histogram map");
  return StatusCode::FAILURE;
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::deReg(const std::string& id)
{
  tbb::concurrent_hash_map<std::string, THistID>::accessor accessor;
  if (m_hists.find(accessor, id)) {
    //Delete the histogram
    accessor->second.obj->Delete();
    m_hists.erase(accessor);
    m_histoMapUpdated = true;
    m_histoMapUpdatedFast = true;
    ATH_MSG_DEBUG("Deregistration of " << id << " done");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_ERROR("Deregistration failed: histogram with id \"" << id << "\" not found");
  return StatusCode::FAILURE;
}

/**************************************************************************************/

std::vector<std::string> WebdaqHistSvc::getHists() const
{
  std::vector<std::string> l;
  l.reserve(m_hists.size());
  for (auto it = m_hists.begin(); it != m_hists.end(); ++it) {
    l.push_back(it->first);
  }
  return l;
}

/**************************************************************************************/

std::set<std::string> WebdaqHistSvc::getSet(boost::regex nameSelect) const
{
  std::vector<std::string> l;
  l.reserve(m_hists.size());
  for (auto it = m_hists.begin(); it != m_hists.end(); ++it) {
    if (boost::regex_match(it->first, nameSelect)) {
      l.push_back(it->first);
    }
  }
  ATH_MSG_DEBUG("Number of histograms matched: " << l.size());
  std::set<std::string> HistoSet(l.begin(), l.end());
  return HistoSet;
}

/**************************************************************************************/

bool WebdaqHistSvc::isObjectAllowed(const std::string& path, const TObject* o) const
{
  boost::cmatch what;

  if (not boost::regex_match(o->ClassName(), what, m_includeTypeRegex)) {
    ATH_MSG_WARNING("Object " << path << " of type " << o->ClassName()
                              << " does NOT match IncludeType \"" << m_includeType << "\"");
    return false;
  }

  if (boost::regex_match(o->ClassName(), what, m_excludeTypeRegex)) {
    ATH_MSG_WARNING("Object " << path << " of type " << o->ClassName() << " matches ExcludeType \""
                              << m_excludeType << "\"");
    return false;
  }

  if (not boost::regex_match(path.c_str(), what, m_includeNameRegex)) {
    ATH_MSG_WARNING("Object " << path << " does NOT match IncludeName \"" << m_includeName << "\"");
    return false;
  }

  if (boost::regex_match(path.c_str(), what, m_excludeNameRegex)) {
    ATH_MSG_WARNING("Object " << path << " matches ExcludeName \"" << m_excludeName << "\"");
    return false;
  }

  return true;
}

bool WebdaqHistSvc::existsHist(const std::string& name) const
{
  return (getHist_i<TH1>(name, 0, true) != nullptr);
}

/**************************************************************************************
 * Typed interface methods
 * All these are just forwarding to the templated xyz_i methods
 **************************************************************************************/
StatusCode WebdaqHistSvc::regHist(const std::string& id)
{
  std::unique_ptr<TH1> hist = nullptr;
  THistID* hid = nullptr;
  return regHist_i(std::move(hist), id, false, hid);
}

StatusCode WebdaqHistSvc::regHist(const std::string& id, std::unique_ptr<TH1> hist)
{
  THistID* hid = nullptr;
  return regHist_i(std::move(hist), id, false, hid);
}

StatusCode WebdaqHistSvc::regHist(const std::string& id, TH1* hist_ptr)
{
  THistID* hid = nullptr;
  std::unique_ptr<TH1> hist(hist_ptr);
  return regHist_i(std::move(hist), id, false, hid);
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::getHist(const std::string& id, TH1*& hist, size_t ind) const
{
  hist = getHist_i<TH1>(id, ind);
  return (hist != nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

StatusCode WebdaqHistSvc::getHist(const std::string& id, TH2*& hist, size_t ind) const
{
  hist = getHist_i<TH2>(id, ind);
  return (hist != nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

StatusCode WebdaqHistSvc::getHist(const std::string& id, TH3*& hist, size_t ind) const
{
  hist = getHist_i<TH3>(id, ind);
  return (hist != nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::getTHists(TDirectory* td, TList& tl, bool recurse) const
{
  if (recurse) ATH_MSG_DEBUG("Recursive flag is not supported in this implementation");
  return getTHists_i(std::string(td->GetPath()), tl);
}

StatusCode WebdaqHistSvc::getTHists(const std::string& dir, TList& tl, bool recurse) const
{
  if (recurse) ATH_MSG_DEBUG("Recursive flag is not supported in this implementation");
  return getTHists_i(dir, tl);
}

StatusCode WebdaqHistSvc::getTHists(TDirectory* td, TList& tl, bool recurse, bool reg)
{
  if (recurse || reg)
    ATH_MSG_DEBUG("Recursive flag and automatic registration flag is not "
                  "supported in this implementation");
  return getTHists_i(std::string(td->GetPath()), tl);
}

StatusCode WebdaqHistSvc::getTHists(const std::string& dir, TList& tl, bool recurse, bool reg)
{
  if (recurse || reg)
    ATH_MSG_DEBUG("Recursive flag and automatic registration flag is not "
                  "supported in this implementation");
  return getTHists_i(dir, tl);
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::regShared(const std::string& id, std::unique_ptr<TH1> hist,
                                      LockedHandle<TH1>& lh)
{
  lh = regShared_i<TH1>(id, std::move(hist));
  return (lh ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

StatusCode WebdaqHistSvc::regShared(const std::string& id, std::unique_ptr<TH2> hist,
                                      LockedHandle<TH2>& lh)
{
  lh = regShared_i<TH2>(id, std::move(hist));
  return (lh ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

StatusCode WebdaqHistSvc::regShared(const std::string& id, std::unique_ptr<TH3> hist,
                                      LockedHandle<TH3>& lh)
{
  lh = regShared_i<TH3>(id, std::move(hist));
  return (lh ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::getShared(const std::string& id, LockedHandle<TH1>& lh) const
{
  lh = getShared_i<TH1>(id);
  return (lh ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

StatusCode WebdaqHistSvc::getShared(const std::string& id, LockedHandle<TH2>& lh) const
{
  lh = getShared_i<TH2>(id);
  return (lh ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

StatusCode WebdaqHistSvc::getShared(const std::string& id, LockedHandle<TH3>& lh) const
{
  lh = getShared_i<TH3>(id);
  return (lh ? StatusCode::SUCCESS : StatusCode::FAILURE);
}

/**************************************************************************************/

/**
 * @brief Histograms publication via WebDAQ.
 * It divides the histograms into batches and publishes them in slots.
 * The method also handles synchronization to ensure that the publication happens at regular intervals.
 *
 * @param numSlots Number of slots to divide the histograms into for publication.
 * @param intervalSeconds Interval in seconds between each publication cycle.
 * @param histoMapUpdated Atomic flag indicating if the histogram map has been updated.
 * @param nameSelect Regular expression to select histograms for publication.
 */
void WebdaqHistSvc::monitoringTask(int numSlots, int intervalSeconds, std::atomic<bool>& histoMapUpdated, boost::regex nameSelect)
{
  ATH_MSG_INFO("Started monitoring task for partition: " << m_partition << "and regex: " << nameSelect.str());
  std::string appName = m_jobOptionsSvc->get("DataFlowConfig.DF_ApplicationName");
  // OH doesn't allow multiple providers for a given server
  // Need to modify the path for one of the threads to avoid the issue
  if (nameSelect != boost::regex(".*"))
    appName = appName + "_fast";
  
  // Set the publication period
  boost::posix_time::time_duration interval{boost::posix_time::seconds(intervalSeconds)};
  ATH_MSG_DEBUG("Interval set to " << interval.total_seconds() << " seconds");
  int interval_ms = interval.total_milliseconds();
  if(numSlots == 0) numSlots = 1;
  boost::posix_time::ptime epoch(boost::gregorian::date(2024,1,1)); 
  // Sleep duration between slots (plus an extra slot for allowing a last sleep cycle)
  boost::posix_time::time_duration slotSleepDuration = interval / (numSlots + 1);

  // Create the Set of the histograms keys to order the histograms publication and reset the histoMapUpdated flag 
  std::set<std::string> HistoSet = getSet(nameSelect);
  histoMapUpdated = false;

  // Sync the publication to the period
  syncPublish(interval_ms, epoch);
  ATH_MSG_DEBUG("Monitoring task synched");

  // Publication loop
  while (!m_stopFlag) 
  {
    // Check if the histograms map has been updated, and if so update the Set
    if (histoMapUpdated)
    {
      ATH_MSG_DEBUG("Histo map updated, updating the Set");
      HistoSet = getSet(nameSelect);
      histoMapUpdated = false;
    }
    size_t totalHists = HistoSet.size();
    ATH_MSG_DEBUG("Going to publish " << totalHists << " histograms");

    // Divide the histograms in batches
    size_t batchSize = (totalHists + numSlots - 1) / numSlots; // Ceiling division
    ATH_MSG_DEBUG("Num of slots:" << numSlots << ", Interval_ms " << interval_ms << " milliseconds, Batch size: " << batchSize);

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::universal_time();
    int counter = 0;
    int BatchCounter = 0;
    auto it = HistoSet.begin();
    while(it != HistoSet.end())
    {
      boost::posix_time::ptime slot_start_time = boost::posix_time::microsec_clock::universal_time();
      //Batch publication
      ATH_MSG_DEBUG("Batch publication number " << BatchCounter << "  starting.");
      for(size_t j = 0; j < batchSize; ++j)
      {
        if(it == HistoSet.end())
        {
          break;
        }
        const std::string& id = *it;
        std::string path = appName + '.' + id;
        ATH_MSG_DEBUG("Publishing to " << m_partition << " Histogram " << path << " to the OH server " << m_tdaqOHServerName);
        tbb::concurrent_hash_map<std::string, THistID>::const_accessor accessor;
        if (!m_hists.find(accessor, id)) {
          ATH_MSG_WARNING("Histogram with name " << id << " not found in histogram map (probably deregistered).");
          it++;
          continue;
        }
        else
        {
          ATH_MSG_DEBUG("Histogram found in map, going to lock mutex and then publish it");
          //Here we clone the histogram to avoid locking the OH mutex during the whole publication 
          TObject* obj = nullptr;
          {
            //Locking the OH mutex before touching the Histogram
            oh_scoped_lock_histogram lock;
            obj = accessor->second.obj->Clone(); 
          }
          if (obj == nullptr) {
            ATH_MSG_ERROR("Failed to clone histogram " << id);
            it++;
            continue;
          }
          if (!webdaq::oh::put(m_partition, m_tdaqOHServerName, path, obj)) {
            ATH_MSG_ERROR("Histogram publishing failed !");
          }
          //Delete the cloned histogram. This was creating a memory leak
          ATH_MSG_DEBUG("Deleting cloned histogram");
          delete obj;
        }
        it++;
        counter++;
      } 
      ATH_MSG_DEBUG("Batch publication completed, " << counter << " histograms published");
      // Sleep for slotSleepDuration - slot publication time, unless it's the last slot
      if (it != HistoSet.end()) {
        boost::posix_time::ptime slot_end_time = boost::posix_time::microsec_clock::universal_time();
        int slot_sleep_time = slotSleepDuration.total_milliseconds() - (slot_end_time-slot_start_time).total_milliseconds();
        if (slot_sleep_time > 0) {
          ATH_MSG_DEBUG("Sleeping for " << slot_sleep_time << " seconds before publishing the next batch");
          conditionedSleep(std::chrono::milliseconds(slot_sleep_time), m_stopFlag);
        }
      }
      BatchCounter++;
    }

    //check if we exceeded the publication period
    boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::universal_time();
    if (boost::posix_time::time_duration(end_time-start_time) > interval) {
      ATH_MSG_WARNING("Publication deadline missed, cycle exceeded the interval.. Total publication time " 
      <<  boost::posix_time::to_simple_string(end_time-start_time));
    }
    ATH_MSG_DEBUG("Completed the publication of " << counter << " histograms. Publication time: " << boost::posix_time::to_simple_string(end_time-start_time));
 
    //sleep till the next cycle
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time(); 
    int nowMs = (now-epoch).total_milliseconds();
    boost::posix_time::time_duration next_cycle(boost::posix_time::milliseconds(interval_ms - (nowMs % interval_ms)));
    ATH_MSG_DEBUG("epoch " << epoch);
    ATH_MSG_DEBUG("interval_ms" << interval_ms);
    ATH_MSG_DEBUG("now_ms " << nowMs);
    ATH_MSG_DEBUG("Sleeping for " << next_cycle.total_milliseconds() << " milliseconds till the next cycle");
    conditionedSleep(std::chrono::milliseconds(next_cycle.total_milliseconds()), m_stopFlag);
  }
  ATH_MSG_INFO("Monitoring task stopped");
}

/**************************************************************************************/

void WebdaqHistSvc::syncPublish(long int interval_ms, boost::posix_time::ptime epoch)
{
  //Sync the publication to a multple of the interval
  //Code taken from TDAQ monsvc https://gitlab.cern.ch/atlas-tdaq-software/monsvc/-/blob/master/src/PeriodicScheduler.cxx?ref_type=heads#L163
  boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
  int now_ms = (now-epoch).total_milliseconds();
  //If now_ms % interval_ms == 0 we skip a cycle. Too bad.
  boost::posix_time::time_duration sync(boost::posix_time::milliseconds(interval_ms - (now_ms % interval_ms)));
  //Do not sync if we are below 50 ms
  if (sync.total_milliseconds() > 50){
    std::this_thread::sleep_for(std::chrono::milliseconds(sync.total_milliseconds()));
  } 
}

void WebdaqHistSvc::conditionedSleep(std::chrono::milliseconds duration, const std::atomic<bool>& stopFlag) {
  auto start = std::chrono::steady_clock::now();
  while (true) {
    if (stopFlag.load()) {
      return;
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    if (elapsed >= duration) {
      break;
    }
    auto remaining = duration - std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    std::this_thread::sleep_for(std::min(std::chrono::milliseconds(500), remaining));
  }
}

