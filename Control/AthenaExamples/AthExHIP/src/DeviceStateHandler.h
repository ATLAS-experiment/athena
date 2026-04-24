//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef ATHEXHIP_HIPSTATEHANDLER_H
#define ATHEXHIP_HIPSTATEHANDLER_H

#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaInterprocess/Incidents.h"


namespace AthHIPExamples{
/**
 * @class DeviceStateHandler
 * @brief Base class to provide common infrastructure 
 *        for handling multiprocess state transitions for GPU.
 *        Based on CaloRecGPU/CaloGPUCUDAInitialization.h */

class DeviceStateHandler : virtual public ::IIncidentListener
{
 protected: 

  ///Glocal initialization.
  virtual StatusCode initialize_global()
  {
    return StatusCode::SUCCESS;
  }

  ///Initialization per process
  virtual StatusCode initialize_worker()
  {
    return StatusCode::SUCCESS;
  }

  ///clean up
  virtual StatusCode stop_worker()
  {
    return StatusCode::SUCCESS;
  }

  virtual StatusCode initialize()
  {
    ATH_CHECK(this->initialize_global());

    const bool is_multiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);

    if (is_multiprocess)
      {
        ServiceHandle<IIncidentSvc> incidentSvc("IncidentSvc","DeviceStateHandler");
        incidentSvc->addListener(this, AthenaInterprocess::UpdateAfterFork::type());
      }
    else
      {
        ATH_CHECK(this->initialize_worker());
      }

    return StatusCode::SUCCESS;
  }

  virtual StatusCode stop() {
    const bool is_multiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);

    if (!is_multiprocess) {
      ATH_CHECK(this->stop_worker());
    } else {
      if (m_is_child){
              ATH_CHECK(this->stop_worker());
      }
    }

  return StatusCode::SUCCESS;
  }

  bool m_is_child = false;

 public:

  void handle(const Incident & incident) override
  {
    const bool is_multiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);
    if (is_multiprocess && incident.type() == AthenaInterprocess::UpdateAfterFork::type())
      {
        m_is_child = true;
        if (!this->initialize_worker().isSuccess())
        {
          throw GaudiException("Failed to initialize the device setup!",
                               "DeviceStateHandler::handle",
                               StatusCode::FAILURE);
        }
      }
  }
  
  virtual ~DeviceStateHandler() = default;

};

}  // namespace AthHIPExamples

#endif //ATHEXHIP_HIPSTATEHANDLER_H
