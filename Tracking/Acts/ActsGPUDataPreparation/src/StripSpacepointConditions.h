/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_STRIPSPACEPOINTCONDITIONS_H
#define ACTSGPUDATAPREPARATION_STRIPSPACEPOINTCONDITIONS_H

#include <memory>
#include <stdexcept>

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "traccc/seeding/strip_spacepoint_formation_data.hpp"

namespace ActsTrk {
/// Read-only device inputs shared by events in the same alignment/topology IOV.
/// The allocator is owned here, so Conditions Store cleanup does not depend on
/// the lifetime of an Algorithm's private memory-resource Tool or service.
class StripSpacepointConditions {
 private:
  static vecmem::memory_resource& checkedResource(
      const std::unique_ptr<vecmem::memory_resource>& resource) {
    if (!resource)
      throw std::invalid_argument("Missing Strip SP conditions allocator");
    return *resource;
  }
  // Declared first: buffers must be destroyed before their allocator.
  std::unique_ptr<vecmem::memory_resource> m_resource;

 public:
  StripSpacepointConditions(std::unique_ptr<vecmem::memory_resource> resource,
                            std::size_t surfaceCount, std::size_t ruleCount)
      : m_resource(std::move(resource)),
        surfaces(surfaceCount, checkedResource(m_resource)),
        rules(ruleCount, checkedResource(m_resource)) {}

  traccc::strip_measurement_surface_info_collection_types::buffer surfaces;
  traccc::strip_pairing_rule_collection_types::buffer rules;
};
}  // namespace ActsTrk
CLASS_DEF(ActsTrk::StripSpacepointConditions, 98063622, 1)
CONDCONT_DEF(ActsTrk::StripSpacepointConditions, 25292202);
#endif
