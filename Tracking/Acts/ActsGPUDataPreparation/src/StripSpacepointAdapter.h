/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_STRIPSPACEPOINTADAPTER_H
#define ACTSGPUDATAPREPARATION_STRIPSPACEPOINTADAPTER_H

#include <vecmem/memory/memory_resource.hpp>

#include "traccc/seeding/strip_spacepoint_formation_data.hpp"

class SCT_ID;
namespace InDetDD {
class SiDetectorElementCollection;
}
namespace InDet {
class SiElementPropertiesTable;
}

namespace ActsTrk {
class GeometryIdMapping;

/// Host inputs share the caller's memory resource, which must outlive them.
struct StripSpacepointInputs {
  traccc::strip_measurement_surface_info_collection_types::host surface_infos;
  traccc::strip_pairing_rule_collection_types::host pairing_rules;
  explicit StripSpacepointInputs(vecmem::memory_resource& mr)
      : surface_infos(&mr), pairing_rules(&mr) {}
};

/// Interpret Athena designs/topology using the reference CPU pairing cuts.
/// Throws on inconsistent geometry; no device state is modified here.
StripSpacepointInputs makeStripSpacepointInputs(
    const InDetDD::SiDetectorElementCollection& elements,
    const SCT_ID& strip_id, const InDet::SiElementPropertiesTable& properties,
    const GeometryIdMapping& mapping, vecmem::memory_resource& mr);

}  // namespace ActsTrk

#endif  // ACTSGPUDATAPREPARATION_STRIPSPACEPOINTADAPTER_H
