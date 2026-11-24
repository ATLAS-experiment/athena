/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_TRACCCSTRIPSPACEPOINTCONVERTERALG_H
#define ACTSGPUEVENTCNV_TRACCCSTRIPSPACEPOINTCONVERTERALG_H
#include <vector>

#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccStripSpacepointShape.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

namespace ActsTrk {
/// Copy device Strip SPs to xAOD and resolve their cluster links using the
/// measurement-index mapping from TracccMeasurementConverterAlg. Retained
/// shape records follow the SP order within each input collection.
/// @author Taisei Kato
class TracccStripSpacepointConverterAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  Gaudi::Property<bool> m_useGPUShapes{this, "UseGPUShapes", true,
                                       "Use shapes retained by GPU formation "
                                       "instead of reconstructing endpoints"};
  SG::ReadHandleKey<traccc::strip_spacepoint_shape_collection_types::buffer> m_shapes{
      this, "InputStripShapes", "TracccStripSpacepointShapes", "Standard GPU shapes"};
  SG::ReadHandleKey<traccc::strip_spacepoint_shape_collection_types::buffer> m_overlapShapes{
      this, "InputStripOverlapShapes", "TracccStripOverlapSpacepointShapes", "Overlap GPU shapes"};
  ToolHandle<AthDevice::ICopyTool> m_copy{this, "CopyProviderTool", "", "Device copy"};
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{this, "HostMR", "", "Host allocator"};
  SG::ReadHandleKey<traccc::edm::spacepoint_collection::buffer> m_standard{
      this, "InputStripSpacepoints", "TracccStripSpacepointCollection", "Standard GPU SPs"};
  SG::ReadHandleKey<traccc::edm::spacepoint_collection::buffer> m_overlap{
      this, "InputStripOverlapSpacepoints", "TracccStripOverlapSpacepointCollection", "Overlap GPU SPs"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_clusters{this, "StripClusters", "ITkTracccStripClusters",
                                                            "Converted GPU clusters"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_mapping{this, "InputMeasToStripCl", "ITkTracccMeasToStripCl",
                                                         "Measurement index to strip cluster index"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_elements{
      this, "StripDetectorElements", "ITkStripDetectorElementCollection", "Aligned elements"};
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_outputStandard{this, "StripSpacePoints", "ITkTracccStripSpacePoints",
                                                                 "Standard xAOD SPs"};
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_outputOverlap{this, "StripOverlapSpacePoints",
                                                                "ITkTracccStripOverlapSpacePoints", "Overlap xAOD SPs"};
};
}  // namespace ActsTrk
#endif
