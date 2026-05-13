/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/WorkerConfig.h>

#include <AnaAlgorithm/AnaAlgorithmWrapper.h>
#include <AnaAlgorithm/AnaReentrantAlgorithmWrapper.h>
#include <AnaAlgorithm/PythonConfigBase.h>
#include <AsgTools/SgEventMeta.h>
#include <EventLoop/AsgServiceWrapper.h>
#include <EventLoop/AsgToolWrapper.h>
#include <EventLoop/ModuleData.h>

#include <EventLoop/MessageCheck.h>

//
// method implementations
//

ClassImp (EL::WorkerConfig)

namespace EL
{
  WorkerConfig ::
  WorkerConfig (Detail::ModuleData *val_data) noexcept
    : m_data (val_data),
      m_metaStore (asg::SgEventMeta::InputStore, nullptr)
  {}



  // FIX ME: something bad happens in the linker step if this is not
  // here and instead rely on the implicit destructor, which I don't
  // want to debug.
  WorkerConfig :: ~WorkerConfig () noexcept = default;



  void WorkerConfig ::
  add (const EL::PythonConfigBase& config)
  {
    using namespace msgEventLoop;
    ANA_MSG_INFO ("in add");
    // no invariant used
    std::unique_ptr<IAlgorithmWrapper> alg;
    if (config.componentType() == "AnaAlgorithm")
      alg = std::make_unique<AnaAlgorithmWrapper> (AnaAlgorithmConfig (config));
    else if (config.componentType() == "AnaReentrantAlgorithm")
      alg = std::make_unique<AnaReentrantAlgorithmWrapper> (AnaReentrantAlgorithmConfig (config));
    else if (config.componentType() == "AsgTool")
      alg = std::make_unique<AsgToolWrapper> (asg::AsgToolConfig (config));
    else if (config.componentType() == "AsgService")
      alg = std::make_unique<AsgServiceWrapper> (asg::AsgServiceConfig (config));
    else
    {
      ANA_MSG_ERROR ("unknown component type: \"" << config.componentType() << "\"");
      throw std::runtime_error ("unknown component type: \"" + config.componentType() + "\"");
    }
    m_data->m_algs.emplace_back (std::move (alg));
  }
}
