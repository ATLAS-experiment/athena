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

  if (m_initialVarInflation.size() != m_trkparam_config.initial_inflation.size()) {
    ATH_MSG_FATAL("initialVarInflation must have " << m_trkparam_config.initial_inflation.size()
                  << " elements, but has " << m_initialVarInflation.size());
    return StatusCode::FAILURE;
  }

  m_trkparam_config.initial_sigma = {
    static_cast<traccc::scalar>(m_sigmaLoc0.value()),
    static_cast<traccc::scalar>(m_sigmaLoc1.value()),
    static_cast<traccc::scalar>(m_sigmaPhi.value()),
    static_cast<traccc::scalar>(m_sigmaTheta.value()),
    static_cast<traccc::scalar>(m_sigmaQOverP.value()),
    static_cast<traccc::scalar>(m_sigmaT0.value())};
  m_trkparam_config.initial_sigma_qopt = static_cast<traccc::scalar>(m_sigmaQOverPt.value());
  m_trkparam_config.initial_sigma_pt_rel = static_cast<traccc::scalar>(m_initialSigmaPtRel.value());
  for (std::size_t i = 0; i < m_trkparam_config.initial_inflation.size(); ++i) {
    m_trkparam_config.initial_inflation[i] = static_cast<traccc::scalar>(m_initialVarInflation[i]);
  }

  ATH_MSG_DEBUG("Initial sigmas (loc0, loc1, phi, theta, q/p, t): "
                << m_sigmaLoc0.value() << ", " << m_sigmaLoc1.value() << ", "
                << m_sigmaPhi.value() << ", " << m_sigmaTheta.value() << ", "
                << m_sigmaQOverP.value() << ", " << m_sigmaT0.value()
                << "; sigmaQOverPt " << m_sigmaQOverPt.value()
                << "; initialSigmaPtRel " << m_initialSigmaPtRel.value());
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
  auto trkparam_alg = m_trkParamAlgProviderTool->getAlgorithm(ctx, m_trkparam_config);

  // ---- 3. Run traccc initial track params estimation ---------------------------------------------
  traccc::bound_track_parameters_collection_types::buffer trkparam_gpu_buffer = (*trkparam_alg)(*m_deviceMagField, *inputTracccMeasurements, *inputTracccSpacePoints, *inputTracccSeeds);

  ATH_MSG_DEBUG("Reconstructed " << trkparam_alg.copy().get_size(trkparam_gpu_buffer) << " track parameters.");

  // ---- 4. Write output traccc track parameters to StoreGate -------------------------
  auto outputTracccTrkParams = SG::makeHandle(m_outputTrkParamKey, ctx);
  ATH_CHECK(outputTracccTrkParams.record(
    std::make_unique<traccc::bound_track_parameters_collection_types::buffer>(
        std::move(trkparam_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote track parameters buffer to '" << m_outputTrkParamKey.key() << "'");

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
