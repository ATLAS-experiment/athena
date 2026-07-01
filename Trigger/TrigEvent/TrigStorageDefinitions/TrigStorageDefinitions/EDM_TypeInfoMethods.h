/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEF_TYPEINFO_METHODS
#define TRIGSTORAGEDEF_TYPEINFO_METHODS

#include "TrigStorageDefinitions/TypeInformation.h"

#include <type_traits>

// Forward declaration
struct TypeInfo_EDM;


/// Check if type T is known in EDMLIST. Compilation failure if not.
template<class T, class EDMLIST = TypeInfo_EDM>
struct IsKnownFeature {
  static constexpr bool value = EDMLIST::map::template has<T, HLT::TypeInformation::MatchFeatures>;
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
  using type = EDMLIST::map::template find<CONTAINER, HLT::TypeInformation::MatchContainer>::object;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Container2Object_t = Container2Object<FEATURE, EDMLIST>::type;


/// Get Aux type for container
template<class CONTAINER, class EDMLIST = TypeInfo_EDM>
struct Container2Aux {
  using type = EDMLIST::map::template find<CONTAINER, HLT::TypeInformation::MatchContainer>::aux;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Container2Aux_t = Container2Aux<FEATURE, EDMLIST>::type;


/// Get container type for object
template<class OBJECT, class EDMLIST = TypeInfo_EDM>
struct Object2Container {
  using type = EDMLIST::map::template find<OBJECT, HLT::TypeInformation::MatchObject>::container;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Object2Container_t = Object2Container<FEATURE, EDMLIST>::type;


/// Get container type for feature
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
struct Features2Container {
  using type = EDMLIST::map::template find<FEATURE, HLT::TypeInformation::MatchFeatures>::container;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Features2Container_t = Features2Container<FEATURE, EDMLIST>::type;


/// Get object type for feature
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
struct Features2Object {
  using type = EDMLIST::map::template find<FEATURE, HLT::TypeInformation::MatchFeatures>::object;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Features2Object_t = Features2Object<FEATURE, EDMLIST>::type;


/// Get object type for feature list
template<class OBJECT, class EDMLIST = TypeInfo_EDM>
struct Object2Features {
  using type = EDMLIST::map::template find<OBJECT, HLT::TypeInformation::MatchObject>::list_of_features;
};
template<class FEATURE, class EDMLIST = TypeInfo_EDM>
using Object2Features_t = Object2Features<FEATURE, EDMLIST>::type;

#endif
