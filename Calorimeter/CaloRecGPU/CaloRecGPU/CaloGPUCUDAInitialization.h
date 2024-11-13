//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOGPUCUDAINITIALIZATION_H
#define CALORECGPU_CALOGPUCUDAINITIALIZATION_H

#include "GaudiKernel/IProperty.h"

#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ConcurrencyFlags.h"

#include "AthenaInterprocess/Incidents.h"


/**
 * @class CaloGPUCUDAInitialization
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 15 March 2024
 * @brief Base class to provide some basic common infrastructure 
 *        for initializing CUDA only at the right place to work fine with multiprocessing... */

class CaloGPUCUDAInitialization : virtual public IIncidentListener
{
 protected:
  

  ///Initialization that does not invoke CUDA functions.
  virtual StatusCode initialize_non_CUDA()
  {
    return StatusCode::SUCCESS;
  }

  ///Initialization that invokes CUDA functions.
  virtual StatusCode initialize_CUDA()
  {
    return StatusCode::SUCCESS;
  }

  virtual StatusCode initialize()
  {
    ATH_CHECK(this->initialize_non_CUDA());

    const bool is_multiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);

    if (is_multiprocess)
      {
        ServiceHandle<IIncidentSvc> incidentSvc("IncidentSvc","CaloGPUCUDAInitialization");
        
        incidentSvc->addListener(this, AthenaInterprocess::UpdateAfterFork::type());
      }
    else
      {
        ATH_CHECK(this->initialize_CUDA());
      }

    return StatusCode::SUCCESS;
  }

 public:

  void handle(const Incident & incident) override
  {
    const bool is_multiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);
    if (is_multiprocess && incident.type() == AthenaInterprocess::UpdateAfterFork::type())
      {
        if (!this->initialize_CUDA().isSuccess())
        {
          throw GaudiException("Failed to perform the CUDA initialization!",
                               "CaloGPUCUDAInitialization::handle",
                               StatusCode::FAILURE);
        }
      }
  }
  
  virtual ~CaloGPUCUDAInitialization() = default;

};

#endif //CALORECGPU_CALOGPUTIMED_H
