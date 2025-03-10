//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationAlg.h"

namespace CP {

  PixelDEdxEqualizationAlg::PixelDEdxEqualizationAlg( const std::string& name,
                                                  ISvcLocator* svcLoc )
    : EL::AnaReentrantAlgorithm( name, svcLoc ) {
  }

  StatusCode PixelDEdxEqualizationAlg::initialize() {

    ATH_MSG_DEBUG("Initializing PixelDEdxEqualizationAlg");

    if (m_m_pixeldEdxEqualDecor.empty())
      {
        ANA_MSG_ERROR ("No equalized dEdx  decoration name set");
        return StatusCode::FAILURE;
      }


    ANA_CHECK ( m_pixelToTPIDDualTool.retrieve() );
    ANA_CHECK ( m_pixelToTPIDDualTool->setProperty("EqualizeClusterMeasurements",true) ); // no reason to use this tool otherwise, right?
    ANA_CHECK ( m_pixelToTPIDDualTool->initialize() );
      
    ATH_CHECK( m_inputTrackParticles.initialize() );
    // ATH_CHECK( m_pixeldEdxEqual.initialize() );
    
    /*
    if (m_scaleFactorTreePath.empty()) {
      ATH_MSG_FATAL("No path to the ROOT file containing the scale factor trees  provided!");
      return StatusCode::FAILURE;
    }
    if (m_scaleFactorTreeName.empty()) {
      ATH_MSG_FATAL("No name for scale factor tree provided!");
      return StatusCode::FAILURE;
    }

    // Initialize RDataFrame with scale factors
    // Check if the file exists
    std::string pathdEdxSFs = PathResolverFindCalibFile(m_scaleFactorTreePath); // Looks in CALIBPATH.
    if (pathdEdxSFs.empty()) {
      ATH_MSG_WARNING("Failed to find dedx equalization SF file called: " << m_scaleFactorTreePath);
      return StatusCode::FAILURE;
    }
    else {
      ATH_MSG_INFO("Using dE/dx scale factor file: " << pathdEdxSFs);
    }

    // Verify file is not a zombie
    std::unique_ptr<TFile> file(TFile::Open(pathdEdxSFs.c_str(), "READ"));
    if (!file || file->IsZombie()) {
      ATH_MSG_ERROR("Failed to open ROOT file: " << m_scaleFactorTreePath);
      return StatusCode::FAILURE;
    }
    // Verify TTree exists
    if (!file->FindKey(m_scaleFactorTreeName.value().c_str())) { // Check key only.  Don't load.
      ATH_MSG_ERROR("TTree " << m_scaleFactorTreeName << " not found in file: " << m_scaleFactorTreePath);
      return StatusCode::FAILURE;
    }

    // Create the RDataFrame
    m_df = std::make_unique<ROOT::RDataFrame>(m_scaleFactorTreeName.value(), pathdEdxSFs);
    */
    
    return StatusCode::SUCCESS;
  }

  StatusCode PixelDEdxEqualizationAlg::execute(const EventContext& ctx) const {

    // Increase the event counter
    m_nEventsProcessed.fetch_add(1, std::memory_order_relaxed);

    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_inputTrackParticles, ctx);
    ATH_CHECK( tracks.isValid() );
    
    // Create decoration handles
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> pixeldEdxEqualDecor(m_pixeldEdxEqual, ctx);
    ATH_CHECK( pixeldEdxEqualDecor.isValid() );

    // Increase the track counter
    unsigned int nTracks = tracks->size();
    m_nTracksProcessed.fetch_add(nTracks, std::memory_order_relaxed);
    if (nTracks==0) return StatusCode::SUCCESS; // do this first?  MT safe?

    // Now decorate
    for (const auto* trk : *tracks) {
      
      // 1. Call fuction that decorates the pixel clusters on a track
      //    Follows links to MSOSs and Clusters.
      //    Calculates raw dE/dx from charge, sensor thickness, & path-length through silicon.
      //       TODO: eventually refactor PixelToTPIDTool.cxx (which runs during reco) so not duplicating this logic
      //    Get correct SF and decorates cluster with equalized (and raw) dE/dx.  And maybe error.
      //    Only call if links, clusters, and MSOSs are present.
      // 2. Have a separate function that calculates the truncated mean, or any other metric.
      //    Make it configurable for what it calculates...
      //       TODO: Eventually refactor PixelToTPIDTool.cxx (which runs during reco) so not duplicating this logic.
      
      // Apply decorations
      // if (m_pixeldEdxEqualDecor) {
      float pixeldEdxEqual = -99.0;
      int nUsedHits = -1;
      int nUsedIBLOverflowHits = -1;
      pixeldEdxEqual = m_pixelToTPIDDualTool-->dEdx(*trk, nUsedHits, nUsedIBLOverflowHits);

      // m_pixeldEdxEqualDecor.set (*trk, pixeldEdxEqual, sys)
      // }
    }



    /*  
   /// ReadHandleKeyArray way
    auto readHandles = m_inputTrackParticles.makeHandles(ctx);
    for (auto& readHandle : readHandles) {
      for (const xAOD::TrackParticle* tp : *readHandle) {
        // do something
        }
      }
    }
    */




    return StatusCode::SUCCESS;
  }

} // namespace CP
