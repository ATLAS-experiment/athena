/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GbtsTrainingAlg.h"

#include "ActsInterop/Logger.h"
#include <fstream>


namespace ActsTrk{

  // static string parser for detector layer information
  static std::vector<Acts::Experimental::GbtsLayerConnectionTool::LayerDescription> geometryParser(
  const std::string& geometryInformation) {

    std::ifstream inStream(geometryInformation);

    if (!inStream) {
      throw std::runtime_error("File does not exist or could not be opened");
    }

    std::vector<Acts::Experimental::GbtsLayerConnectionTool::LayerDescription> detectorGeometry{};
    // create geometry objects
    float minR{};
    float maxR{};

    float minZ{};
    float maxZ{};

    std::int32_t gbtsId{};

    while (inStream >> minR >> maxR >> minZ >> maxZ >> gbtsId) {
      detectorGeometry.emplace_back(minR, maxR, minZ, maxZ, gbtsId);
    }
        
    return detectorGeometry;
  }

  // static function for obtaining old formatting of connection table (will be retired at some point soon)
  // but good to keep for comparison for now 
  static void oldStyleFormatting(
  const std::string& outputFileLocation,
  const Acts::Experimental::GbtsLayerConnectionTool::LayerIdPairs& tempTable){

    std::ofstream outputFile(outputFileLocation);

    outputFile << tempTable.size() << " " << 0.2 << "\n";
    for (const auto& layerPair : tempTable) {
        outputFile << 0 << " " << 1 << " " << layerPair.second << " "
                << layerPair.first << " " << 1 << " " << 1 << " " << 100 << "\n"
                << 100 << "\n";
    }
  }

  StatusCode GbtsTrainingAlg::initialize(){

    // retireive truth track builder tool
    ATH_CHECK(m_truthTrackBuilderTool.retrieve());
    // Make the logger And Propagate to ACTS routines
    m_logger = makeActsAthenaLogger(this, "Acts");

    m_config.detectorGeometry = geometryParser(m_geometryFile);
    applyConfiguration();

    m_layerConnectionTool.emplace(m_config, logger().cloneWithSuffix("gbtsLayerTool"));

    return StatusCode::SUCCESS;
  }

  // outline:
  // get the truth particle to reco cluster map,
  // "invert" such that we have a map of truth particles with a vector of clusters
  // time or distance order (time prefereable) to "recontruct" a truth matched track
  // do for both pixel and strip clusters
  StatusCode GbtsTrainingAlg::execute(const EventContext& ctx) const{

    ActsTrk::TruthTrackBuilderTool::TruthTracks truthTracks{};

    ATH_CHECK(m_truthTrackBuilderTool->buildTruthTracks(ctx, truthTracks));

    for (auto& [truthParticle, truthClusters] : truthTracks){
      // add cluster positions to hits to be passed into layer connection tool
      std::vector<Acts::Experimental::GbtsLayerConnectionTool::HitCoordinates> hits{};
      for(const auto &cluster : truthClusters){

          const float r = std::hypot(cluster.globalPosition.x(), cluster.globalPosition.y());
          
          hits.push_back({r, cluster.globalPosition.z()});
      }

      // add track to training algorithm
      {
          std::lock_guard<std::mutex> lock(m_gbtsTrainingToolMutex);
          m_layerConnectionTool->addTrack(hits);
      }
            
    }
        
    return StatusCode::SUCCESS;
  }

  StatusCode GbtsTrainingAlg::finalize(){

    const auto layerTable = m_layerConnectionTool->createConnectionTable(m_outputConnectionTable);

    // finally, add transitions to output file (old or new format)
    if (m_useOldFormatting) {
        
      oldStyleFormatting(m_outputConnectionTable, layerTable);
    } else {

      // define output text file
      std::ofstream outputFile(m_outputConnectionTable);
      outputFile << layerTable.size() << "\n";
      for (const auto& layerPair : layerTable) {

        // swap order as we want outward -> inward ordering
        outputFile << layerPair.second << " " << layerPair.first << "\n";
      }
    }
        
    return StatusCode::SUCCESS;
  }

  void GbtsTrainingAlg::applyConfiguration(){

    m_config.doSymmetrization = m_doSymmetrization;
    m_config.probThreshold = m_probThreshold;
    m_config.rMaxTol = m_rMaxTol;
    m_config.rMinTol = m_rMinTol;
    m_config.zMaxTol = m_zMaxTol;
    m_config.zMinTol = m_zMinTol;

  }    
} // ActsTrk namespace