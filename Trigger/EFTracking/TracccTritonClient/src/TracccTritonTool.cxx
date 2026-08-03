/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TracccTritonTool.h"

#include "CxxUtils/StringUtils.h"

// Framework include(s).
#include <cmath>

#include <fstream>
#include <chrono>

#include "PathResolver/PathResolver.h"

TracccTritonTool::TracccTritonTool(const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type, name, parent) {

    declareInterface<ITracccTritonTool>(this);
}

StatusCode TracccTritonTool::initialize() {

    ATH_CHECK(m_TracccTritonTool.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode TracccTritonTool::getTracks(
    std::vector<TracccCell>& cells,
    std::vector<TracccTrackParameters>& TracccTrackParams,
    std::vector<LocalMeasurementInfoInTracks>& TracccMeasurementInfoInTracks
) const {

    auto start_prep = std::chrono::high_resolution_clock::now();
    int numCells = cells.size();

    std::vector<int64_t> CellPositions;
    std::vector<float> CellProperties;

    for (const auto& cell : cells)
    {
        CellPositions.push_back(cell.geometry_id);
        CellPositions.push_back(cell.measurement_id);
        CellPositions.push_back(cell.channel0);
        CellPositions.push_back(cell.channel1);

        CellProperties.push_back(cell.timestamp);
        CellProperties.push_back(cell.value);
    }

    if (m_saveEventsToCSV && m_eventCounter.load() < m_maxEventsToSave) {
        // header: geometry_id, measurement_id, channel0, channel1, timestamp, value
        std::string csv_filename = "events/event" +
            std::to_string(m_eventCounter.fetch_add(1)) + "-cells.csv";
        std::ofstream csv_file(csv_filename);
        if (!csv_file.is_open()) {
            ATH_MSG_ERROR("Failed to open CSV file: " << csv_filename);
            return StatusCode::FAILURE;
        }
        csv_file << "geometry_id,measurement_id,channel0,channel1,timestamp,value\n";
        for (const auto& cell : cells) {
            csv_file << cell.geometry_id << ","
                    << cell.measurement_id << ","
                    << cell.channel0 << ","
                    << cell.channel1 << ","
                    << cell.timestamp << ","
                    << cell.value << "\n";
        }
        csv_file.close();
    }

    AthInfer::InputDataMap inputData;
    inputData["CELL_POSITIONS"] = std::make_pair(
        std::vector<int64_t>{numCells, 4}, std::move(CellPositions));
    inputData["CELL_PROPERTIES"] = std::make_pair(
        std::vector<int64_t>{numCells, 2}, std::move(CellProperties));

    AthInfer::OutputDataMap outputData;
    outputData["TRK_PARAMS"] = std::make_pair(
        std::vector<int64_t>{-1, 8}, std::vector<float>{});
    outputData["MEASUREMENTS"] = std::make_pair(
        std::vector<int64_t>{-1, 6}, std::vector<float>{});
    outputData["COVARIANCES"] = std::make_pair(
        std::vector<int64_t>{-1, 25}, std::vector<float>{});
    outputData["GEOMETRY_IDS"] = std::make_pair(
        std::vector<int64_t>{-1, 1}, std::vector<int64_t>{});

    auto end_prep = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> prep_time = end_prep - start_prep;
    ATH_MSG_INFO("Traccc Triton input preparation time: " << prep_time.count() << " ms");

    auto start_inference = std::chrono::high_resolution_clock::now();
    ATH_CHECK(m_TracccTritonTool->inference(inputData, outputData));
    auto end_inference = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> inference_time = end_inference - start_inference;
    ATH_MSG_INFO("Traccc Triton inference time: " << inference_time.count() << " ms");

    auto start_parse = std::chrono::high_resolution_clock::now();

    // Parse outputs
    auto& trkParamsVec = std::get<std::vector<float>>(outputData["TRK_PARAMS"].second);
    auto& measurementsVec = std::get<std::vector<float>>(outputData["MEASUREMENTS"].second);
    auto& covariancesVec = std::get<std::vector<float>>(outputData["COVARIANCES"].second);
    auto& outputGeometryIds = std::get<std::vector<int64_t>>(outputData["GEOMETRY_IDS"].second);

    if (trkParamsVec.empty()) {
        ATH_MSG_DEBUG("No tracks found in the event.");
        return StatusCode::SUCCESS;
    }

    TracccTrackParams.clear();
    TracccMeasurementInfoInTracks.clear();

    int numTrkFeatures = 8;
    int numMeasurementFeatures = 6;

    const size_t nMeasEntries = measurementsVec.size() / numMeasurementFeatures;
    const size_t nCovEntries  = covariancesVec.size() / 25;

    LocalMeasurementInfoInTracks measurement;

    size_t track = 0;
    size_t meas_pos = 0;
    size_t cov_pos  = 0;
    for (size_t geo_idx = 0; geo_idx < outputGeometryIds.size(); ++geo_idx)
    {
        // tracks are flattened by a zero
        if (outputGeometryIds.at(geo_idx) == 0)
        {
            TracccMeasurementInfoInTracks.push_back(measurement);

            measurement.local_x.clear();
            measurement.local_y.clear();
            measurement.phi.clear();
            measurement.theta.clear();
            measurement.qop.clear();
            measurement.time.clear();
            measurement.covariances.clear();
            measurement.athena_id.clear();

            TracccTrackParameters params;
            params.chi2 = trkParamsVec.at(track * numTrkFeatures + 0);
            params.ndf = trkParamsVec.at(track * numTrkFeatures + 1);
            params.l0 = trkParamsVec.at(track * numTrkFeatures + 2);
            params.l1 = trkParamsVec.at(track * numTrkFeatures + 3);
            params.phi = trkParamsVec.at(track * numTrkFeatures + 4);
            params.theta = trkParamsVec.at(track * numTrkFeatures + 5);
            params.qop = trkParamsVec.at(track * numTrkFeatures + 6);
            params.time = trkParamsVec.at(track * numTrkFeatures + 7);

            TracccTrackParams.push_back(params);
            track++;

            continue;
        }

        if (meas_pos >= nMeasEntries || cov_pos >= nCovEntries) {
            ATH_MSG_ERROR("Out-of-range while parsing outputs: meas_pos="
                          << meas_pos << "/" << nMeasEntries
                          << " cov_pos=" << cov_pos << "/" << nCovEntries
                          << " geo_idx=" << geo_idx);
            break;
        }

        measurement.local_x.push_back(measurementsVec.at(meas_pos * numMeasurementFeatures + 0));
        measurement.local_y.push_back(measurementsVec.at(meas_pos * numMeasurementFeatures + 1));
        measurement.phi.push_back(measurementsVec.at(meas_pos * numMeasurementFeatures + 2));
        measurement.theta.push_back(measurementsVec.at(meas_pos * numMeasurementFeatures + 3));
        measurement.qop.push_back(measurementsVec.at(meas_pos * numMeasurementFeatures + 4));
        measurement.time.push_back(measurementsVec.at(meas_pos * numMeasurementFeatures + 5));

        for (size_t cov_idx = 0; cov_idx < 25; ++cov_idx)
        {
            measurement.covariances.push_back(covariancesVec.at(cov_pos * 25 + cov_idx));
        }

        measurement.athena_id.push_back(outputGeometryIds.at(geo_idx));

        meas_pos++;
        cov_pos++;
    }

    // Push the last track (no trailing separator in GEOMETRY_IDS)
    if (!measurement.athena_id.empty())
    {
        TracccMeasurementInfoInTracks.push_back(measurement);

        TracccTrackParameters params;
        params.chi2 = trkParamsVec.at(track * numTrkFeatures + 0);
        params.ndf = trkParamsVec.at(track * numTrkFeatures + 1);
        params.l0 = trkParamsVec.at(track * numTrkFeatures + 2);
        params.l1 = trkParamsVec.at(track * numTrkFeatures + 3);
        params.phi = trkParamsVec.at(track * numTrkFeatures + 4);
        params.theta = trkParamsVec.at(track * numTrkFeatures + 5);
        params.qop = trkParamsVec.at(track * numTrkFeatures + 6);
        params.time = trkParamsVec.at(track * numTrkFeatures + 7);
        TracccTrackParams.push_back(params);
    }

    ATH_MSG_INFO("Number of tracks found: " << TracccTrackParams.size());

    if (TracccTrackParams.size() != TracccMeasurementInfoInTracks.size()) {
        ATH_MSG_WARNING("Mismatch: tracks=" << TracccTrackParams.size()
                        << " measurements=" << TracccMeasurementInfoInTracks.size());
    }

    int n_tracks_to_print = std::min(3, (int)TracccTrackParams.size());
    for (int i = 0; i < n_tracks_to_print; ++i) {
        ATH_MSG_DEBUG("Track " << i << " parameters: "
                        << "chi2=" << TracccTrackParams[i].chi2
                        << ", ndf=" << TracccTrackParams[i].ndf
                        << ", l0=" << TracccTrackParams[i].l0
                        << ", l1=" << TracccTrackParams[i].l1
                        << ", phi=" << TracccTrackParams[i].phi
                        << ", theta=" << TracccTrackParams[i].theta
                        << ", qop=" << TracccTrackParams[i].qop
                        << ", time=" << TracccTrackParams[i].time);

        if (i < (int)TracccMeasurementInfoInTracks.size()) {
            const auto& measurements = TracccMeasurementInfoInTracks[i];
            for (size_t j = 0; j < measurements.athena_id.size(); ++j) {
                ATH_MSG_DEBUG("  Measurement " << j << ": "
                                << "local_x=" << measurements.local_x[j]
                                << ", local_y=" << measurements.local_y[j]
                                << ", phi=" << measurements.phi[j]
                                << ", theta=" << measurements.theta[j]
                                << ", qop=" << measurements.qop[j]
                                << ", time=" << measurements.time[j]
                                << ", athena_id=" << measurements.athena_id[j]);

                // Print full covariance matrix (assumes 5x5 = 25 elements per measurement)
                const size_t cov_per_meas = 25;
                size_t cov_start = j * cov_per_meas;
                if (measurements.covariances.size() >= cov_start + cov_per_meas) {
                    std::ostringstream oss;
                    oss << "    Covariance matrix:";
                    for (size_t r = 0; r < 5; ++r) {
                        oss << "\n      [ ";
                        for (size_t c = 0; c < 5; ++c) {
                            oss << measurements.covariances[cov_start + r * 5 + c] << " ";
                        }
                        oss << "]";
                    }
                    ATH_MSG_DEBUG(oss.str());
                } else {
                    ATH_MSG_WARNING("    Covariance data unavailable for measurement " << j);
                }
            }
        }
    }

    auto end_parse = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> parse_time = end_parse - start_parse;
    ATH_MSG_INFO("Traccc Triton output parsing time: " << parse_time.count() << " ms");

    return StatusCode::SUCCESS;
}
