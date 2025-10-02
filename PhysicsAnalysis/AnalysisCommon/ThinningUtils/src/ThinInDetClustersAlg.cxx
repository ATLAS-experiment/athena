/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// ThinInDetClustersAlg.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Removes clusters associated with ID tracks which do not pass a user-defined cut
// Does NOT remove the tracks themselves

#include "ThinInDetClustersAlg.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// STL includes
#include <vector>
#include <string>

// FrameWork includes
#include "Gaudi/Property.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/ThinningHandle.h"
#include "ExpressionEvaluation/ExpressionParser.h"
#include "ExpressionEvaluation/ExpressionParserUser.icc"

///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructors
////////////////
ThinInDetClustersAlg::ThinInDetClustersAlg(
  const std::string& name,
  ISvcLocator* pSvcLocator): 

  ExpressionParserUser< ::AthReentrantAlgorithm>( name, pSvcLocator )
{
}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode ThinInDetClustersAlg::initialize()
{

  // Set up the text-parsing machinery for thinning the tracks directly according to user cuts
  if (m_selectionString.empty()) {
    ATH_MSG_FATAL("No inner detector track selection strings provided!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Track thinning selection string: " << m_selectionString);
  ATH_CHECK(initializeParser(m_selectionString));

  ATH_CHECK( m_inDetSGKey.initialize (m_streamName) );
  ATH_MSG_INFO("Using " << m_inDetSGKey << "as the source collection for inner detector track particles");

  if (m_thinPixelHitsOnTrack) {
    ATH_MSG_INFO("Pixel states collection as source for thinning: " << m_statesPixSGKey.key());
    ATH_MSG_INFO("Pixel measurements collection as source for thinning: " << m_measurementsPixSGKey.key());
  }
  if (m_thinSCTHitsOnTrack) {
    ATH_MSG_INFO("SCT states collection as source for thinning: " << m_statesSctSGKey.key());
    ATH_MSG_INFO("SCT measurements collection as source for thinning: " << m_measurementsSctSGKey.key());
  }
  if (m_thinTRTHitsOnTrack) {
    ATH_MSG_INFO("TRT states collection as source for thinning: " << m_statesTrtSGKey.key());
    ATH_MSG_INFO("TRT measurements collection as source for thinning: " << m_measurementsTrtSGKey.key());
  }

  ATH_MSG_DEBUG("Initializing measurement ThinningHandleKeys.");
  ATH_CHECK( m_measurementsPixSGKey.initialize (m_streamName, m_thinPixelHitsOnTrack && !m_measurementsPixSGKey.empty()) );
  ATH_CHECK( m_measurementsSctSGKey.initialize (m_streamName, m_thinSCTHitsOnTrack && !m_measurementsSctSGKey.empty()) );
  ATH_CHECK( m_measurementsTrtSGKey.initialize (m_streamName, m_thinTRTHitsOnTrack && !m_measurementsTrtSGKey.empty()) );

  ATH_MSG_DEBUG("Initializing MSOS ThinningHandleKeyArrays.");
  ATH_CHECK( m_statesPixSGKey.initialize (m_streamName, m_thinPixelHitsOnTrack && !m_statesPixSGKey.empty()) );
  ATH_CHECK( m_statesSctSGKey.initialize (m_streamName, m_thinSCTHitsOnTrack && !m_statesSctSGKey.empty()) );
  ATH_CHECK( m_statesTrtSGKey.initialize (m_streamName, m_thinTRTHitsOnTrack && !m_statesTrtSGKey.empty()) );

  ATH_CHECK(m_SCTDetEleCollKey.initialize( !m_SCTDetEleCollKey.key().empty() ));

  return StatusCode::SUCCESS;
}

StatusCode ThinInDetClustersAlg::finalize()
{
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed "<< m_ntot <<" tracks, "<< m_npass<< " had their hits retained ");
  if (m_thinPixelHitsOnTrack) {
    ATH_MSG_INFO("Pixel state objects thinning, Total / Passed (Efficiency): "
                 << m_ntot_pix_states << " / " << m_npass_pix_states
                 << " (" << (m_ntot_pix_states == 0 ? 0 : static_cast<float>(m_npass_pix_states) / m_ntot_pix_states) << ")");
    ATH_MSG_INFO("Total pixel measurements objects from *all* track collections:: " << m_ntot_pix_measurements);
    ATH_MSG_INFO("Pixel measurements objects from this track collection passing selection: " << m_npass_pix_measurements);
  }
  if (m_thinSCTHitsOnTrack) {
    ATH_MSG_INFO("SCT state objects thinning, Total / Passed (Efficiency): "
                 << m_ntot_sct_states << " / " << m_npass_sct_states
                 << " (" << (m_ntot_sct_states == 0 ? 0 : static_cast<float>(m_npass_sct_states) / m_ntot_sct_states) << ")");
    ATH_MSG_INFO("Total SCT measurements objects from *all* track collections:: " << m_ntot_sct_measurements);
    ATH_MSG_INFO("SCT measurements objects from this track collection passing selection: " << m_npass_sct_measurements);
  }
  if (m_thinTRTHitsOnTrack) {
    ATH_MSG_INFO("TRT state objects thinning, Total / Passed (Efficiency): "
                 << m_ntot_trt_states << " / " << m_npass_trt_states
                 << " (" << (m_ntot_trt_states == 0 ? 0 : static_cast<float>(m_npass_trt_states) / m_ntot_trt_states) << ")");
    ATH_MSG_INFO("Total TRT measurements objects from *all* track collections:: " << m_ntot_trt_measurements);
    ATH_MSG_INFO("TRT measurements objects from this track collection passing selection: " << m_npass_trt_measurements);
  }

  ATH_CHECK( finalizeParser() );

  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode ThinInDetClustersAlg::execute(const EventContext& ctx) const
{

  // Retrieve main TrackParticle collection
  SG::ThinningHandle<xAOD::TrackParticleContainer> importedTrackParticles(m_inDetSGKey, ctx);
  ATH_CHECK( importedTrackParticles.isValid() );

  ////////////////////////
  if (m_thinPixelHitsOnTrack && !m_measurementsPixSGKey.empty()) {
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> importedMeasurements(m_measurementsPixSGKey, ctx);
    ATH_CHECK( importedMeasurements.isValid() );
    unsigned int size_measurements = importedMeasurements->size();
    m_ntot_pix_measurements += size_measurements;
  }
  if (m_thinSCTHitsOnTrack && !m_measurementsSctSGKey.empty()) {
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> importedMeasurements(m_measurementsSctSGKey, ctx);
    ATH_CHECK( importedMeasurements.isValid() );
    unsigned int size_measurements = importedMeasurements->size();
    m_ntot_sct_measurements += size_measurements;
  }
  if (m_thinTRTHitsOnTrack && !m_measurementsTrtSGKey.empty()) {
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> importedMeasurements(m_measurementsTrtSGKey, ctx);
    ATH_CHECK( importedMeasurements.isValid() );
    unsigned int size_measurements = importedMeasurements->size();
    m_ntot_trt_measurements += size_measurements;
  }
  ////////////////////////

  // Check the event contains tracks
  unsigned int nTracks = importedTrackParticles->size();
  if (nTracks==0) return StatusCode::SUCCESS;

  // Set up a mask with the same entries as the full TrackParticle collection
  std::vector<bool> mask;
  mask.assign(nTracks,false); // default: don't keep any tracks
  m_ntot += nTracks;

  // Execute the text parser and update the mask
  if (m_parser) {
    std::vector<int> entries =  m_parser->evaluateAsVector();
    unsigned int nEntries = entries.size();
    // check the sizes are compatible
    if (nTracks != nEntries ) {
      ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used ID TrackParticles?");
      return StatusCode::FAILURE;
    }
    // set mask
    for (unsigned int i=0; i<nTracks; ++i) {
      if (entries[i]==1) {
        mask[i]=true; // Set track mask
        m_npass += 1; // Count passing tracks.
      }
    }
  }

  if (m_thinPixelHitsOnTrack) {
    ATH_CHECK( filterTrackHits (
                                TrkState_Pixel,
                                *importedTrackParticles,
                                mask,
                                m_statesPixSGKey,
                                m_measurementsPixSGKey,
                                m_ntot_pix_states,
                                m_npass_pix_states,
                                m_npass_pix_measurements,
                                ctx) );
  }
  if (m_thinSCTHitsOnTrack) {
    ATH_CHECK( filterTrackHits (
                                TrkState_SCT,
                                *importedTrackParticles,
                                mask,
                                m_statesSctSGKey,
                                m_measurementsSctSGKey,
                                m_ntot_sct_states,
                                m_npass_sct_states,
                                m_npass_sct_measurements,
                                ctx) );
  }
  if (m_thinTRTHitsOnTrack) {
    ATH_CHECK( filterTrackHits (
                                TrkState_TRT,
                                *importedTrackParticles,
                                mask,
                                m_statesTrtSGKey,
                                m_measurementsTrtSGKey,
                                m_ntot_trt_states,
                                m_npass_trt_states,
                                m_npass_trt_measurements,
                                ctx) );
  }
  
  return StatusCode::SUCCESS;
}

StatusCode ThinInDetClustersAlg::filterTrackHits
(MeasurementType detTypeToSelect,
 const xAOD::TrackParticleContainer& inputTrackParticles,
 const std::vector<bool>& inputMask,
 const SG::ThinningHandleKey<xAOD::TrackStateValidationContainer>& statesKey,
 const SG::ThinningHandleKey<xAOD::TrackMeasurementValidationContainer>& measurementsKey,
 std::atomic<unsigned int>& ntot_states,
 std::atomic<unsigned int>& npass_states,
 std::atomic<unsigned int>& npass_measurements,
 const EventContext& ctx) const
{
  std::vector<bool> maskStates;
  std::vector<bool> maskMeasurements;

  selectTrackHits (inputTrackParticles, inputMask, detTypeToSelect,
                   maskStates, maskMeasurements);

  auto count = [] (const std::vector<bool>& m)
    { return std::count (m.begin(), m.end(), true); };
  npass_states += count (maskStates);
  npass_measurements += count (maskMeasurements);

  if (!statesKey.empty()) {
    SG::ThinningHandle<xAOD::TrackStateValidationContainer> importedStates(statesKey, ctx);
    ATH_CHECK( importedStates.isValid() );
    unsigned int size_states = importedStates->size();
    if (size_states == 0) {
      ATH_MSG_WARNING("States container is empty: " << statesKey.key());
    }
    else {
      ntot_states += size_states;
      if (maskStates.size() > size_states) {
        ATH_MSG_ERROR("States mask size mismatch: mask size (" << maskStates.size() << ") > number of states (" << size_states << ").");
        return StatusCode::FAILURE;
      }
      maskStates.resize (size_states);
      importedStates.keep (maskStates);
    }
  }

  if (!measurementsKey.empty()) {
    SG::ThinningHandle<xAOD::TrackMeasurementValidationContainer> importedMeasurements(measurementsKey, ctx);
    ATH_CHECK( importedMeasurements.isValid() );
    unsigned int size_measurements = importedMeasurements->size();
    if (size_measurements == 0) {
      ATH_MSG_WARNING("Measurements container is empty: " << measurementsKey.key());
    }
    else {
      if (maskMeasurements.size() > size_measurements) {
        ATH_MSG_ERROR("Measurements mask size mismatch: mask size (" << maskMeasurements.size() << ") > number of measurements (" << size_measurements << ").");
        return StatusCode::FAILURE;
      }
      maskMeasurements.resize (size_measurements);
      importedMeasurements.keep (maskMeasurements);
    }
  }

  return StatusCode::SUCCESS;
}

void ThinInDetClustersAlg::selectTrackHits(const xAOD::TrackParticleContainer& inputTrackParticles,
                                           const std::vector<bool>& inputMask,
                                           MeasurementType detTypeToSelect,
                                           std::vector<bool>& outputStatesMask, std::vector<bool>& outputMeasurementsMask) const
{
  using StatesOnTrack = std::vector<ElementLink<xAOD::TrackStateValidationContainer>>;

  // loop over track particles, consider only the ones pre-selected by the mask
  int trkIndex=-1;
  for (const xAOD::TrackParticle* trkIt : inputTrackParticles) {
    trkIndex++;

    if (not inputMask[trkIndex]) continue;

    // loop over the TrackStateValidation objects, and add them to the outputStatesMask
    static const SG::AuxElement::ConstAccessor< StatesOnTrack > trackStateAcc("Reco_msosLink");
    if( ! trackStateAcc.isAvailable( *trkIt ) ) {
      ATH_MSG_INFO("Cannot find TrackState link from xAOD::TrackParticle. Skipping track.");
      continue;
    }
    const StatesOnTrack& measurementsOnTrack = trackStateAcc(*trkIt);

    for( const ElementLink<xAOD::TrackStateValidationContainer>& trkState_el : measurementsOnTrack) {
      if (not trkState_el.isValid()) {
        ATH_MSG_INFO("Cannot find a valid link to TrackStateValidation object for track index: " << trkIndex);
        continue; //not a valid link
      }
      if ((*trkState_el)->detType() != detTypeToSelect) {
        ATH_MSG_VERBOSE("Discarding TrackState as not of correct type " << detTypeToSelect);
        continue;
      }
      if (trkState_el.index() >= outputStatesMask.size()) {
        outputStatesMask.resize (trkState_el.index()+1);
      }
      outputStatesMask[trkState_el.index()] = true;

      // get the corresponding TrackMeasurementValidation object, if any, and add it to the outputMeasurementsMask
      const ElementLink<xAOD::TrackMeasurementValidationContainer> trkMeasurement_el = (*trkState_el)->trackMeasurementValidationLink();
      if (not trkMeasurement_el.isValid()) {
        ATH_MSG_VERBOSE("Cannot find a valid link to TrackMeasurementValidation object from track state for track index: " << trkIndex
                        << ", trackState index: " << trkState_el.index());
        continue; //not a valid link
      }
      if (*trkMeasurement_el == nullptr) {
        ATH_MSG_VERBOSE("Invalid pointer to TrackMeasurementValidation object from track state for track index: " << trkIndex
                        << ", trackState index: " << trkState_el.index());
        continue; //not linking to a valid object -- is it necessary?
      }
      if (trkMeasurement_el.index() >= outputMeasurementsMask.size()) {
        outputMeasurementsMask.resize (trkMeasurement_el.index()+1);
      }
      outputMeasurementsMask[trkMeasurement_el.index()] = true;
    }
  } // end loop over xAOD::TrackParticle container
}
