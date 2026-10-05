/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRACKCONVERTERALG_H
#define ACTSGPUEVENT_TRACCCTRACKCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsGPUEvent/TracccTrackContainer.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/GeometryIdMapping.h"

#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

#include "GaudiKernel/ToolHandle.h"

enum TrackValidity { kValid, kInvalidState, kInvalidGlobalParams };

namespace ActsTrk {

/**
 * @class TracccTrackConverterAlg
 *
 * @brief Algorithm converting traccc tracks ( tracks + track states sdevice buffer) to ACTS tracks (host container)
 *
 * This algorithm retrieves the input device resident traccc track and track states collection from the event store,
 * copies the data to host buffer and converts the traccc tracks to ActsTrk::Track.
 * It works under the assumption that the other relevant traccc objects 
 * have already been converted to xAOD and are available on host in the event store
 * and that the indices between the HOST and DEVICE resident containers have been recorded in a index map. 
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class TracccTrackConverterAlg : public AthReentrantAlgorithm
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

  // helper functions
  const xAOD::UncalibratedMeasurement& makeSourceLink(
    const unsigned int& meas_index,
    std::span<const unsigned int> pixelMap,
    std::span<const unsigned int> stripMap,
    const xAOD::PixelClusterContainer& pixel_clusters,
    const xAOD::StripClusterContainer& strip_clusters) const;

  const Acts::Surface& findActsSurface(const Acts::GeometryIdentifier& actsID) const;

  std::optional<Acts::BoundTrackParameters> convertGlobalToActsParameters(
    traccc::bound_track_parameters<traccc::default_algebra> const& trkParams) const;

  template <typename state_t>
  std::optional<Acts::BoundTrackParameters> convertSmoothedToActsParameters(traccc::edm::track_state<state_t> const& state) const;

  /// @name The input device resident cluster, measurement and cell collection names
  /// {@
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClustersKey{
      this, "InputPixelClusters", "",
      "Input xAOD pixel clusters converted from Traccc"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClustersKey{
      this, "InputStripClusters", "",
      "Input xAOD strip clusters converted from Traccc"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_inputMeasToPixelSPKey{
      this, "InputMeasToPixelSP", "",
      "Input mapping from traccc measurement index to pixel spacepoint index"};    
  SG::ReadHandleKey<std::vector<unsigned int>> m_inputMeasToStripClKey{
      this, "InputMeasToStripCl", "",
      "Input mapping from traccc measurement index to strip cluster index"};            
  SG::ReadHandleKey<traccc_track_container::buffer> m_inputTracksKey{
      this, "InputTracks", "",
      "Input traccc track collection buffer"}; 
  /// @}

  /// @name The output host resident track container name
  /// {@
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_outputTracksKey{
      this, "OutputTracks", "",
      "Output ACTS track container"};
  /// @}

  /// @name The host memory resource tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource tool"};
  /// @name The copy tool used for copying data from device
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};

  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;    
  ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};    
  std::map<Acts::GeometryIdentifier, const Acts::Surface*> m_actsSurfaceMap;
  // acts helper for the output
  ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{
        this};
  Gaudi::Property<std::string> m_hostDetectorObjectName{
        this, "HostDetectorName", "",
        "Detray host detector object"};
  const traccc::host_detector* m_hostDetector = nullptr;
  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<int> m_nTracksIn = 0;
  mutable std::atomic<int> m_nTracksOut = 0;
  mutable std::atomic<int> m_nExcludedFitOutcome = 0;
  mutable std::atomic<int> m_nExcludedNoState = 0;
  mutable std::atomic<int> m_nExcludedBadNdf = 0;
  mutable std::atomic<int> m_nExcludedWeirdState = 0;
  mutable std::atomic<int> m_nExcludedWeirdGlobal = 0;
  /// @}    
  
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCTRACKCONVERTERALG_H