/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_DEVICESTRIPSPFORMATIONALG_H
#define ACTSGPUDATAPREPARATION_DEVICESTRIPSPFORMATIONALG_H

#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccStripSpacepointShape.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "IDeviceStripSPFormationAlgProviderTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace ActsTrk {
/// Produce standard and overlap Strip SPs using the device provider.
/// Read IOV-dependent device inputs prepared by StripSpacepointInputsCondAlg;
/// measurements and beam spot are read separately for each event.
class DeviceStripSPFormationAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  Gaudi::Property<bool> m_retainShapes{
      this, "RetainShapes", true,
      "Keep GPU-computed strip shapes for xAOD conversion"};
  SG::WriteHandleKey<traccc::strip_spacepoint_shape_collection_types::buffer>
      m_shapes{this, "OutputStripShapes", "TracccStripSpacepointShapes",
               "Standard strip shapes"};
  SG::WriteHandleKey<traccc::strip_spacepoint_shape_collection_types::buffer>
      m_overlapShapes{this, "OutputStripOverlapShapes",
                      "TracccStripOverlapSpacepointShapes",
                      "Overlap strip shapes"};
  ToolHandle<IDeviceStripSPFormationAlgProviderTool> m_provider{
      this, "SPFormationAlgProviderTool", "", "Strip SP backend provider"};
  SG::ReadHandleKey<traccc::edm::measurement_collection::const_view>
      m_measurements{this, "InputTracccMeasurements",
                     "TracccMeasurementCollection", "Device measurements"};
  SG::WriteHandleKey<traccc::edm::spacepoint_collection::buffer> m_standard{
      this, "OutputTracccStripSpacepoints", "TracccStripSpacepointCollection",
      "Standard SPs"};
  SG::WriteHandleKey<traccc::edm::spacepoint_collection::buffer> m_overlap{
      this, "OutputTracccStripOverlapSpacepoints",
      "TracccStripOverlapSpacepointCollection", "Overlap SPs"};
  SG::ReadCondHandleKey<StripSpacepointConditions> m_inputs{
      this, "InputStripSPConditions", "TracccStripSpacepointConditions",
      "IOV-dependent device inputs"};
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpot{
      this, "BeamSpotKey", "BeamSpotData", "Beam spot"};
};
}  // namespace ActsTrk
#endif
