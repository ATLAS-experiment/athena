/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TracccTritonTool.h"

#include <chrono>

TracccTritonTool::TracccTritonTool(const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type, name, parent) {

    declareInterface<ITracccTritonTool>(this);
}

StatusCode TracccTritonTool::initialize() {

    ATH_CHECK(m_TracccTritonTool.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode TracccTritonTool::getTracks(
    std::vector<uint8_t>& cellBytes,
    TracccTritonResult& result
) const {

    // Forward the serialized traccc silicon_cell_collection as a single UINT8
    AthInfer::InputDataMap inputData;
    inputData["CELLS"] = std::make_pair(
        std::vector<int64_t>{static_cast<int64_t>(cellBytes.size())},
        cellBytes);

    // Every output is one serialized traccc EDM collection
    AthInfer::OutputDataMap outputData;
    for (const char* name : {"MEASUREMENTS", "CLUSTERS", "TRACKS", "TRACK_STATES"}) {
        outputData[name] = std::make_pair(
            std::vector<int64_t>{-1}, std::vector<uint8_t>{});
    }

    auto start_inference = std::chrono::high_resolution_clock::now();
    ATH_CHECK(m_TracccTritonTool->inference(inputData, outputData));
    auto end_inference = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> inference_time = end_inference - start_inference;
    ATH_MSG_INFO("Traccc Triton inference time: " << inference_time.count() << " ms");

    auto take = [&outputData](const char* name) {
        return std::move(std::get<std::vector<uint8_t>>(outputData[name].second));
    };
    result.measurements = take("MEASUREMENTS");
    result.clusters = take("CLUSTERS");
    result.tracks = take("TRACKS");
    result.trackStates = take("TRACK_STATES");

    ATH_MSG_DEBUG("Received " << result.measurements.size() << " + "
                  << result.clusters.size() << " + " << result.tracks.size()
                  << " + " << result.trackStates.size()
                  << " bytes of measurements, clusters, tracks and track states");

    return StatusCode::SUCCESS;
}
