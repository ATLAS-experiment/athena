/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceTrkParamEstimationAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// traccc EDM
#include "traccc/edm/track_parameters.hpp"
#include "traccc/edm/seed_collection.hpp"

namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceTrkParamEstimationAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_trkParamAlgProviderTool.retrieve());
  ATH_CHECK(m_inputSpacePointsKey.initialize());
  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_inputSeedsKey.initialize());
  ATH_CHECK(m_outputTrkParamKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_deviceMagField, m_inputMagFieldKey.value()));

  ATH_CHECK(configureTrkParamEstimation());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceTrkParamEstimationAlg::configureTrkParamEstimation()
{

  ATH_MSG_INFO("Setting up configs");
  // leaving this here in case we want to configure initial trk param errors
  return StatusCode::SUCCESS;

}

StatusCode DeviceTrkParamEstimationAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device track parameter estimation.");

  // ---- 1. Read traccc input from StoreGate --------------------------------
  auto inputTracccMeasurements = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(inputTracccMeasurements.isValid());
  ATH_MSG_DEBUG("Read traccc measurements from '"
                         << inputTracccMeasurements.key() << "'");
  
  auto inputTracccSpacePoints = SG::makeHandle(m_inputSpacePointsKey, ctx);
  ATH_CHECK(inputTracccSpacePoints.isValid());
  ATH_MSG_DEBUG("Read traccc space points from '"
                         << inputTracccSpacePoints.key() << "'");  

  auto inputTracccSeeds = SG::makeHandle(m_inputSeedsKey, ctx);
  ATH_CHECK(inputTracccSeeds.isValid());
  ATH_MSG_DEBUG("Read traccc seeds from '"
                         << inputTracccSeeds.key() << "'");  

  // ---- 2. Get traccc track params estimation alg ---------------------------------------------
  auto trkparam_pair = m_trkParamAlgProviderTool->getTrkParamAlgorithm(ctx, m_trkparam_config);
  std::shared_ptr<const traccc::device::seed_parameter_estimation_algorithm> trkparam_alg = trkparam_pair.second;
  
  // ---- 3. Run traccc initial track params estimation ---------------------------------------------
  traccc::bound_track_parameters_collection_types::buffer trkparam_gpu_buffer = (*trkparam_alg)(*m_deviceMagField, *inputTracccMeasurements, *inputTracccSpacePoints, *inputTracccSeeds);

  ATH_MSG_DEBUG("Reconstructed " << (trkparam_pair.first)->get_size(trkparam_gpu_buffer) << " track parameters.");

  // ---- 4. Write output traccc track parameters to StoreGate -------------------------
  auto outputTracccTrkParams = SG::makeHandle(m_outputTrkParamKey, ctx);
  ATH_CHECK(outputTracccTrkParams.record(
    std::make_unique<traccc::bound_track_parameters_collection_types::buffer>(
        std::move(trkparam_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote track parameters buffer to '" << m_outputTrkParamKey.key() << "'");

  return StatusCode::SUCCESS;
}



} // namespace ActsTrk