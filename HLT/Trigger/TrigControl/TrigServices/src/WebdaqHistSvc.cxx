/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    m_thread = std::thread( &WebdaqHistSvc::monitoringTask, this);
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
  ATH_MSG_DEBUG("Clearing list of histograms");
  m_hists.clear();
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
    T* hist = hist_unique.release();

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
    accessor->second = THistID(id, hist);
    //finished
    if (shared) accessor->second.mutex = new std::mutex;
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
      if (phid) lh.set(phist, phid->mutex);
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
      lh.set(phist, accessor->second.mutex);
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
  if (!m_hists.find(accessor, id)) {
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
    return LockedHandle<T>(phist, accessor->second.mutex);
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
  return StatusCode::FAILURE;
}

/**************************************************************************************/

StatusCode WebdaqHistSvc::deReg(const std::string& id)
{
  tbb::concurrent_hash_map<std::string, THistID>::accessor accessor;
  if (m_hists.find(accessor, id)) {
    m_hists.erase(accessor);
    ATH_MSG_DEBUG("Deregistration of " << id << " done");
    return StatusCode::SUCCESS;
  }
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

StatusCode WebdaqHistSvc::regHist(const std::string& id, std::unique_ptr<TH1> hist, TH1* /*hist_ptr*/)
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

void WebdaqHistSvc::monitoringTask()
{
  ATH_MSG_INFO("Started monitoring task for partition: " << m_partition);
  
  std::string appName = m_jobOptionsSvc->get("DataFlowConfig.DF_ApplicationName");
  
  //Set the publication period
  boost::posix_time::time_duration interval{boost::posix_time::seconds(m_intervalSeconds.value())};
  int interval_ms = interval.total_milliseconds();
  boost::posix_time::ptime epoch(boost::gregorian::date(2024,1,1)); 

  // Sleep duration between slots
  boost::posix_time::time_duration slotSleepDuration = interval / m_numSlots;

  //Sync the publication to the period
  syncPublish(interval_ms,epoch);

  //Publication loop
  while (!m_stopFlag) {
    ATH_MSG_DEBUG("Total histograms registered: " << m_hists.size());

    //Instead of looping over the elements, I want to access them by key, so that I'm sure I follow a sorted order  
    std::vector<std::string> HistoList = getHists(); 
    std::sort(HistoList.begin(), HistoList.end());
    ATH_MSG_DEBUG("Going to publish " << HistoList.size() << " histograms");

    //Divide the histograms in batches
    size_t totalHists = HistoList.size();
    size_t batchSize = (totalHists + m_numSlots - 1) / m_numSlots; // Ceiling division

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::universal_time();
    for (size_t i = 0; i < totalHists; i += batchSize) {
      boost::posix_time::ptime slot_start_time = boost::posix_time::microsec_clock::universal_time();
      // Determine the end index for the current batch
      size_t end = std::min(i + batchSize, totalHists);

      // Publish histograms in the current batch
      for (size_t j = i; j < end; ++j) {
        const std::string& id = HistoList[j];
        std::string path = appName + '.' + id;
        ATH_MSG_DEBUG("Going to publish to " << m_partition << " Histogram " << path << " to the OH server " << m_tdaqOHServerName);
        tbb::concurrent_hash_map<std::string, THistID>::const_accessor accessor;
        if (!m_hists.find(accessor, id)) {
          ATH_MSG_ERROR("Histogram with name " << id << " not found in histogram map");
          continue;
        }
        else
        {
          ATH_MSG_DEBUG("Histogram found in map, going to lock mutex and then publish it");
          //Locking the OH mutex before the Histogram publication
          oh_scoped_lock_histogram lock;
          if (!webdaq::oh::put(m_partition, m_tdaqOHServerName, path, accessor->second.obj)) {
            ATH_MSG_ERROR("Histogram publishing failed !");
          }
        }
      }     

      // Sleep for slotSleepDuration - slot publication time, unless it's the last slot
      if (end < totalHists) {
        ATH_MSG_DEBUG("Batch publication completed, " << end << " histograms published");
        boost::posix_time::ptime slot_end_time = boost::posix_time::microsec_clock::universal_time();
        int slot_sleep_time = slotSleepDuration.total_milliseconds() - (slot_end_time-slot_start_time).total_milliseconds();
        if (slot_sleep_time > 0) {
          ATH_MSG_DEBUG("Sleeping for " << slot_sleep_time
                        << " seconds before publishing the next batch");
          std::this_thread::sleep_for(std::chrono::milliseconds(slot_sleep_time));
        }
      }
    }

    //check if we exceeded the publication period
    boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::universal_time();
    if (boost::posix_time::time_duration(end_time-start_time) > interval) {
      ATH_MSG_WARNING("Publication deadline missed, cycle exceeded the interval.. Total publication time " 
      <<  boost::posix_time::to_simple_string(end_time-start_time));
    }
    ATH_MSG_DEBUG("Publication time: " << boost::posix_time::to_simple_string(end_time-start_time));
    
    //sleep till the next cycle
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time(); 
    int now_ms = (now-epoch).total_milliseconds();
    boost::posix_time::time_duration next_cycle(boost::posix_time::milliseconds(abs(now_ms % interval_ms)));
    std::this_thread::sleep_for(std::chrono::milliseconds(next_cycle.total_milliseconds()));
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
