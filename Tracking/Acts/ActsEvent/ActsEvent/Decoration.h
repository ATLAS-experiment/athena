/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_Decoration_h
#define ActsEvent_Decoration_h

#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/IConstAuxStore.h"
#include "xAODCore/AuxContainerBase.h"

namespace ActsTrk {
using IndexType = std::uint32_t;  // TODO take from a common header
namespace detail {
using SetterType =
    std::function<std::any(SG::IAuxStore*, ActsTrk::IndexType, SG::auxid_t)>;
using GetterType = std::function<const std::any(
    const SG::IConstAuxStore*, ActsTrk::IndexType, SG::auxid_t)>;
using CopierType =
    std::function<void(SG::IAuxStore*, ActsTrk::IndexType, SG::auxid_t, const std::any&)>;

struct Decoration {
  std::string name;                    // for our info
  uint32_t hash = 0;                   // Acts API comes with this
  SG::auxid_t auxid = SG::null_auxid;  // xAOD talks with this
  GetterType getter = nullptr;            // type aware accessors
  CopierType copier = nullptr;
  SetterType setter = nullptr;
};

template <typename T>
struct accepted_decoration_types {
  constexpr static bool value =
      std::is_same<T, float>::value or std::is_same<T, double>::value or
      std::is_same<T, short>::value or std::is_same<T, int>::value or
      std::is_same<T, std::uint32_t>::value;
};

// getter that is good for non-mutable containers
template <typename T>
const std::any constDecorationGetter(const SG::IConstAuxStore* container,
                                     ActsTrk::IndexType idx,
                                     SG::auxid_t decorationId) {
  const void* data = container->getData(decorationId);
  return &(static_cast<const T*>(data)[idx]);
}
// getter that is good for mutable containers (returns const ptr wrapped in
template <typename T>
const std::any decorationGetter(const SG::IAuxStore* container,
                                ActsTrk::IndexType idx,
                                SG::auxid_t decorationId) {
  const void* data = container->getData(decorationId);
  return &(static_cast<T*>(data)[idx]);
}

// setter for mutable containers (i.e. provides non const ptr wrapped in
// std::any)
template <typename T>
std::any decorationSetter(SG::IAuxStore* container, ActsTrk::IndexType idx,
                          SG::auxid_t decorationId) {
  void* data = container->getData(decorationId, idx + 1, idx + 1);
  return &(static_cast<T*>(data)[idx]);
}

template <typename T>
void decorationCopier(SG::IAuxStore* dst, ActsTrk::IndexType dst_idx,
                      SG::auxid_t decorationId, const std::any& src_ptr) {
  *std::any_cast<T*>(decorationSetter<T>(dst, dst_idx, decorationId)) =
      *std::any_cast<const T*>(src_ptr);
}

template <typename T>
static Decoration decoration(std::string_view n, GetterType g, CopierType c,
                          SetterType s = static_cast<SetterType>(nullptr)) {
  Decoration dec;
  dec.name = n;
  dec.hash = Acts::hashStringDynamic(n);
  dec.auxid = SG::AuxTypeRegistry::instance().getAuxID<T>(dec.name);
  if (dec.auxid == SG::null_auxid)
    throw std::runtime_error("ActsTrk::Decoration Aux ID for " + dec.name +
                             " could not be found");
  dec.getter = std::move(g);
  dec.copier = std::move(c);
  dec.setter = std::move(s);
  return dec;
}


/**
* @arg container - source container to look for decorations
* @arg staticVaraibles - set of names of predefined variables for this container
*/
std::vector<Decoration> restoreDecorations(
    const SG::IConstAuxStore* container,
    const std::set<std::string>& staticVariables);

}  // namespace detail
}  // namespace ActsTrk

#endif
