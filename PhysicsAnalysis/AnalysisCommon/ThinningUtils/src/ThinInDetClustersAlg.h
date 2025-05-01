///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef THINNINGUTILS_ThinInDetClustersAlg_H
#define THINNINGUTILS_ThinInDetClustersAlg_H 1

/**
 @class ThinInDetClusterAlg
*/

//////////////////////////
// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "ExpressionEvaluation/ExpressionParserUser.h"

// STL includes
#include <atomic>
#include <string>
#include <vector>

class ThinInDetClustersAlg final : public ExpressionParserUser<::AthReentrantAlgorithm>
{
 public:
  /// Constructor with parameters:
  ThinInDetClustersAlg(const std::string& name, ISvcLocator* pSvcLocator);
  
  /// Destructor:
  virtual ~ThinInDetClustersAlg() = default;

  /// Athena algorithm's initalize hook
  virtual StatusCode initialize() override;

  /// Athena algorithm's execute hook
  virtual StatusCode execute(const EventContext& ctx) const override;

  /// Athena algorithm's finalize hook
  virtual StatusCode finalize() override;

 private:
  /// Counters and keys for xAOD::TrackParticle container
  mutable std::atomic<unsigned int> m_ntot{};
  mutable std::atomic<unsigned int> m_npass{};
  /// Thinning logic 
  BooleanProperty m_thinPixelHitsOnTrack
  { this, "ThinPixelHitsOnTrack", false, ""};
  BooleanProperty m_thinSCTHitsOnTrack
  { this, "ThinSCTHitsOnTrack", false, ""};
  BooleanProperty m_thinTRTHitsOnTrack
  { this, "ThinTRTHitsOnTrack", false, ""};
  
  /// Stream for object thinning selections
  StringProperty m_streamName
  { this, "StreamName", "", "Name of the stream being thinned" };

  /// Expressions for object thinning selections
  /// Default InDetTrackParticles (also accomplished through flag.Tracking.thinPixelClustersSelectionStrings defaults)
  
  StringProperty m_selectionString
  { this, "SelectionString", "InDetTrackParticles.pt>10*GeV", "Selection string for each TrackParticle container" };

  /// SGKey for TrackParticleContainer
  SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_inDetSGKey
  { this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };

  /// Counters and keys for xAOD::TrackStateValidation and xAOD::TrackMeasurementValidation containers
  mutable std::atomic<unsigned int> m_ntot_pix_states{};
  mutable std::atomic<unsigned int> m_npass_pix_states{};
  SG::ThinningHandleKey<xAOD::TrackStateValidationContainer> m_statesPixSGKey
  { this, "InDetTrackStatesPixKey", "PixelMSOSs", "" }; // original
  
  mutable std::atomic<unsigned int> m_ntot_pix_measurements{};
  mutable std::atomic<unsigned int> m_npass_pix_measurements{};
  SG::ThinningHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurementsPixSGKey
  { this, "InDetTrackMeasurementsPixKey", "PixelClusters", "" };
  
  mutable std::atomic<unsigned int> m_ntot_sct_states{};
  mutable std::atomic<unsigned int> m_npass_sct_states{};
  SG::ThinningHandleKey<xAOD::TrackStateValidationContainer> m_statesSctSGKey
  { this, "InDetTrackStatesSctKey", "SCT_MSOSs", "" };
  
  mutable std::atomic<unsigned int> m_ntot_sct_measurements{};
  mutable std::atomic<unsigned int> m_npass_sct_measurements{};
  SG::ThinningHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurementsSctSGKey
  { this, "InDetTrackMeasurementsSctKey", "SCT_Clusters", "" };
  
  mutable std::atomic<unsigned int> m_ntot_trt_states{};
  mutable std::atomic<unsigned int> m_npass_trt_states{};
  SG::ThinningHandleKey<xAOD::TrackStateValidationContainer> m_statesTrtSGKey
  { this, "InDetTrackStatesTrtKey", "TRT_MSOSs", "" };
  
  mutable std::atomic<unsigned int> m_ntot_trt_measurements{};
  mutable std::atomic<unsigned int> m_npass_trt_measurements{};
  SG::ThinningHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurementsTrtSGKey
  { this, "InDetTrackMeasurementsTrtKey", "TRT_DriftCircles", "" };
  
  /// For P->T converter of SCT_Clusters
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  /// Track state types
  /// Subset of enum MeasurementType from Athena: Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs.h
  enum MeasurementType {
	TrkState_unidentified = 0,
	TrkState_Pixel      = 1,
	TrkState_SCT        = 2,
	TrkState_TRT        = 3,
	TrkState_Pseudo     = 8,
	TrkState_Vertex     = 9,
	TrkState_SpacePoint = 11,
	TrkState_NumberOfMeasurementTypes=16
  };

  /// Select TrackStateValidation and TrackMeasurementValidation objects that are used in the (thinned) track container
  void selectTrackHits(const xAOD::TrackParticleContainer& inputTrackParticles,
					   const std::vector<bool>& inputMask,
					   MeasurementType detTypeToSelect,
					   std::vector<bool>& outputStatesMask, std::vector<bool>& outputMeasurementsMask) const;
  
  StatusCode filterTrackHits
	(MeasurementType detTypeToSelect,
	 const xAOD::TrackParticleContainer& inputTrackParticles,
	 const std::vector<bool>& inputMask,
	 const SG::ThinningHandleKey<xAOD::TrackStateValidationContainer>& statesKey,
	 const SG::ThinningHandleKey<xAOD::TrackMeasurementValidationContainer>& measurementsKey,
	 std::atomic<unsigned int>& ntot_states,
	 std::atomic<unsigned int>& npass_states,
	 std::atomic<unsigned int>& npass_measurements,
	 const EventContext& ctx) const;
}; 

#endif //> !THINNINGUTILS_ThinInDetClustersAlg_H

