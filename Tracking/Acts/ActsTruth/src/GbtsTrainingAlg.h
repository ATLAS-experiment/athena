/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GBTSTRAININGALG_H
#define ACTSTRK_GBTSTRAININGALG_H 1

// ACTS packages 
#include "Acts/Seeding/GbtsLayerConnectionTool.hpp"

// Athena packages
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TruthTrackBuilderTool.h"


namespace ActsTrk{

  class GbtsTrainingAlg final : public AthReentrantAlgorithm {

    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    
    public:

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;
    StatusCode finalize() override;

    private:
    
    ToolHandle<TruthTrackBuilderTool> m_truthTrackBuilderTool{ this, "TruthTrackBuilderTool", "ActsTrk::TruthTrackBuilderTool/TruthTrackBuilderTool", "Tool used to build ordered truth tracks"};
    
    mutable std::mutex m_gbtsTrainingToolMutex;
    mutable std::optional<Acts::Experimental::GbtsLayerConnectionTool> m_layerConnectionTool ATLAS_THREAD_SAFE;
    Acts::Experimental::GbtsLayerConnectionTool::Config m_config;

    std::unique_ptr<const Acts::Logger> m_logger;

    // config settings for connection table
    Gaudi::Property<std::string> m_geometryFile{this, "geometryFile", "","Input GBTS detector-layer geometry description file"};
    Gaudi::Property<std::string> m_outputConnectionTable{this, "outputConnectionTable", "gbts_connection_table.txt","Output GBTS connection table"};
    Gaudi::Property<float> m_zMinTol{this, "zMinTol", 0.2340,"Tolerance subtracted from layer z-min"};
    Gaudi::Property<float> m_zMaxTol{this, "zMaxTol", 0.2340, "Tolerance added to layer z-max"};
    Gaudi::Property<float> m_rMinTol{ this, "rMinTol", 2.5337,"Tolerance subtracted from layer r-min"};
    Gaudi::Property<float> m_rMaxTol{  this, "rMaxTol", 2.5337, "Tolerance added to layer r-max"};
    Gaudi::Property<bool> m_doSymmetrization{ this, "doSymmetrization", false, "Symmetrize GBTS connection table"};
    Gaudi::Property<bool> m_useOldFormatting{ this, "useOldFormatting", false, "Write connection table using old formatting"};
    Gaudi::Property<float> m_probThreshold{ this, "probThreshold", -1.0, "Minimum transition probability; -1 keeps all non-zero transitions"};
    
    /// private member functions

    const Acts::Logger &logger() const { return *m_logger; }

    // applies configuration to algorithms
    void applyConfiguration();

  }; 
    
} // ActsTrk namespace

#endif
