/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/IMPIClusterSvc.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

// A synthetic input fixture: no external event file or detector is needed.
// Log EndProcessing separately from MPI's completion log to detect a failure
// policy which accidentally skips lifecycle cleanup at its stopping threshold.
class MPIHiveLifecycleTestAlg
    : public extends<AthReentrantAlgorithm, IIncidentListener> {
public:
  using extends::extends;

  StatusCode initialize() override {
    ATH_CHECK(m_cluster.retrieve());
    ATH_CHECK(m_incidents.retrieve());
    m_incidents->addListener(this, IncidentType::EndProcessing);
    m_incidents->fireIncident(
        FileIncident(name(), IncidentType::BeginInputFile, "synthetic-mpi-input"));
    return StatusCode::SUCCESS;
  }

  StatusCode execute(const EventContext& ctx) const override {
    // Give multiple workers and slots time to participate in the success case.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (std::find(m_failEvents.begin(), m_failEvents.end(), ctx.evt()) !=
        m_failEvents.end()) {
      ATH_MSG_INFO("MPI fixture failing event " << ctx.evt());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  void handle(const Incident& incident) override {
    ATH_MSG_INFO("MPI lifecycle end event " << incident.context().evt());
  }

  StatusCode stop() override {
    SmartIF<IHiveWhiteBoard> wb;
    wb = serviceLocator()->service("EventDataSvc");
    if (!wb || wb->freeSlots() != wb->getNumberOfStores()) {
      ATH_MSG_ERROR("Completed MPI events left occupied whiteboard slots");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("MPI lifecycle slots released");
    return StatusCode::SUCCESS;
  }

  StatusCode finalize() override {
    m_incidents->removeListener(this);
    return StatusCode::SUCCESS;
  }

private:
  ServiceHandle<IMPIClusterSvc> m_cluster{this, "ClusterSvc", "MPIClusterSvc"};
  ServiceHandle<IIncidentSvc> m_incidents{this, "IncidentSvc", "IncidentSvc"};
  Gaudi::Property<std::vector<unsigned long>> m_failEvents{
      this, "FailEvents", {}, "Global event indices which deliberately fail"};
};

DECLARE_COMPONENT(MPIHiveLifecycleTestAlg)
