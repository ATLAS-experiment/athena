/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRKPARAMCONVERTERALG_H
#define ACTSGPUEVENT_TRACCCTRKPARAMCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsGPUEvent/TracccTrkParam.h"
#include "src/TracccTrkParamConversionHelper.h"

#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

#include "ActsEvent/SeedMeasurementsContainer.h"
#include "ActsEvent/TrackParametersContainer.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class TracccTrkParamConverterAlg
 *
 * @brief Algorithm converting the track parameters packed on device (TracccTrkParamDeviceConverterAlg) to the host containers the CKF reads
 *
 * This algorithm copies the TracccTrkParam records, i.e. the track parameters estimated on
 * device for each traccc seed and the measurement indices of each seed, to the host in
 * one copy. It converts them into
 * an ActsTrk::BoundTrackParametersContainer and an index-aligned ActsTrk::SeedMeasurementsContainer,
 * which the CPU track finding can use as seeds instead of space point seeds.
 * Neither the traccc space points nor the traccc seeds are needed on the host.
 * It works under the assumption that the traccc measurements have already been converted
 * to xAOD clusters by the TracccMeasurementConverterAlg, together with the maps
 * from traccc measurement index to xAOD cluster index.
 */
class TracccTrkParamConverterAlg : public AthReentrantAlgorithm
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

  /// @name The input device resident packed track parameters (TracccTrkParam)
  SG::ReadHandleKey<TracccTrkParamBuffer> m_inputPackedTrkParamsKey{
      this, "InputPackedTrkParams", "",
      "Input buffer with the packed track parameters and measurement indices of each seed"};

  /// @name The input host resident clusters converted from traccc and the index maps
  /// {@
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClustersKey{
      this, "InputPixelClusters", "",
      "Input xAOD pixel clusters converted from Traccc"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClustersKey{
      this, "InputStripClusters", "",
      "Input xAOD strip clusters converted from Traccc"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_inputMeasToPixelSPKey{
      this, "InputMeasToPixelSP", "",
      "Input mapping from traccc measurement index to pixel cluster index"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_inputMeasToStripClKey{
      this, "InputMeasToStripCl", "",
      "Input mapping from traccc measurement index to strip cluster index"};
  /// @}

  /// @name The output host resident containers read by the CKF
  /// {@
  SG::WriteHandleKey<ActsTrk::BoundTrackParametersContainer> m_outputTrackParamsKey{
      this, "OutputTrackParameters", "",
      "Output initial track parameters of the seeds"};
  SG::WriteHandleKey<ActsTrk::SeedMeasurementsContainer> m_outputSeedMeasurementsKey{
      this, "OutputSeedMeasurements", "",
      "Output measurements of the seeds, index-aligned with the track parameters"};
  /// @}

  /// @name The host memory resource tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource tool"};
  /// @name The copy tool used for copying data from device
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};

  ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
  /// Detray -> ACTS surface map built by the ACTS Detray plugin, owned by the TrackingGeometrySvc
  const ActsPlugins::DetraySurfaceIdMap* m_detraySurfaceIds = nullptr;

  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<int> m_nSeeds = 0;
  mutable std::atomic<int> m_nNoParameters = 0;
  /// @}
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCTRKPARAMCONVERTERALG_H
