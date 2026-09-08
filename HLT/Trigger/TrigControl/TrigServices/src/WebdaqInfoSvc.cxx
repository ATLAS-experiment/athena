/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "WebdaqInfoSvc.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaInterprocess/Incidents.h"

#include "hltinterface/ContainerFactory.h"
#include "hltinterface/GenericHLTContainer.h"
#include "webdaq/webdaq.hpp"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <utility>

/// One entry per registered IS object.
///
/// The container is written by the producer and read by the service only inside endEvent(), i.e. on the producer's own thread. 
/// The publishing thread never touches it, it only ever reads snapshot. 
struct WebdaqInfoSvc::Holder {
  /// The producer container. The service only ever reads it.
  std::shared_ptr<hltinterface::GenericHLTContainer> cont;
  /// Name to publish under: publish path + container object name
  std::string       fullName;
  /// IS type name
  std::string       typeName;
  /// JSON image of the container, rebuilt on every endEvent() and shipped by the publishing thread
  nlohmann::json    snapshot;
  /// Protects snapshot only. It is written by the event thread in endEvent() and read by the publishing thread. 
  std::mutex        mtx;
  /// Set when a new snapshot is stored, and claimed by the publishing thread so a snapshot is published once. 
  /// It is set again if the publication fails, so that the next cycle retries it.
  std::atomic<bool> dirty{false};
};

/**************************************************************************************/

StatusCode WebdaqInfoSvc::initialize ATLAS_NOT_THREAD_SAFE()
{
  //Retrieve environment variables
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
    ATH_MSG_ERROR("TDAQ_WEBDAQ_BASE environment variable not set! "
                  "Is needed for the IS publication through webdaq");
    return StatusCode::FAILURE;
  }
  const char* tdaq_is_server = std::getenv("TDAQ_IS_SERVER");
  if (tdaq_is_server != nullptr) {
    m_tdaqIsServerName = std::string(tdaq_is_server);
  } else {
    m_tdaqIsServerName = m_isServerName.value();
  }
  ATH_MSG_INFO("TDAQ_IS_SERVER value: " << m_tdaqIsServerName);

  // Install as the IInfoRegister singleton 
  hltinterface::IInfoRegister::setInstance(this, /*force=*/true);
  ATH_MSG_INFO("Installed WebdaqInfoSvc as the hltinterface::IInfoRegister singleton");

  // Provide a default ContainerFactory to build GenericHLTContainer instances via ContainerFactory::getInstance() 
  m_factory = hltinterface::ContainerFactory::getInstance();
  if (m_factory) {
    ATH_MSG_INFO("Reusing the hltinterface::ContainerFactory installed by another component");
  }
  else {
    m_factory = std::make_shared<hltinterface::ContainerFactory>();
    hltinterface::ContainerFactory::setInstance(m_factory);
    ATH_MSG_INFO("Installed the default hltinterface::ContainerFactory");
  }

  // Start publishing thread after fork
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
  ATH_CHECK( incSvc.retrieve() );
  incSvc->addListener(this, AthenaInterprocess::UpdateAfterFork::type());
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

void WebdaqInfoSvc::handle(const Incident& incident)
{
  if (incident.type() == AthenaInterprocess::UpdateAfterFork::type()) {
    ATH_MSG_INFO("Going to initialize the IS publishing thread (interval: "
                 << m_intervalSeconds.value() << " s)");
    m_thread = std::thread(&WebdaqInfoSvc::publishingTask, this);
  }
}

/**************************************************************************************/

StatusCode WebdaqInfoSvc::stop()
{
  ATH_MSG_DEBUG("Stopping IS publishing task");
  m_stopFlag = true;
  if (m_thread.joinable()) {
    ATH_MSG_DEBUG("Going to join the IS publishing thread");
    try {
        m_thread.join();
    }
    catch (const std::exception& e) {
      ATH_MSG_ERROR("Failed to join the IS publishing thread: " << e.what());
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_DEBUG("Final flush of registered IS objects");
  for (const auto& h : holders()) {
    publishOne(*h);
  }
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

StatusCode WebdaqInfoSvc::finalize()
{
  ATH_MSG_INFO("finalize");

  // Uninstall ourselves.
  if (hltinterface::IInfoRegister::instance() == this) {
    hltinterface::IInfoRegister::setInstance(nullptr, /*force=*/true);
  }

  std::scoped_lock lk(m_objsMtx);
  m_objs.clear();
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

// cppcheck-suppress passedByValue; signature fixed by hltinterface::IInfoRegister
bool WebdaqInfoSvc::registerObject(const std::string publishPath,
                                   std::shared_ptr<hltinterface::GenericHLTContainer> obj)
{
  if (!obj) {
    ATH_MSG_WARNING("registerObject called with null container");
    return false;
  }

  // Key on the full IS name: the publish path alone is not unique, producers conventionally
  // register under the same one (e.g. "/HLTObjects/") and would otherwise collide.
  const std::string full = publishPath + obj->getObjName();

  std::scoped_lock lk(m_objsMtx);
  if (m_objs.find(full) != m_objs.end()) {
    ATH_MSG_WARNING("IS object " << full << " is already registered");
    return false;
  }

  auto h = std::make_shared<Holder>();
  h->cont     = obj;
  h->fullName = full;
  h->typeName = obj->getTypeName();
  m_objs.emplace(full, std::move(h));

  ATH_MSG_INFO("Registered IS object " << full
               << " (type=" << obj->getTypeName() << ")"
               << " for publication on " << m_tdaqIsServerName
               << " in partition " << m_partition);
  return true;
}

/**************************************************************************************/

// cppcheck-suppress passedByValue; signature fixed by hltinterface::IInfoRegister
bool WebdaqInfoSvc::releaseObject(const std::string fullName)
{
  std::scoped_lock lk(m_objsMtx);
  auto it = m_objs.find(fullName);
  if (it == m_objs.end()) {
    ATH_MSG_WARNING("releaseObject: " << fullName << " is not registered");
    return false;
  }
  m_objs.erase(it);
  ATH_MSG_DEBUG("Released IS object " << fullName);
  return true;
}

/**************************************************************************************/

auto WebdaqInfoSvc::holders() const -> std::vector<std::shared_ptr<Holder>>
{
  std::vector<std::shared_ptr<Holder>> all;
  std::scoped_lock lk(m_objsMtx);
  all.reserve(m_objs.size());
  for (const auto& [_, h] : m_objs) {
    all.push_back(h);
  }
  return all;
}

/**************************************************************************************/

bool WebdaqInfoSvc::endEvent(const boost::property_tree::ptree&)
{
  // The producer has finished mutating the in-memory container. 
  // Serialise each container into its snapshot here (the producer still holds its lock), then mark it dirty so that the
  // publishing thread can ship the snapshot without touching the container itself.
  for (const auto& h : holders()) {
    try {
      nlohmann::json j = serializeContainer(*h->cont);
      {
        std::scoped_lock hlk(h->mtx);
        h->snapshot = std::move(j);
      }
      h->dirty.store(true, std::memory_order_release);
    }
    catch (const std::exception& e) {
      ATH_MSG_WARNING("Failed to serialise IS object " << h->fullName << ": " << e.what());
    }
  }
  return true;
}

/**************************************************************************************/

void WebdaqInfoSvc::publishingTask() const
{
  ATH_MSG_INFO("Started webdaq IS publishing task for partition " << m_partition
               << ", IS server " << m_tdaqIsServerName);

  while (!m_stopFlag) {
    // Collect the holders that have been touched since the last cycle
    std::vector<std::shared_ptr<Holder>> dirtyObjs;
    for (const auto& h : holders()) {
      if (h->dirty.exchange(false, std::memory_order_acq_rel)) {
        dirtyObjs.push_back(h);
      }
    }
    for (const auto& h : dirtyObjs) {
      // Keep the object marked dirty if the publication failed, so that it is retried next cycle
      if (!publishOne(*h)) {
        h->dirty.store(true, std::memory_order_release);
      }
      if (m_stopFlag) break;
    }
    // Contrary to WebdaqHistSvc we don't use a sync to a global period. 
    TrigServices::conditionedSleep(std::chrono::seconds(m_intervalSeconds.value()), m_stopFlag);
  }
  ATH_MSG_INFO("Webdaq IS publishing task stopped");
}

/**************************************************************************************/

nlohmann::json WebdaqInfoSvc::serializeContainer(hltinterface::GenericHLTContainer& cont)
{
  using GHC = hltinterface::GenericHLTContainer;
  nlohmann::json j;

  // Only INT/FLOAT/INTVEC/FLOATVEC are supported: GenericHLTContainer::getFieldNames() throws
  // std::range_error for STRING/STRINGVEC, so those fields cannot be enumerated at all.
  auto names = cont.getFieldNames(GHC::INT);
  for (size_t i = 0; i < names.size(); ++i) j[names[i]] = cont.getIntField(i);

  names = cont.getFieldNames(GHC::FLOAT);
  for (size_t i = 0; i < names.size(); ++i) j[names[i]] = cont.getFloatField(i);

  names = cont.getFieldNames(GHC::INTVEC);
  for (size_t i = 0; i < names.size(); ++i) j[names[i]] = cont.getIntVecField(i);

  names = cont.getFieldNames(GHC::FLOATVEC);
  for (size_t i = 0; i < names.size(); ++i) j[names[i]] = cont.getFloatVecField(i);

  return j;
}

/**************************************************************************************/

bool WebdaqInfoSvc::publishOne(Holder& h) const
{
  nlohmann::json j;
  {
    std::scoped_lock lk(h.mtx);
    j = h.snapshot;
  }

  // Registered but never filled (no endEvent yet): do not overwrite the IS object with a null value
  if (j.is_null()) {
    ATH_MSG_DEBUG("No snapshot available for IS object " << h.fullName << ", skipping publication");
    return true;
  }

  ATH_MSG_DEBUG("Publishing IS object " << h.fullName
                << " (type=" << h.typeName << ") to "
                << m_tdaqIsServerName << "@" << m_partition);

  try {
    if (!webdaq::is::put(m_partition, m_tdaqIsServerName, h.fullName,
                         h.typeName, j)) {
      ATH_MSG_WARNING("webdaq::is::put failed for " << h.fullName);
      return false;
    }
  }
  catch (const std::exception& e) {
    ATH_MSG_WARNING("webdaq::is::put threw for " << h.fullName << ": " << e.what());
    return false;
  }
  return true;
}
