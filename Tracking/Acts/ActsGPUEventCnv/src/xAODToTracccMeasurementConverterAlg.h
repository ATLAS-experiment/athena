/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_XAODTOTRACCCMEASUREMENTCONVERTERALG_H
#define ACTSGPUEVENTCNV_XAODTOTRACCCMEASUREMENTCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "ActsGPUEvent/GeometryIdMapping.h"

#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "SGTools/StlVectorClids.h"

#include <array>
#include <atomic>
#include <memory_resource>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ActsTrk {

/**
 * @class xAODToTracccMeasurementConverterAlg
 *
 * @brief Algorithm converting xAOD clusters (host container) to traccc measurements (device buffer)
 *
 * This algorithm retrieves the xAOD pixel and strip cluster containers from the event store,
 * converts them to a single traccc measurement collection sorted by detray surface identifier,
 * copies the collection to the device and records the resulting device resident buffer
 * in the event store.
 *
 * The mappings from the traccc measurement indices to the xAOD cluster indices
 * are recorded in the event store, so that downstream algorithms can relate
 * device resident objects referencing the measurements to the host clusters.
 *
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */
class xAODToTracccMeasurementConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;
  /// Function finalizing the algorithm
  virtual StatusCode finalize() override;

private:
  /// Host side description of a measurement before it is written to the traccc collection
  struct MeasurementRecord {
    std::uint64_t geometryId{0};
    unsigned int condIndex{0};
    bool isPixel{true};
    unsigned int hostIndex{0};
    std::array<float, 2u> localPosition{0.f, 0.f};
    std::array<float, 2u> localVariance{0.f, 0.f};
    float diameter{0.f};
  };

  /// Look up the traccc conditions index for an Athena module identifier
  StatusCode condIndexFor(const Identifier& moduleId, unsigned int& condIndex) const;

  /// @name The input host resident cluster container names
  /// {@
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClustersKey{
      this, "InputPixelClusters", "ITkPixelClusters", "Input xAOD pixel clusters"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClustersKey{
      this, "InputStripClusters", "ITkStripClusters", "Input xAOD strip clusters"};
  /// @}

  /// @name The output device resident measurement collection name
  /// {@
  SG::WriteHandleKey<traccc::edm::measurement_collection::buffer> m_outputMeasKey{
      this, "OutputTracccMeasurements", "TracccMeasurementCollection",
      "Output traccc measurement collection buffer"};
  /// @}

  /// @name The output host resident index map names
  /// {@
  SG::WriteHandleKey<std::vector<unsigned int>> m_outputMeasToPixelKey{
      this, "OutputMeasToPixelCluster", "TracccMeasToPixelCluster",
      "Output mapping from traccc measurement index to the index of the xAOD pixel cluster in its owning container"};
  SG::WriteHandleKey<std::vector<unsigned int>> m_outputMeasToStripKey{
      this, "OutputMeasToStripCluster", "TracccMeasToStripCluster",
      "Output mapping from traccc measurement index to the index of the xAOD strip cluster in its owning container"};
  /// @}

  /// @name The memory resources and copy tools
  /// {@
  ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{
      this, "MemoryResourcesTool", "",
      "The memory resources tool to use for allocating memory on the host and the device"};
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool{
      this, "CopiesTool", "", "Tool that provides host and device copy objects"};
  /// @}

  /// @name The detector description objects retrieved from the detector store
  /// {@
  Gaudi::Property<std::string> m_geoIdMappingObjectName{
      this, "GeoIdMappingObjectName", "TracccGeometryIdMapping",
      "StoreGate name for the detray/acts/athena geo id mapping"};
  SG::ReadCondHandleKey<traccc::detector_conditions_description::host> m_hostCondKey{
      this, "HostConditionsObjectName", "TracccHostCondConfig",
      "Traccc host conditions object"};
  Gaudi::Property<std::string> m_hostDesignObjectName{
      this, "HostDigitizationObjectName", "TracccHostDigitizationConfig",
      "Traccc host digitization object"};
  const ActsTrk::GeometryIdMapping* m_geoIdMapping{nullptr};
  const traccc::detector_design_description::host* m_hostDesign{nullptr};
  /// @}

  const PixelID* m_pixelID{nullptr};
  const SCT_ID* m_stripID{nullptr};

  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<unsigned long> m_nPixel = 0;
  mutable std::atomic<unsigned long> m_nStrip = 0;
  /// @}
};

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_XAODTOTRACCCMEASUREMENTCONVERTERALG_H
