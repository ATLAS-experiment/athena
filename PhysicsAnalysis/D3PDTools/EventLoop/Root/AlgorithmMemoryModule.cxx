/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmMemoryModule.h>

#include <EventLoop/ModuleData.h>
#include <EventLoop/AlgorithmMemoryWrapper.h>
#include <TSystem.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode AlgorithmMemoryModule ::
    firstInitialize (ModuleData& data)
    {
      ::ProcInfo_t pinfo;
      if (gSystem->GetProcInfo (&pinfo) != 0) {
        ANA_MSG_ERROR ("Could not get memory usage information");
        return StatusCode::FAILURE;
      }
      ANA_MSG_INFO ("Memory usage at start of job: " << pinfo.fMemResident << " kB resident, " << pinfo.fMemVirtual << " kB virtual");

      for (auto& alg : data.m_algs)
        alg.m_algorithm = std::make_unique<AlgorithmMemoryWrapper>(std::move (alg.m_algorithm));
      return StatusCode::SUCCESS;
    }
  }
}
