/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ASG_TOOLS__ASG_COMPONENT_FACTORIES_H
#define ASG_TOOLS__ASG_COMPONENT_FACTORIES_H

#include <AsgMessaging/StatusCode.h>
#include <functional>
#include <memory>

namespace asg
{
  class AsgComponent;

#ifdef XAOD_STANDALONE

  /// @brief register a factory for the given tool type
  StatusCode registerComponentFactory (const std::string& type, const std::function<std::unique_ptr<AsgComponent>(const std::string& name)>& factory);

  /// @brief get the factory for the given tool type (or null if none exists)
  const std::function<std::unique_ptr<AsgComponent>(const std::string& name)> *getComponentFactory (const std::string& type);


  /// @brief simple helpers to register factories via templates
  ///
  /// There is one for each component type, as they can differ in their constructor.
  ///
  /// @{
  template<typename T> StatusCode registerToolFactory (const std::string& type)
  {
    return registerComponentFactory (type, [] (const std::string& name) -> std::unique_ptr<AsgComponent> { return std::make_unique<T> (name); });
  }
  template<typename T> StatusCode registerServiceFactory (const std::string& type)
  {
    return registerComponentFactory (type, [] (const std::string& name) -> std::unique_ptr<AsgComponent> { return std::make_unique<T> (name, nullptr); });
  }
  template<typename T> StatusCode registerAlgorithmFactory (const std::string& type)
  {
    return registerComponentFactory (type, [] (const std::string& name) -> std::unique_ptr<AsgComponent> { return std::make_unique<T> (name, nullptr); });
  }
  /// @}

#endif
}

#endif
