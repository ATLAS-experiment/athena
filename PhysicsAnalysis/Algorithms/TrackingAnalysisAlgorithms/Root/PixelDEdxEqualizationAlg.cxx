//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationAlg.h"

namespace CP {

  PixelDEdxEqualizationAlg::PixelDEdxEqualizationAlg( const std::string& name,
                                                      ISvcLocator* svcLoc )
    : EL::AnaReentrantAlgorithm( name, svcLoc ) { }

  StatusCode PixelDEdxEqualizationAlg::initialize() {

    ATH_MSG_DEBUG("Initializing PixelDEdxEqualizationAlg");

    ANA_CHECK ( m_pixelDEdxEqualizationTool.retrieve() );
      
    ATH_CHECK(m_eventInfoKey.initialize());

    ATH_CHECK ( m_trackContainerName.initialize() );

    /// Determine equalization strategy
    if (m_equalizeClusterMeasurements && m_equalizeTrackMeasurements) {
      ATH_MSG_ERROR("Can only equalize the dE/dx measurements at cluster-level OR track-level, not both.");
      return StatusCode::FAILURE;
    }
    else if (m_equalizeClusterMeasurements) {
      ATH_MSG_INFO("Will equalize individual cluster dE/dx measurements and calculate the truncated mean.");
      ATH_MSG_INFO("NB: MC20 does not model radiation damage and is not yet supported.  Will throw error.");
    }
    else if (m_equalizeTrackMeasurements) {
      ATH_MSG_INFO("Will equalize the track-level truncated mean dE/dx from the AOD.");
      ATH_MSG_INFO("NB: Run 3 data is not yet supported.  Will apply SFs from end of Run 2 to Run 3 tracks.");
      ATH_MSG_INFO("NB: MC20 and MC23 are not yet supported. Will throw error.");
    }
    else{
      ATH_MSG_ERROR("Must choose to equalize the dE/dx measurements at cluster-level OR track-level.");
      return StatusCode::FAILURE;
    }

    if (m_tightClusterCleaning) {
      ATH_MSG_ERROR("Tight cluster cleaning requested for dE/dx calculation, but feature not yet supported.");
      return StatusCode::FAILURE;
    }

    /// Initialize decorator keys, independent of equalization strategy.
    /// Most won't be used for track-level equalization strategy since no pixel clusters.
    std::string trackContainer = m_trackContainerName.key();

    std::string varName = "pixeldEdx"; // hardcode to match existing raw truncated mean dE/dx/
    std::string varNameStdDev = "pixeldEdxStdDev"; // hardcode to match existing raw truncated mean dE/dx/
    std::string varNameNUsed = "numberOfUsedHitsdEdx"; // hardcode to match existing raw truncated mean dE/dx/
    std::string varNameIBL = "numberOfIBLOverflowsdEdx"; // hardcode to match existing raw truncated mean dE/dx/
    std::string eqStrat;
    if(m_equalizeClusterMeasurements) {
      eqStrat = "ClusterEqualized";
    }
    else if(m_equalizeTrackMeasurements) {
      eqStrat = "TrackEqualized";
    }
    else { //redundant with check above.
      ATH_MSG_ERROR("Must choose to equalize the dE/dx measurements at cluster-level OR track-level.");
      return StatusCode::FAILURE;
    }

    /// Decorator for equalized truncated mean dE/dx.
    if (m_trackdEdxEqKey.empty()) {
      std::string fullKey = trackContainer + "." + varName + eqStrat;
      m_trackdEdxEqKey = fullKey;
    }
    ANA_CHECK ( m_trackdEdxEqKey.initialize() );

    /// Decorator for equalized truncated standard deviation dE/dx.
    if (m_trackdEdxEqStdDevKey.empty()) {
      std::string fullKey = trackContainer + "." + varNameStdDev + eqStrat;
      m_trackdEdxEqStdDevKey = fullKey;
    }
    ANA_CHECK ( m_trackdEdxEqStdDevKey.initialize() );

    /// Decorator for nUsedHits (divisor of truncated mean).
    if (m_trackdEdxEqNUsedKey.empty()) {
      std::string fullKey = trackContainer + "." + varNameNUsed + eqStrat;
      m_trackdEdxEqNUsedKey = fullKey;
    }
    ANA_CHECK ( m_trackdEdxEqNUsedKey.initialize() );

    /// Decorator for number of good IBL hits in overflow.
    if (m_trackdEdxEqIBLOFKey.empty()) {
      std::string fullKey = trackContainer + "." + varNameIBL + eqStrat;
      m_trackdEdxEqIBLOFKey = fullKey;
    }
    ANA_CHECK ( m_trackdEdxEqIBLOFKey.initialize() );

    ATH_MSG_INFO("Will decorate track container " << m_trackContainerName << " with variable " <<  m_trackdEdxEqKey);

    ANA_CHECK ( m_clusterdEdxKey.initialize() );
    ANA_CHECK ( m_clusterdEdxEqKey.initialize() );

    /// If performing cluster-level equalization, then clusters are present, and these variables can be re-calculated after EQ using xAOD EDM.
    if(m_equalizeClusterMeasurements) {
      ATH_MSG_INFO("Will decorate track container " << m_trackContainerName << " with variable " <<  m_trackdEdxEqStdDevKey);
      ATH_MSG_INFO("Will decorate track container " << m_trackContainerName << " with variable " <<  m_trackdEdxEqNUsedKey);
      ATH_MSG_INFO("Will decorate track container " << m_trackContainerName << " with variable " <<  m_trackdEdxEqIBLOFKey);
      ATH_MSG_INFO("Will decorate PixelClusters with their raw dE/dx using key: " << m_clusterdEdxKey);
      ATH_MSG_INFO("Will decorate PixelClusters with their equalized dE/dx using key: " << m_clusterdEdxEqKey);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode PixelDEdxEqualizationAlg::execute(const EventContext& ctx) const {

    using StatesOnTrack = std::vector<ElementLink<xAOD::TrackStateValidationContainer>>;

    /// Declare decorators here
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float > trackdEdxEqHandle(m_trackdEdxEqKey, ctx);
    /// Following only used if doing cluster-level equalization
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float > trackdEdxEqStdDevHandle(m_trackdEdxEqStdDevKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, unsigned char > trackdEdxEqNUsedHandle(m_trackdEdxEqNUsedKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, unsigned char > trackdEdxEqIBLOFHandle(m_trackdEdxEqIBLOFKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float > clusterdEdxHandle(m_clusterdEdxKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float > clusterdEdxEqHandle(m_clusterdEdxEqKey, ctx);

    /// Get run number for scale factor determination.
    /// For MC, run number indicates MC subcampaign.
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    int runNumber =  eventInfo->runNumber();
    ATH_MSG_DEBUG("The current event has run number: " << runNumber);
    
    /// Get tracks
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_trackContainerName, ctx);
    ATH_CHECK( tracks.isValid() );
    
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
      /// The equalization strategy is determined by the tool properties: EqualizeTrackMeasurements or EqualizeClusterMeasurements.
      ///    These are configured in TrackingAnalysisConfig.py & passed through to the tool.
      /// If the cluster EQ strategy is chosen AND pixel clusters & MSOSs are available, this tool will follow the links from the track to the clusters.
      ///    It will then calculate the cluster dE/dx, apply the equalization SF, and decorate the cluster with the raw & equalized dE/dx.
      ///    It will then calculate the truncated mean (and other metrics) using the equalized cluster measurements.
      
      int allPixelHits = 0; // all pixel hits linked to the track.
      unsigned char nUsedHits = 0; // divisor in truncated mean.
      unsigned char nIBLOverflowHits = 0; // number of IBL hits in overflow.

      /// Get pixel clusters in this simple struct to abstract away the two EDMs.
      std::vector<PixelDEdx::PixelClusterStruct> clusters;

      /// Get the raw track-level truncated mean dE/dx & counters from the AOD.
      float stored_dEdx { 0 };
      unsigned char stored_numberOfUsedHitsdEdx = 99;
      unsigned char stored_numberOfIBLOverflowsdEdx = 99;

      trk->summaryValue(stored_dEdx, xAOD::pixeldEdx);

      static const SG::AuxElement::ConstAccessor<unsigned char> nUsedAcc("numberOfUsedHitsdEdx");
      if (!nUsedAcc.isAvailable(*trk)) {
        ATH_MSG_ERROR("numberOfUsedHitsdEdx auxdata is missing!");
        return StatusCode::FAILURE;
      }
      stored_numberOfUsedHitsdEdx = nUsedAcc(*trk);

      static const SG::AuxElement::ConstAccessor<unsigned char> nIBLOFAcc("numberOfIBLOverflowsdEdx");
      if (!nIBLOFAcc.isAvailable(*trk)) {
        ATH_MSG_ERROR("numberOfIBLOverflowsdEdx auxdata is missing!");
        return StatusCode::FAILURE;
      }
      stored_numberOfIBLOverflowsdEdx = nIBLOFAcc(*trk);

      ////////////////////
      // Track-level EQ //
      ////////////////////

      if(m_equalizeTrackMeasurements) {
        
        /// Get track-level equalization SF
        double SF = m_pixelDEdxEqualizationTool->getTrackdEdxSF(*trk, runNumber);
        if (SF < 0.) {
          ATH_MSG_ERROR("Could not find valid SF for this track.  Exiting!");
          return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG("Found track SF: " << SF);
        
        /// Apply the SF
        float averagedEdxEq = stored_dEdx * SF;

        /// Decorate track
        ATH_MSG_DEBUG("Will decorate  variable " << m_trackdEdxEqKey << " with value " << averagedEdxEq);
        trackdEdxEqHandle(*trk) = averagedEdxEq;
      }

      //////////////////////
      // Cluster-level EQ //
      //////////////////////
      else if(m_equalizeClusterMeasurements) {

        /// Check for track states:
        static const SG::AuxElement::ConstAccessor< StatesOnTrack > trackStateAcc(m_msosLink);
        if( ! trackStateAcc.isAvailable( *trk ) ) {
          ATH_MSG_DEBUG("Requested cluster-level equalization, but cannot find TrackState link from xAOD::TrackParticle.");
          ATH_MSG_DEBUG("Could be missing or thinned away. Skipping track.");
          /// Return an invalid value for the equalized truncated mean dE/dx.
          continue;
        }
        const StatesOnTrack& measurementsOnTrack = trackStateAcc(*trk);

        /// Loop over clusters
        for( const ElementLink<xAOD::TrackStateValidationContainer>& msos : measurementsOnTrack) {
          if (not msos.isValid()) {
            continue; //not a valid link.  Can happen if clusters are thinned away via ThinInDetClustersAlg.
          }
          if ((int) (*msos)->detType() != 1) {
            continue; // not a pixel cluster. See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs.h
          }
          allPixelHits++;
          if ( (*msos)->type()!=0) {
            continue; // not fittable.  See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs.h
          }
      
          /// Get the corresponding TrackMeasurementValidation object (cluster/drift tube)
          const ElementLink<xAOD::TrackMeasurementValidationContainer>& pixclus = (*msos)->trackMeasurementValidationLink();
          if (not pixclus.isValid()) {
            ATH_MSG_DEBUG("Invalid link to cluster.");
            continue;
          }
          if (*pixclus == nullptr) {
            ATH_MSG_DEBUG("pixclus is a nullptr.");
            continue;
          }

          /// Build simple cluster struct
          PixelDEdx::PixelClusterStruct cluster;
          StatusCode sc = getPixelClusterStruct(*pixclus, *msos, cluster);
          if (sc.isFailure()) {
            ATH_MSG_ERROR("Failed to build a PixelClusterStruct due to missing info. Exiting.");
            return StatusCode::FAILURE;
          }

          /// Get raw cluster dE/dx.  Will update cluster.dEdx.
          PixelDEdx::getClusterdEdx(cluster, nIBLOverflowHits, m_tightClusterCleaning);
          
          /// Check that the cluster had a valid dE/dx measurement.
          if(cluster.dEdx > 0.) {

            /// Get cluster SF
            double SF = m_pixelDEdxEqualizationTool->getClusterdEdxSF(cluster, runNumber);
            
            if (SF < 0.) {
              ATH_MSG_ERROR("Could not find valid SF for this cluster.  Exiting!");
              return StatusCode::FAILURE;
            }
            ATH_MSG_DEBUG("Found cluster SF: " << SF);

            /// Valid cluster SF found, so calculate the equalized cluster dE/dx
            cluster.dEdxEq = cluster.dEdx * SF;
            
            /// Push back to vector for truncated mean calculation
            clusters.push_back(cluster);
          }

          /// Decorate pixel cluster on track with raw dE/dx, whether it's a good dE/dx measurement or not.
          /// Will be negative default value if cluster fails the cleaning cuts.
          /// By default, only loose cuts on the local (x,y) and track angle are applied.
          /// If requested, tight cuts are applied.
          ATH_MSG_DEBUG("Will decorate  variable " << m_clusterdEdxKey << " with value " << cluster.dEdx);
          clusterdEdxHandle(**pixclus) = cluster.dEdx;
          ATH_MSG_DEBUG("Will decorate  variable " << m_clusterdEdxEqKey << " with value " << cluster.dEdxEq);
          clusterdEdxEqHandle(**pixclus) = cluster.dEdxEq;

        } // end loop over MSOSs / pixel clusters

        /// Get unequalized dE/dx metrics first
        float averagedEdx = 0;
        float sigmadEdx = 0;
        PixelDEdx::getdEdxMetrics(clusters, averagedEdx, sigmadEdx, nUsedHits);

        /// Sanity check that the recalculated raw dE/dx matches what was calculated during reco and stored as a track summary variable.
        float epsilon = 1e-3;
        if(allPixelHits == 0) {
          ATH_MSG_DEBUG("No pixel clusters found on track, so cannot compare calculated dE/dx with value stored in AOD."
                    << "\nThis can occur when pixel clusters are not saved to the AOD, or if they are thinned.");
        }
        else {
          if ( std::abs(stored_dEdx - averagedEdx) > epsilon ) {
            ATH_MSG_DEBUG("The track dE/dx stored in the AOD as summary variable (" << stored_dEdx
                            << ") does not match the value calculated here (" << averagedEdx << ")!"
                            << "\nThis may be due to the local (x,y) of the cluster migrating from the ESD to xAOD EDM.");
          }
          if ( stored_numberOfUsedHitsdEdx != nUsedHits ) {
            ATH_MSG_DEBUG("The numberOfUsedHitsdEdx stored in the AOD ("<< (int) stored_numberOfUsedHitsdEdx
                          << ") does not match the value calculated here ("<< (int) nUsedHits <<")!"
                            << "\nThis may be due to the local (x,y) of the cluster migrating from the ESD to xAOD EDM.");
          }
          if ( stored_numberOfIBLOverflowsdEdx != nIBLOverflowHits) {
            ATH_MSG_DEBUG("The numberOfIBLOverflowsdEdx stored in the AOD ("<< (int) stored_numberOfIBLOverflowsdEdx
                          << ") does not match the value calculated here ("<< (int) nIBLOverflowHits <<")!"
                            << "\nThis may be due to the local (x,y) of the cluster migrating from the ESD to xAOD EDM.");
          }
        }
        
        /// Now get the cluster equalized dE/dx metrics.
        unsigned char nUsedHitsEq=0; // need separate counter or will double count if calculating both raw and equalized dE/dx
        float averagedEdxEq = 0;
        float sigmadEdxEq = 0;
        PixelDEdx::getdEdxMetrics(clusters, averagedEdxEq, sigmadEdxEq, nUsedHitsEq, true);
        
        /// Sanity check that nUsedHits and nUsedHitsEq are the same.
        if (nUsedHitsEq != nUsedHits) {
          ATH_MSG_DEBUG("The numberOfUsedHitsdEdx calculated for the raw ("<< (int) nUsedHits <<") and equalized ("<< (int) nUsedHitsEq <<") dE/dx differ!"
                        << "\nThis can happen if the equalization changes the order of the clusters and there's an IBL OF hit.");
          /// For example, imagine there are 4 good clusters on track, and one is an IBL overflow hit.
          /// Say it is the third cluster when sorting by increasing dE/dx.
          /// On the third iteration in getdEdxMetrics, you'll notice that you have an IBL OF hit.
          ///  Then since you have an IBL OF hit + 4 hits total + 2 hits used, you'll stop.
          /// Now imagine the equalization makes that IBL OF hit the largest.
          /// The algorithm will never see this hit, and never know there was an IBL OF hit.
          /// After the third cluster, it will stop since 3 of 4 seen.
        }
        
        /// Decorate track with cluster-level equalized dE/dx truncated mean and std dev.
        ATH_MSG_DEBUG("Will decorate  variable " << m_trackdEdxEqKey << " with value " << averagedEdxEq);
        trackdEdxEqHandle(*trk) = averagedEdxEq;
        
        ATH_MSG_DEBUG("Will decorate  variable " << m_trackdEdxEqStdDevKey << " with value " << sigmadEdxEq);
        trackdEdxEqStdDevHandle(*trk) = sigmadEdxEq;
        
        /// Decorate with nUsedHits and nIBLOverflowHits as calculated here with the xAOD EDM.
        /// Can be different from those calculated during reconstruction due to migration across cluster quality cuts.
        /// Particularly the cluster local (x,y), we changes between the ESD and the xAOD...
        ATH_MSG_DEBUG("Will decorate  variable " << m_trackdEdxEqNUsedKey << " with value " << (int) nUsedHitsEq);
        trackdEdxEqNUsedHandle(*trk) = nUsedHitsEq;
        
        ATH_MSG_DEBUG("Will decorate  variable " << m_trackdEdxEqIBLOFKey << " with value " << (int) nIBLOverflowHits);
        trackdEdxEqIBLOFHandle(*trk) = nIBLOverflowHits;
        
      } // end cluster-level equalization if 

    } // end loop over tracks

    return StatusCode::SUCCESS;
  }



  StatusCode PixelDEdxEqualizationAlg::getPixelClusterStruct(
                                                             const xAOD::TrackMeasurementValidation* pixclus,
                                                             const xAOD::TrackStateValidation* msos,
                                                             PixelDEdx::PixelClusterStruct& cluster) const {
    cluster = PixelDEdx::PixelClusterStruct();

    static const SG::AuxElement::ConstAccessor< float > localXAcc("localX");
    if (!localXAcc.isAvailable(*pixclus)) {
      ATH_MSG_ERROR("localX auxdata is missing!");
      return StatusCode::FAILURE;
    }
    cluster.locx = localXAcc(*pixclus);

    static const SG::AuxElement::ConstAccessor<float> localYAcc("localY");
    if (!localYAcc.isAvailable(*pixclus)) {
      ATH_MSG_ERROR("localY auxdata is missing!");
      return StatusCode::FAILURE;
    }
    cluster.locy = localYAcc(*pixclus);

    static const SG::AuxElement::ConstAccessor<int> becAcc("bec");
    if (!becAcc.isAvailable(*pixclus)) {
      ATH_MSG_ERROR("bec auxdata is missing!");
      return StatusCode::FAILURE;
    }
    cluster.bec = becAcc(*pixclus);

    static const SG::AuxElement::ConstAccessor<int> layerAcc("layer");
    if (!layerAcc.isAvailable(*pixclus)) {
      ATH_MSG_ERROR("layer auxdata is missing!");
      return StatusCode::FAILURE;
    }
    cluster.layer = layerAcc(*pixclus);

    static const SG::AuxElement::ConstAccessor<int> etaAcc("eta_module");
    if (!etaAcc.isAvailable(*pixclus)) {
      ATH_MSG_ERROR("eta_module auxdata is missing!");
      return StatusCode::FAILURE;
    }
    cluster.eta_module = etaAcc(*pixclus);

    // Calculate theta and phi
    float msosTheta = msos->localTheta();
    float msosPhi = msos->localPhi();
    float alpha = std::atan(std::hypot(std::tan(msosTheta), std::tan(msosPhi)));
    cluster.cosalpha = std::cos(alpha);

    static const SG::AuxElement::ConstAccessor<float> chargeAcc("charge");
    if (!chargeAcc.isAvailable(*pixclus)) {
      ATH_MSG_ERROR("charge auxdata is missing!");
      return StatusCode::FAILURE;
    }
    cluster.charge = chargeAcc(*pixclus);

    /// Keep track if this is an ibl cluster with overflow
    bool iblOverflow = false;
    if ((cluster.bec==0) and (cluster.layer==0)) { // check if IBL
      constexpr int overflowIBLToT = 16; // see getFEI4OverflowToT() in PixelChargeCalibCondData.h
      std::vector<int> ToTs;
      static const SG::AuxElement::ConstAccessor< std::vector<int> > totAcc("rdo_tot");
      if (!totAcc.isAvailable(*pixclus)) {
        ATH_MSG_ERROR("rdo_tot auxdata is missing!");
        return StatusCode::FAILURE;
      }
      ToTs = totAcc(*pixclus);
      
      for (int pixToT : ToTs) {
        if (pixToT >= overflowIBLToT) {
          //overflow pixel hit -- flag cluster
          iblOverflow = true;
          break; //no need to check other hits of this cluster
        }
      }// end
      cluster.isIBL = true;
      cluster.iblOverflow = iblOverflow;
    }

    return StatusCode::SUCCESS;
  }

} // namespace CP
