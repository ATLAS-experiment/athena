// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "isRegisteredType.h"

// Framework include(s).
#include "AthContainers/AuxTypeRegistry.h"

namespace xAOD::details {

bool isRegisteredType(SG::auxid_t auxid) {

  // Cache some data:
  static SG::AuxTypeRegistry& registry ATLAS_THREAD_SAFE =
      SG::AuxTypeRegistry::instance();
  static const SG::auxid_t sauxid =
      registry.getAuxID<SG::AuxTypePlaceholder>("AuxTypePlaceholder");
  static const std::type_info* const sti = registry.getVecType(sauxid);

  // Check if the types match:
  return (*sti != *(registry.getVecType(auxid)));
}

}  // namespace xAOD::details
