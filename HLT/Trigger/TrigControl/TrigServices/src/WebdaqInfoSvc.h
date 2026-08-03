/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSERVICES_WEBDAQINFOSVC_H
#define TRIGSERVICES_WEBDAQINFOSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IIncidentListener.h"
#include "CxxUtils/checker_macros.h"

#include "TrigServicesUtils.h"

#include "hltinterface/IInfoRegister.h"

#include <nlohmann/json_fwd.hpp>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class TObject;

namespace hltinterface {
  class ContainerFactory;
  class GenericHLTContainer;
}

/**
 * @class  WebdaqInfoSvc
 * @brief  Service that publishes IS objects via the webdaq REST API.
 *
 * On initialize() the service installs itself as the hltinterface::IInfoRegister singleton and provides a default ContainerFactory.
 * Producers (e.g. TrigLArNoiseBurstRecoAlg) keep using the hltinterface API unchanged: construct GenericHLTContainer objects via the factory, 
 * register through IInfoRegister and append values inside beginEvent / endEvent.
 * endEvent takes a JSON snapshot of each container and a background thread ships the snapshots to the IS server via webdaq.
 *
 * Only the IS half of the IInfoRegister interface is implemented, histograms are handled by WebdaqHistSvc.
 *
 * Required environment variables:
 *  - TDAQ_PARTITION    : the partition to publish to
 *  - TDAQ_WEBDAQ_BASE  : protocol, hostname and, if required, port of the webdaq server to publish through
 *                        (e.g. http://localhost:8080). Same variable used by WebdaqHistSvc.
 *  - TDAQ_IS_SERVER    : (optional) name of the IS server, default is taken from the ISServerName property
 */
class WebdaqInfoSvc : public extends<AthService, IIncidentListener>,
                      public hltinterface::IInfoRegister
{
public:
  using base_class::base_class;
  virtual ~WebdaqInfoSvc() noexcept override {}

  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode stop() override;
  virtual StatusCode finalize() override;
  virtual void handle(const Incident& incident) override;

  // ------------------------------------------------------------------------
  // hltinterface::IInfoRegister — IS
  // ------------------------------------------------------------------------
  virtual bool configure     (const boost::property_tree::ptree&) override { return true; }
  virtual bool prepareForRun (const boost::property_tree::ptree&) override { return true; }
  virtual bool prepareWorker (const boost::property_tree::ptree&) override { return true; }
  virtual bool finalizeWorker(const boost::property_tree::ptree&) override { return true; }
  virtual bool finalize      (const boost::property_tree::ptree&) override { return true; }

  // The by-value strings below are imposed by the hltinterface::IInfoRegister base class.
  // cppcheck-suppress passedByValue; signature fixed by hltinterface::IInfoRegister
  virtual bool registerObject(const std::string publishPath,
                              std::shared_ptr<hltinterface::GenericHLTContainer> obj) override;
  /// De-register an object. The argument is the full IS name, i.e. the publish path passed to
  /// registerObject() followed by the container's object name.
  // cppcheck-suppress passedByValue; signature fixed by hltinterface::IInfoRegister
  virtual bool releaseObject (const std::string fullName) override;
  virtual bool pullInfo      (const std::string&, const std::string&) override { NOSUPPORT(DEBUG, "pullInfo"); }

  virtual bool beginEvent(const boost::property_tree::ptree&) override { return true; }
  virtual bool endEvent  (const boost::property_tree::ptree&) override;

  virtual std::vector<std::shared_ptr<hltinterface::GenericHLTContainer>>
       queryISRegistry(const std::string&) override { NOSUPPORT(DEBUG, "queryISRegistry"); }

  // ------------------------------------------------------------------------
  // hltinterface::IInfoRegister — Histograms (handled by WebdaqHistSvc, unsupported).
  // ------------------------------------------------------------------------
  virtual void get        (const std::string&, THList&) override { NOSUPPORT_VOID(DEBUG, "Histogram retrieval"); }
  virtual void getUnsummed(const std::string&,
                           std::map<std::string, std::vector<TObject*>>&) override { NOSUPPORT_VOID(DEBUG, "Histogram retrieval"); }
  virtual void clear      (const std::string&) override { NOSUPPORT_VOID(DEBUG, "Histogram clearing"); }
  virtual void reset      (const std::string&) override { NOSUPPORT_VOID(DEBUG, "Histogram reset"); }

  virtual bool registerTObject (const std::string&, const std::string&, TObject*) override { NOSUPPORT(WARNING, "Histogram registration"); }
  virtual bool discoverTObject (const std::string&, const std::string&, TObject*&) override { NOSUPPORT(WARNING, "Histogram discovery"); }
  virtual bool releaseTObject  (const std::string&, const std::string&) override { NOSUPPORT(WARNING, "Histogram release"); }
  virtual void clearToRelease  (const std::string&) override { NOSUPPORT_VOID(DEBUG, "Histogram release"); }

  /// Producers are expected to hold m_pubMutex while mutating a registered container, and across the endEvent() call itself.
  /// endEvent() serialises every registered container, because the IInfoRegister interface gives no way to tell which one the caller just finished writing.
  ///
  /// TrigLArNoiseBurstRecoAlg is currently not using this mutex (note that this is currently not a problem since TrigLArNoiseBurstRecoAlg is the only producer we have).
  /// The reason is that MonSvcInfoService hands out the global OH mutex, and this would coupel IS publication with the OH one. 
  /// In this new service we hand out our own, avoiding coupling IS to histogram operations.
  ///
  /// @todo Adopt this in TrigLArNoiseBurstRecoAlg, which still uses a private mutex, once the legacy service is retired. TrigExISPublishing already uses it. 
  ///       The mutex would become unnecessary overloading endEvent(ptree, fullName) in hltinterface, identifying the caller's object.
  ///       Each producer would then touch only its own container and a private mutex would suffices. 
  ///       A cleaner alternative would be locking inside GenericHLTContainer. But this would require changing the accessors that are currently handing out raw references
  ///       The hltinterface change should also drop const from getPublicationMutex(): returning a non-const reference from a const method forces the workaround below.
  virtual mutex_type& getPublicationMutex() const override {
    // We want to give non-const access through a const interface (IInfoRegister declares this const) --> need annotated local
    mutex_type& mtx ATLAS_THREAD_SAFE = m_pubMutex;
    return mtx;
  }
  virtual void setModification(bool b) override { p_setModification(b); }

private:
  /// One entry per registered IS object (defined in the .cxx)
  struct Holder;

  /// Background loop that periodically publishes containers
  void publishingTask() const;
  /// Send the pre-built snapshot via webdaq::is::put
  bool publishOne(Holder& h) const;
  /// Serialise all supported fields into a JSON object
  static nlohmann::json serializeContainer(hltinterface::GenericHLTContainer& cont);
  /// Copy of the registered holders, taken under m_objsMtx
  std::vector<std::shared_ptr<Holder>> holders() const;

  /// Container factory
  std::shared_ptr<hltinterface::ContainerFactory> m_factory;

  /// Registered IS objects, keyed by full IS name (publishPath + object name)
  std::unordered_map<std::string, std::shared_ptr<Holder>> m_objs;
  mutable std::mutex m_objsMtx;

  /// Background publishing thread + stop flag. 
  std::thread       m_thread;
  std::atomic<bool> m_stopFlag{false};

  std::string m_partition;
  std::string m_tdaqWebdaqBase;
  std::string m_tdaqIsServerName;

  /// Publication mutex handed out by getPublicationMutex().
  mutable std::mutex m_pubMutex;

  Gaudi::Property<std::string> m_isServerName{this, "ISServerName", "DF",
      "Name of the IS server to publish to (used if TDAQ_IS_SERVER unset)"};
  Gaudi::Property<unsigned int> m_intervalSeconds{this, "IntervalSeconds", 10,
      "Period between publication cycles (s)"};
};

#endif // TRIGSERVICES_WEBDAQINFOSVC_H
