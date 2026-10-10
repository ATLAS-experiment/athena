/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GbtsTrainingAlg.h"

#include <fstream>

#include "ActsInterop/Logger.h"

namespace ActsTrk {

namespace {

// string parser for detector layer information
std::vector<Acts::Experimental::GbtsLayerConnectionTool::LayerDescription>
geometryParser(const std::string& geometryInformation) {

  std::ifstream inStream(geometryInformation);

  if (!inStream) {
    throw std::runtime_error("File does not exist or could not be opened: " +
                             geometryInformation);
  }

  std::vector<Acts::Experimental::GbtsLayerConnectionTool::LayerDescription>
      detectorGeometry{};
  // create geometry objects
  float minR{};
  float maxR{};

  float minZ{};
  float maxZ{};

  std::int32_t gbtsId{};

  while (inStream >> minR >> maxR >> minZ >> maxZ >> gbtsId) {
    detectorGeometry.emplace_back(minR, maxR, minZ, maxZ, gbtsId);
  }

  // the loop above stops on the first extraction that fails, so check why:
  // reaching the end of the file is the only acceptable reason
  if (inStream.bad()) {
    throw std::runtime_error("I/O error while reading geometry file: " +
                             geometryInformation);
  }

  if (!inStream.eof()) {
    throw std::runtime_error("Malformed record in geometry file: " +
                             geometryInformation);
  }

  return detectorGeometry;
}

// function for obtaining old formatting of connection table (will be retired at
// some point soon) but good to keep for comparison for now
bool writeOldConnectionTable(
    const std::string& outputFileLocation,
    const Acts::Experimental::GbtsLayerConnectionTool::LayerIdPairs&
        tempTable) {

  std::ofstream outputFile(outputFileLocation);

  if (!outputFile) {
    return false;
  }

  outputFile << tempTable.size() << " " << 0.2 << "\n";
  for (const auto& layerPair : tempTable) {
    outputFile << 0 << " " << 1 << " " << layerPair.second << " "
               << layerPair.first << " " << 1 << " " << 1 << " " << 100 << "\n"
               << 100 << "\n";
  }

  // close explicitly so that any failure to flush is reported here
  outputFile.close();

  return outputFile.good();
}

// writes the layer transitions out, in either the current or the old format
bool writeConnectionTable(
    const std::string& outputFileLocation,
    const Acts::Experimental::GbtsLayerConnectionTool::LayerIdPairs& layerTable,
    const bool useOldFormatting) {

  if (useOldFormatting) {
    return writeOldConnectionTable(outputFileLocation, layerTable);
  }

  // define output text file
  std::ofstream outputFile(outputFileLocation);

  if (!outputFile) {
    return false;
  }

  outputFile << layerTable.size() << "\n";
  for (const auto& layerPair : layerTable) {

    // swap order as we want outward -> inward ordering
    outputFile << layerPair.second << " " << layerPair.first << "\n";
  }

  // close explicitly so that any failure to flush is reported here
  outputFile.close();

  return outputFile.good();
}
}  // namespace

StatusCode GbtsTrainingAlg::initialize() {

  // retireive truth track builder tool
  ATH_CHECK(m_truthTrackBuilderTool.retrieve());
  // Make the logger And Propagate to ACTS routines
  m_logger = makeActsAthenaLogger(this, "Acts");

  m_config.detectorGeometry = geometryParser(m_geometryFile);
  applyConfiguration();

  m_layerConnectionTool.emplace(m_config,
                                logger().cloneWithSuffix("gbtsLayerTool"));

  return StatusCode::SUCCESS;
}

// outline:
// get the truth particle to reco cluster map,
// "invert" such that we have a map of truth particles with a vector of clusters
// time or distance order (time prefereable) to "recontruct" a truth matched
// track do for both pixel and strip clusters
StatusCode GbtsTrainingAlg::execute(const EventContext& ctx) const {

  ActsTrk::TruthTrackBuilderTool::TruthTracks truthTracks{};

  ATH_CHECK(m_truthTrackBuilderTool->buildTruthTracks(ctx, truthTracks));

  for (auto& [truthParticle, truthClusters] : truthTracks) {
    // find the GBTS layer of each cluster; a cluster without a layer splits
    // the track, so that no transition skips over it
    std::vector<std::vector<Acts::Experimental::GbtsExperimentLayerId>>
        segments(1);
    for (const auto& cluster : truthClusters) {

      const float r =
          std::hypot(cluster.globalPosition.x(), cluster.globalPosition.y());
      const float z = cluster.globalPosition.z();

      const auto gbtsId = findGbtsIdByCoord(r, z);
      if (!gbtsId) {
        ATH_MSG_WARNING("No Gbts Layer for coordinates with r: "
                        << r << " and z: " << z);
        if (!segments.back().empty()) {
          segments.emplace_back();
        }
        continue;
      }

      segments.back().push_back(*gbtsId);
    }

    // add track segments to training algorithm
    {
      std::lock_guard<std::mutex> lock(m_gbtsTrainingToolMutex);
      for (const auto& segment : segments) {
        if (segment.size() < 2) {
          continue;
        }
        m_layerConnectionTool->addTrack(segment);
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode GbtsTrainingAlg::finalize() {

  const auto layerTable = m_layerConnectionTool->createConnectionTable();

  // finally, add transitions to output file (old or new format)
  if (!writeConnectionTable(m_outputConnectionTable, layerTable,
                            m_useOldFormatting)) {

    ATH_MSG_ERROR("Could not write connection table to "
                  << m_outputConnectionTable.value());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

void GbtsTrainingAlg::applyConfiguration() {

  m_config.doSymmetrization = m_doSymmetrization;
  m_config.probThreshold = m_probThreshold;
}

std::optional<Acts::Experimental::GbtsExperimentLayerId>
GbtsTrainingAlg::findGbtsIdByCoord(const float r, const float z) const {
  for (const auto& layer : m_config.detectorGeometry) {
    const float zMin = layer.minZ - m_zMinTol;
    const float zMax = layer.maxZ + m_zMaxTol;
    const float rMin = layer.minR - m_rMinTol;
    const float rMax = layer.maxR + m_rMaxTol;

    if (zMin <= z && z <= zMax && rMin <= r && r <= rMax) {
      return layer.gbtsId;
    }
  }

  return std::nullopt;
}
}  // namespace ActsTrk