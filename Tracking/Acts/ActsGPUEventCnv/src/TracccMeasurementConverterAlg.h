/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCMEASUREMENTCONVERTERALG_H
#define ACTSGPUEVENT_TRACCCMEASUREMENTCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "traccc/edm/measurement_collection.hpp"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "SCT_ReadoutGeometry/SCT_BarrelModuleSideDesign.h"
#include "SCT_ReadoutGeometry/SCT_ForwardModuleSideDesign.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"


#include "ActsGPUInterfaces/IDeviceDetectorDescriptionProviderSvc.h"

#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class TracccMeasurementConverterAlg
 *
 * @brief Algorithm converting traccc measurements (device buffer) to xAOD clusters (host container)
 *
 * This algorithm retrieves the input traccc measurement collection device buffer from the event store,
 * copies the data to host buffer and converts the traccc measurements to xAOD clusters.
 * In case the clusters require the associated RDOs (e.g. for truth matching),
 * the algorithm also retrieves the traccc cluster collection device buffer from the event store,
 * and uses the cell indices stored in the traccc clusters to construct an RDO list associated with each xAOD cluster.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class TracccMeasurementConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;
  /// Function finalizing the algorthm
  virtual StatusCode finalize() override;

private:

  /// @name Bolean variable deciding weather the conversion includes cell to cluster association
  Gaudi::Property<bool> m_convertClustersWithCells{
      this, "ConvertClustersWithCells", true,
      "Whether to associate cells with the clusters."};

  /// @name The input device resident cluster, measurement and cell collection names
  /// {@
  SG::ReadHandleKey<traccc::edm::measurement_collection::buffer> m_inputMeasKey{
      this, "InputMeasurements", "TracccMeasurements",
      "Input traccc measurement collection buffer"};
  SG::ReadHandleKey<traccc::edm::silicon_cluster_collection::buffer> m_inputClusterKey{
      this, "InputClusters", "TracccClusters",
      "Input traccc cluster collection buffer"};
  SG::ReadHandleKey<traccc::edm::silicon_cell_collection::buffer> m_inputCellsKey{
      this, "InputCells", "TracccCells",
      "Input traccc cell collection buffer"};
  /// @}

  /// @name The output host resident cluster container names
  /// {@
  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_outputPixelKey{
      this, "OutputPixelClusters", "ITkTracccPixelClusters",
      "Output xAOD pixel cluster container"};
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_outputStripKey{
      this, "OutputStripClusters", "ITkTracccStripClusters",
      "Output xAOD strip cluster container"};
  /// @}

  /// @name The host memory resource tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource tool"};
  /// @name The copy tool used for copying data from device
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};
  /// @name The detector description service providing the Athena<->Detray ID map
  ServiceHandle<ActsTrk::IDeviceDetectorDescriptionProviderSvc> m_detDescSvc{
    this, "DetectorDescriptionSvc", "ActsTrk::JSONDeviceDetectorDescriptionProviderSvc"};

  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<int> m_nPix = 0;
  mutable std::atomic<int> m_nStrip = 0;
  mutable std::atomic<int> m_nMeas = 0;
  /// @}

  /// The athena <-> detray identifier map
  const std::unordered_map<uint64_t, Identifier>* m_detrayToAthena{};

  /// Conversion helpers (to retrieve module design, hash, etc.)
  /// {@
  const PixelID* m_pixelID{nullptr};
  const SCT_ID*  m_stripID{nullptr};
  Gaudi::Property<std::string> m_idHelperName {this, "IDHelperName", "PixelID",
    "Pixel-like ID helper name to retrieve from DetectorStore"};

  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager*  m_stripManager{nullptr};
  /// @}
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCMEASUREMENTCONVERTERALG_H