/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGNAVIGATION_TYPEREGISTRATION_H
#define TRIGNAVIGATION_TYPEREGISTRATION_H

#include "TrigNavigation/NavigationInit.h"
#include "TrigStorageDefinitions/EDM_TypeInfo.h"
#include <type_traits>


/// Register feature type
struct registertype {
  template<typename TypeInfoElement>
  void operator()() const {
    using FEATURES  = typename TypeInfoElement::list_of_features;
    using CONTAINER = typename TypeInfoElement::container;
    using AUX       = typename TypeInfoElement::aux;

    // Register feature container
    FEATURES::for_each( []<typename FEATURE>() {
      HLT::RegisterFeatureContainerTypes<FEATURE, CONTAINER>::instan();} );

    // Register Aux container if needed
    if constexpr (!std::is_same_v<AUX, HLT::TypeInformation::no_aux>) {
      HLT::RegisterAuxType<AUX>::instan();
    }
  }
};


/// Use inline static variable initialization to execute the registration at startup
template <typename TYPELIST>
inline const bool trignav_register = []() {
  TYPELIST::for_each(registertype{});
  return true;
}();


/// Macro to register a package with the Navigation
#define REGISTER_PACKAGE_WITH_NAVI(name) \
  inline const bool trignav_registry_##name = trignav_register<class_##name::map>;


#endif // TRIGNAVIGATION_TYPEREGISTRATION_H
