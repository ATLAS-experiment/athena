/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEF_TYPEINFO_METHODS
#define TRIGSTORAGEDEF_TYPEINFO_METHODS

#include "TrigStorageDefinitions/EDM_MasterSearch.h"
#include <type_traits>

// Forward declaration
struct TypeInfo_EDM;


/// Check if type T is known in EDMLIST. Compilation failure if not.
template<class T, class EDMLIST = TypeInfo_EDM>
struct IsKnownFeature {
  using search_result = master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_feat,T>::result::search_result;

  static constexpr bool value = !std::is_same_v<HLT::TypeInformation::ERROR_THE_FOLLOWING_TYPE_IS_NOT_KNOWN_TO_THE_EDM<T>, search_result>;
  static_assert("The following class is not a valid feature. Contact Trigger Core SW experts.");
};


/// Get link type for feature
template<class REQUESTED, class CONTAINER>
using Features2LinkHelper_t = std::conditional_t<
  std::is_same_v<REQUESTED, CONTAINER>,
  DataLink<CONTAINER>,
  ElementLink<CONTAINER>
  >;


//
// Type "converters" from/to object, features and containers.
//
// Note that we cannot implement these as pure alias templates since TypeInfo_EDM
// is only forward declared here. We provide the usual "_t" helper type.
//

/// Get object type within container
template<class CONTAINER, class EDMLIST = TypeInfo_EDM>
struct Container2Object {
  using type = typename master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_cont,
                                      CONTAINER>::result::search_result::object;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Container2Object_t = Container2Object<FEATURE, EDMLIST>::type;


/// Get Aux type for container
template<class CONTAINER, class EDMLIST = TypeInfo_EDM>
struct Container2Aux {
  using type = typename master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_cont,
                                      CONTAINER>::result::search_result::aux;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Container2Aux_t = Container2Aux<FEATURE, EDMLIST>::type;


/// Get container type for object
template<class OBJECT, class EDMLIST = TypeInfo_EDM>
struct Object2Container {
  using type = typename master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_objt,
                                      OBJECT>::result::search_result::container;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Object2Container_t = Object2Container<FEATURE, EDMLIST>::type;


/// Get container type for feature
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
struct Features2Container {
  using type = typename master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_feat,
                                      FEATURE>::result::search_result::container;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Features2Container_t = Features2Container<FEATURE, EDMLIST>::type;


/// Get object type for feature
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
struct Features2Object {
  using type = typename master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_feat,
                                      FEATURE>::result::search_result::object;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Features2Object_t = Features2Object<FEATURE, EDMLIST>::type;


/// Get object type for feature list
template<class OBJECT, class EDMLIST = TypeInfo_EDM>
struct Object2Features {
  using type = typename master_search<typename EDMLIST::map,
                                      HLT::TypeInformation::get_objt,
                                      OBJECT>::result::search_result::list_of_features;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Object2Features_t = Object2Features<FEATURE, EDMLIST>::type;


#endif
