/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_STRIPSPACEPOINTINPUTSCONDALG_H
#define ACTSGPUDATAPREPARATION_STRIPSPACEPOINTINPUTSCONDALG_H

#include "ActsGPUEvent/GeometryIdMapping.h"
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "IDeviceStripSPFormationAlgProviderTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StripSpacepointConditions.h"

class SCT_ID;
namespace ActsTrk {
/// Prepare and upload Strip SP inputs once for the intersection of the aligned
/// detector-element and neighbour-properties IOVs. No event data is retained.
class StripSpacepointInputsCondAlg : public AthCondAlgorithm {
 public:
  using AthCondAlgorithm::AthCondAlgorithm;
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_elements{
      this, "StripDetectorElements", "ITkStripDetectorElementCollection",
      "Aligned elements"};
  SG::ReadCondHandleKey<InDet::SiElementPropertiesTable> m_properties{
      this, "StripProperties", "ITkStripElementPropertiesTable",
      "Neighbour properties"};
  SG::WriteCondHandleKey<StripSpacepointConditions> m_output{
      this, "OutputStripSPConditions", "TracccStripSpacepointConditions",
      "IOV-dependent device inputs"};
  ToolHandle<IDeviceStripSPFormationAlgProviderTool> m_provider{
      this, "SPFormationAlgProviderTool", "", "Strip SP backend provider"};
  Gaudi::Property<std::string> m_mappingName{this, "GeoIdMapping",
                                             "TracccGeometryIdMapping",
                                             "Static geometry ID mapping"};
  const GeometryIdMapping* m_mapping{nullptr};
  const SCT_ID* m_stripId{nullptr};
};
}  // namespace ActsTrk
#endif
