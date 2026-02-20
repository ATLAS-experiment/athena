// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "IOUtils.h"

// Framework include(s).
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/DataVector.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainersInterfaces/IConstAuxStore.h"

// System include(s).
#include <stdexcept>

namespace {

/// Helper class for exposing the @c SG::AuxVectorBase::initAuxVectorBase
/// function
class ForceTrackIndices : public SG::AuxVectorBase {
 public:
  using SG::AuxVectorBase::initAuxVectorBase;
};  // class ForceTrackIndices

}  // anonymous namespace

namespace xAOD::Details {

void forceTrackIndices NO_SANITIZE_UNDEFINED(SG::AuxVectorBase& vec) {

  // Treat the received object like it would be of type @c ForceTrackIndices
  ForceTrackIndices& xvec = static_cast<ForceTrackIndices&>(vec);

  // Which would allow us to call a protected function on it.
  xvec.initAuxVectorBase<DataVector<SG::IAuxElement> >(
      SG::OWN_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
}

bool hasAuxStore(const TClass& cl) {

  // The classes whose children can have an auxiliary store attached
  // to them.
  static const TClass* const dvClass =
      ::TClass::GetClass(typeid(SG::AuxVectorBase));
  static const TClass* const aeClass =
      ::TClass::GetClass(typeid(SG::AuxElement));

  // Do the check.
  return (cl.InheritsFrom(dvClass) || cl.InheritsFrom(aeClass));
}

bool isAuxStore(const TClass& cl) {

  // The classes whose children are considered auxiliary stores.
  static const TClass* const storeClass =
      ::TClass::GetClass(typeid(SG::IConstAuxStore));
  static const TClass* const storeHolderClass =
      ::TClass::GetClass(typeid(SG::IAuxStoreHolder));

  // Do the check.
  return (cl.InheritsFrom(storeClass) || cl.InheritsFrom(storeHolderClass));
}

bool isStandalone(const TClass& cl) {

  // The classes whose children can have an auxiliary store attached
  // to them:
  static const TClass* const dvClass =
      TClass::GetClass(typeid(SG::AuxVectorBase));
  static const TClass* const aeClass = TClass::GetClass(typeid(SG::AuxElement));

  // Do the check:
  if (cl.InheritsFrom(aeClass)) {
    return kTRUE;
  } else if (cl.InheritsFrom(dvClass)) {
    return kFALSE;
  }

  // Some logic error happened.
  throw std::runtime_error("A logic error happened in the code");
}

}  // namespace xAOD::Details
