/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccTrkParamConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <vecmem/containers/device_vector.hpp>

#include <memory>
#include <stdexcept>

namespace ActsTrk {

StatusCode TracccTrkParamConverterAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing.");

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(m_inputPackedTrkParamsKey.initialize());
    ATH_CHECK(m_inputPixelClustersKey.initialize());
    ATH_CHECK(m_inputStripClustersKey.initialize());
    ATH_CHECK(m_inputMeasToPixelSPKey.initialize());
    ATH_CHECK(m_inputMeasToStripClKey.initialize());

    ATH_CHECK(m_outputTrackParamsKey.initialize());
    ATH_CHECK(m_outputSeedMeasurementsKey.initialize());

    ATH_CHECK(m_trackingGeometrySvc.retrieve());
    // The Detray -> ACTS surface map is built by the ACTS Detray plugin
    // together with the Detray geometry in the TrackingGeometrySvc
    m_detraySurfaceIds = m_trackingGeometrySvc->detraySurfaceIdMap();
    if (m_detraySurfaceIds == nullptr) {
        ATH_MSG_FATAL("No Detray geometry available from " << m_trackingGeometrySvc.name()
                      << ", cannot translate the traccc track parameters to Acts surfaces. "
                      "Set its BuildDetrayGeometry property to True.");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully initialized");
    return StatusCode::SUCCESS;
}

StatusCode TracccTrkParamConverterAlg::execute(const EventContext& ctx) const
{
    // ---- Retrieve HOST RESIDENT clusters and the maps from traccc measurement index ----
    // These are made during the TracccMeasurementConverterAlg
    auto pixel_clusters = SG::makeHandle(m_inputPixelClustersKey, ctx);
    ATH_CHECK(pixel_clusters.isValid());
    auto strip_clusters = SG::makeHandle(m_inputStripClustersKey, ctx);
    ATH_CHECK(strip_clusters.isValid());
    auto pixelMeasMap = SG::makeHandle(m_inputMeasToPixelSPKey, ctx);
    ATH_CHECK(pixelMeasMap.isValid());
    auto stripMeasMap = SG::makeHandle(m_inputMeasToStripClKey, ctx);
    ATH_CHECK(stripMeasMap.isValid());

    // ---- Copy the DEVICE resident packed track parameters to host, in one copy ----
    auto packedTrkParams = SG::makeHandle(m_inputPackedTrkParamsKey, ctx);
    ATH_CHECK(packedTrkParams.isValid());

    auto copy = m_copy->copy(ctx);

    TracccTrkParamBuffer packedTrkParamsBuffer{copy->get_size(*packedTrkParams), m_hostMR->mr()};
    copy->setup(packedTrkParamsBuffer)->ignore();
    (*copy)(*packedTrkParams, packedTrkParamsBuffer)->wait();

    const vecmem::device_vector<const TracccTrkParam> packed_trk_params(packedTrkParamsBuffer);

    const std::size_t nSeeds = packed_trk_params.size();
    ATH_MSG_DEBUG("Read the packed track parameters of " << nSeeds << " seeds from device.");

    // ---- Write the HOST resident containers the CKF reads ----
    auto paramsHandle = SG::makeHandle(m_outputTrackParamsKey, ctx);
    ATH_CHECK(paramsHandle.record(std::make_unique<ActsTrk::BoundTrackParametersContainer>()));
    ActsTrk::BoundTrackParametersContainer* outputParams = paramsHandle.ptr();
    outputParams->reserve(nSeeds);

    auto seedMeasurementsHandle = SG::makeHandle(m_outputSeedMeasurementsKey, ctx);
    ATH_CHECK(seedMeasurementsHandle.record(std::make_unique<ActsTrk::SeedMeasurementsContainer>()));
    ActsTrk::SeedMeasurementsContainer* outputMeasurements = seedMeasurementsHandle.ptr();
    outputMeasurements->reserve(nSeeds, 3 * nSeeds);

    std::vector<const xAOD::UncalibratedMeasurement*> measurements;
    unsigned int nNoParameters = 0;

    for (std::size_t iseed = 0; iseed < nSeeds; ++iseed) {

        // ---- Initial track parameters, a null entry marks degenerate parameters ----
        std::optional<Acts::BoundTrackParameters> actsParams;
        try {
            actsParams = detail::toActsParameters(packed_trk_params.at(iseed), *m_detraySurfaceIds);
        } catch (const std::domain_error& e) {
            ATH_MSG_ERROR("Seed " << iseed << ": " << e.what());
            return StatusCode::FAILURE;
        }
        if (actsParams) {
            outputParams->push_back(std::make_unique<Acts::BoundTrackParameters>(std::move(*actsParams)));
        } else {
            ATH_MSG_DEBUG("Seed " << iseed << ": no valid track parameters (degenerate or non-finite estimate)");
            outputParams->push_back(std::unique_ptr<Acts::BoundTrackParameters>());
            ++nNoParameters;
        }

        // ---- Measurements of the seed, all used for the duplicate detection ----
        // Pixel seeds only: one measurement per space point (see TracccTrkParam)
        measurements.clear();
        for (const unsigned int measIndex : packed_trk_params.at(iseed).measurementIndices) {
            const xAOD::UncalibratedMeasurement* measurement = detail::findMeasurement(
                measIndex, *pixelMeasMap, *stripMeasMap, *pixel_clusters, *strip_clusters);
            if (measurement == nullptr) {
                ATH_MSG_ERROR("Seed " << iseed << ": traccc measurement " << measIndex
                              << " not found in " << m_inputMeasToPixelSPKey.key()
                              << " or " << m_inputMeasToStripClKey.key());
                return StatusCode::FAILURE;
            }
            measurements.push_back(measurement);
        }
        outputMeasurements->push_back(measurements);
    }

    ATH_MSG_DEBUG("Converted " << nSeeds << " seeds to " << m_outputTrackParamsKey.key()
                  << " and " << m_outputSeedMeasurementsKey.key()
                  << ", " << nNoParameters << " without track parameters");

    m_nSeeds += nSeeds;
    m_nNoParameters += nNoParameters;

    return StatusCode::SUCCESS;
}

StatusCode TracccTrkParamConverterAlg::finalize()
{
    ATH_MSG_DEBUG("Finalizing.");

    ATH_MSG_INFO("Converted " << m_nSeeds << " seeds, " << m_nNoParameters << " without valid track parameters (not given to the CKF)");

    ATH_MSG_DEBUG("Successfully finalized");
    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
