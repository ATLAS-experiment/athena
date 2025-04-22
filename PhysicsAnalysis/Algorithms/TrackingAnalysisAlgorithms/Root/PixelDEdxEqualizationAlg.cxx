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

    ANA_CHECK ( m_trackContainerName.initialize () );

    ANA_CHECK ( m_pixelToTPIDDualTool.retrieve() );
      
    ATH_CHECK ( m_trackContainerName.initialize() );

    if ( m_dEdxEqVarName.empty() ) {
      ATH_MSG_FATAL("No variable name provided for the equalized dE/dx truncated mean!");
      return StatusCode::FAILURE;
    }

    std::string trackContainer = m_trackContainerName.key();
    std::string dEdxEqKey = Form("%s.%s", trackContainer.c_str(), m_dEdxEqVarName.value().c_str());
    m_dEdxEqKey = SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>{
      this, "dEdxEqName", dEdxEqKey, "SG key for the equalized pixel dE/dx attribute"};
    
    ANA_CHECK ( m_dEdxEqKey.initialize() );
    ATH_MSG_INFO("Will decorate track container " << m_trackContainerName << " with variable " << m_dEdxEqKey);
    
    return StatusCode::SUCCESS;
  }

#ifdef XAOD_STANDALONE
  StatusCode PixelDEdxEqualizationAlg::execute(const EventContext& ctx) const {

    // Increase the event counter
    m_nEventsProcessed.fetch_add(1, std::memory_order_relaxed);

    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_trackContainerName, ctx);
    ATH_CHECK( tracks.isValid() );
    
    // Increase the track counter
    unsigned int nTracks = tracks->size();
    m_nTracksProcessed.fetch_add(nTracks, std::memory_order_relaxed);
    if (nTracks==0) return StatusCode::SUCCESS; // do this first?  MT safe?

    // Now decorate
    for (const auto* trk : *tracks) {
      
      /// Apply dE/dx equalization scale factors and recalculate the dE/dx truncated mean.
      ///    This is to account for:
      ///        Radiation damage (worsens charge collection eff)
      ///        Conditions changes (bias voltage, threshold, feedback current).
      ///    These SFs are calculated using IDTIDE data from every run.
      /// During reconstruction, the dE/dx is calculated for each pixel cluster, then the truncated mean is calculated.
      ///    Only this truncated mean is stored in the AOD (as a track summary variable).
      /// For the nominal AODs, one can apply a run-specific equalization scale factor to the raw truncated mean.
      ///    These SFs are also binned in track eta and IBL overflow status. 
      /// However, the charge collection eff. in each layer of the pixel detector is degrading at a different rate.
      ///    This motivates run- and module-specific equalization scale factors.
      ///    To use these, custom datasets with pixel clusters and MSOSs are required.
      /// If pixel clusters & MSOSs are available, this tool will follow the links from the track to the clusters.
      ///    It will then calculate the cluster dE/dx, apply the equalization SF, and decorate the cluster with the equalized dE/dx.
      ///    It will also calculate the truncated mean using the equalized cluster measurements.
      /// If pixel clusters & MSOSs are not available (e.g. in nominal AODs), the simple run-specific SFs will be applied the the stored truncated mean instead.
      
      float pixeldEdxEqual = -99.0;
      int nUsedHits = -1;
      int nUsedIBLOverflowHits = -1;
      pixeldEdxEqual = m_pixelToTPIDDualTool->dEdx(*trk, nUsedHits, nUsedIBLOverflowHits); // returns raw or equalized based on 'EqualizeClusterMeasurements' boolean property

      /// As a sanity check, can confirm that nUsedHits & nUsedIBLOverflowHits match what was calculated during reconstruction.
      /// NB: these actually can be different since the dE/dx calculated during reco uses the ESD EDM.
      ///    There might be some migration between ESD to xAOD, particularly in the local (x,y) position of the cluster.
      ///       Perhaps due to a refitting?
      ///    If a cluster is too close to a sensor edge, it is ignored in the calculation.
      ///    So some clusters included in the truncated mean during reco (ESD) may not be included here (xAOD).  And vice versa. 

      ATH_MSG_INFO("Will decorate  variable " << m_dEdxEqKey << " with value " << pixeldEdxEqual);
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, float > dEdxEqHandle(m_dEdxEqKey, ctx);
      dEdxEqHandle(*trk) = pixeldEdxEqual;

    }

    return StatusCode::SUCCESS;
  }
#endif

} // namespace CP
