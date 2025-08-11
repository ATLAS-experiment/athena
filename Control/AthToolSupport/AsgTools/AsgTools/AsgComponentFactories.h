/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef ASG_TOOLS__ASG_COMPONENT_FACTORIES_H
#define ASG_TOOLS__ASG_COMPONENT_FACTORIES_H

#include <AsgMessaging/StatusCode.h>
#include <AsgTools/MessageCheckAsgTools.h>
#include <functional>
#include <memory>

namespace EL
{
  class AnaAlgorithm;
  class AnaReentrantAlgorithm;
}

namespace asg
{
  class AsgComponent;
  class AsgService;
  class AsgTool;

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

  /// more generic versions of factory registration
  /// @{
  template<typename T> requires std::is_base_of_v<AsgTool,T>
  StatusCode registerGenericComponentFactory (const std::string& type)
  {
    return registerToolFactory<T> (type);
  }
  template<typename T> requires std::is_base_of_v<EL::AnaAlgorithm,T> || std::is_base_of_v<EL::AnaReentrantAlgorithm,T>
  StatusCode registerGenericComponentFactory (const std::string& type)
  {
    return registerAlgorithmFactory<T> (type);
  }
  template<typename T> requires std::is_base_of_v<AsgService,T>
  StatusCode registerGenericComponentFactory (const std::string& type)
  {
    return registerServiceFactory<T> (type);
  }
  /// @}

  /// @brief a helper class to automatically register a component factory
  ///
  /// This is mostly useful to match the Gaudi/Athena convention with
  /// the `DECLARE_COMPONENT` macro.
  template<typename T> class AutoRegisterComponentFactory final
  {
  public:
    AutoRegisterComponentFactory (const std::string& name)
    {
      using namespace asg::msgComponentConfig;
      if (!asg::registerGenericComponentFactory<T>(name).isSuccess())
      {
        ANA_MSG_FATAL("Failed to register component factory for " << name);
        std::terminate();
      } else
      {
        ANA_MSG_DEBUG("Registered component factory for " << name);
      }
    }
  };
  #define DECLARE_COMPONENT_JOIN(x,y) DECLARE_COMPONENT_JOIN2(x,y)
  #define DECLARE_COMPONENT_JOIN2(x,y) x ## y
  #define DECLARE_COMPONENT(TYPE) \
    namespace { \
      asg::AutoRegisterComponentFactory<TYPE> DECLARE_COMPONENT_JOIN(autoRegisterComponent,__LINE__) (#TYPE); \
    }

  /// @brief get the list of loaded component factory types
  std::vector<std::string> getLoadedComponentFactoryTypes ();

  /// @brief load a component factory module, optionally specifying the module path name
  void loadComponentFactoryModule (const std::string& moduleName, const std::string& modulePath = "");

  /// @brief load a component factory map from the given path
  void loadComponentFactoryMap (const std::string& path);

#endif
}

#endif
