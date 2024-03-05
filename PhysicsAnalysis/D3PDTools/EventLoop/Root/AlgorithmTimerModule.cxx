/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/AlgorithmTimerModule.h>

#include <EventLoop/ModuleData.h>
#include <EventLoop/AlgorithmTimerWrapper.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    StatusCode AlgorithmTimerModule ::
    firstInitialize (ModuleData& data)
    {
      for (auto& alg : data.m_algs)
        alg.m_algorithm = std::make_unique<AlgorithmTimerWrapper>(std::move (alg.m_algorithm));
      return StatusCode::SUCCESS;
    }
  }
}
