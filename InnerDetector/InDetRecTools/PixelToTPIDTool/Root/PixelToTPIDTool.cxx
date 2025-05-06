#include "PixelToTPIDTool/PixelToTPIDTool.h"

namespace {

  // Some functions

  // Accessors

} // namepsace

namespace CP {
  
  PixelToTPIDTool::PixelToTPIDTool(const std::string& tool_name) : asg::AsgTool(tool_name) {    

    float energyPair = 3.68e-6; // Energy in MeV to create an electron-hole pair in silicon
    float sidensity = 2.329; // silicon density in g cm^-3
    m_conversionfactor=energyPair/sidensity;

    /// Sensor thicknesses (in cm).
    m_Pixel_sensorthickness=.025; // 250 microns Pixel Planars
    m_IBL_3D_sensorthickness=.023; // 230 microns IBL 3D
    m_IBL_PLANAR_sensorthickness=.020; // 200 microns IBL Planars

#ifndef XAOD_STANDALONE
    m_pixelid = nullptr; // not used in XAOD_STANDALONE.
#endif
  }

  PixelToTPIDTool::~PixelToTPIDTool() = default;

  StatusCode PixelToTPIDTool::initialize() {
    ATH_MSG_INFO("Initializing PixelToTPIDTool");

    /// Common to both EDMs ///
    if (m_equalizeClusterMeasurements && m_equalizeTrackMeasurements) {
      ATH_MSG_ERROR("Can only equalize the dE/dx measurements at cluster-level OR track-level, not both.");
      return StatusCode::FAILURE;
    }
    else if (m_equalizeClusterMeasurements) {
      ATH_MSG_INFO("Will equalize cluster dE/dx measurements before calculating truncated mean.");
    }
    else if (m_equalizeTrackMeasurements) {
      ATH_MSG_INFO("Will equalize track-level dE/dx measurements after calculating truncated mean.");
    }
    else{
      ATH_MSG_INFO("Will NOT equalize cluster dE/dx measurements before calculating truncated mean.");
    }

    if (m_extraClusterCleaning) {
      ATH_MSG_WARNING("Extra cluster cleaning requested for dE/dx calculation, but feature not yet supported.");
    }


    /// xAOD EDM ///
#ifdef XAOD_STANDALONE

    /// For determining data vs MC, as well as run number.
    ATH_CHECK(m_eventInfo.initialize());

    /// Set up scale factors. In XAOD_STANDALONE, read SFs from trees stored on CVMFS
    if(m_equalizeClusterMeasurements || m_equalizeTrackMeasurements) {
      if (m_sfLocalFileName != "") {
        ATH_MSG_WARNING("!! SETTING UP WITH USER SPECIFIED INPUT LOCATION \"" << m_sfLocalFileName << "\"!! FOR DEVELOPMENT USE ONLY !! ");
      }
      ATH_CHECK(initSFsFromTrees());
    }

    /// Initialize decorator keys
    ANA_CHECK ( m_clusterdEdxKey.initialize() );
    ATH_MSG_INFO("Will decorate PixelCluster container with variable " << m_clusterdEdxKey);
    ANA_CHECK ( m_clusterdEdxEqKey.initialize() ); // only use if m_equalizeClusterMeasurements == true.
    ATH_MSG_INFO("Will decorate PixelCluster container with variable " << m_clusterdEdxEqKey << " (if equalization is enabled).");
#endif


    /// ESD EDM ///
#ifndef XAOD_STANDALONE
    
    /// When NOT in XAOD_STANDALONE, will read SFs from conditions database instead.
    /// See QT from Rebecca Hicks (ATLIDTRKCP-579).

    ATH_CHECK(detStore()->retrieve(m_pixelid,"PixelID"));
    
    if (!m_IBLParameterSvc.empty()) {
      if (m_IBLParameterSvc.retrieve().isFailure()) {
        ATH_MSG_FATAL("Could not retrieve IBLParameterSvc"); 
        return StatusCode::FAILURE; 
      } else
        ATH_MSG_INFO("Retrieved service " << m_IBLParameterSvc); 

      ATH_CHECK(m_moduleDataKey.initialize());

    }
#endif

    return StatusCode::SUCCESS;
  }

  //////////////////
  //////////////////
  //////////////////

  /// When in XAOD_STANDALONE, initialize SFs from ROOT TTrees on CVMFS.
  /// Read into an RDataFrame.  Will filter to get SFs from closest run  in dEdx().
#ifdef XAOD_STANDALONE
  StatusCode PixelToTPIDTool::initSFsFromTrees()  {
    
    ATH_MSG_INFO("Initializing dE/dx equalization scale factor trees");

    /// Get path to SF trees.
    std::string filename;
    
    if (!m_sfLocalFileName.empty()) { // override official version in ASG calibration area.
      filename = m_sfLocalFileName;
    }
    else {
      filename = PathResolverFindCalibFile( m_sfFileName );
    }

    if (filename.empty()) {
      ATH_MSG_ERROR("Could not find file: " << filename);
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Found scale factor tree file: " << filename);

    /// Get file
    m_file = std::make_shared<TFile>(filename.c_str(), "READ");
    if (!m_file || m_file->IsZombie()) {
      ATH_MSG_ERROR("Failed to open file: " << filename);
      return StatusCode::FAILURE;
    }

    /// Get dataframe
    /// Already checked in initialize that m_equalizeClusterMeasurements or m_equalizeTrackMeasurements is true, but not both.
    if(m_equalizeClusterMeasurements) {
      m_df = std::make_shared<ROOT::RDataFrame>(m_clusterSFTreeName.value().c_str(), m_file.get());
    }
    else if(m_equalizeTrackMeasurements) {
      m_df = std::make_shared<ROOT::RDataFrame>(m_trackSFTreeName.value().c_str(), m_file.get());
    }
    else { // should not get here
      ATH_MSG_ERROR("Called initSFsFromTrees() but did not request dE/dx equalization at cluster or track level.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("RDataFrame successfully initialized.");

    return StatusCode::SUCCESS;
  }
#endif

  //////////////////
  //////////////////
  //////////////////

#ifndef XAOD_STANDALONE
  /// This is the version ran during reconstruction on the ESD EDM.
  /// Will return the dE/dx, and will update nUsedHits (the divisor in the truncated mean) and nUsedIBLOverflowHits.
  /// Whether this is the raw or equalized dE/dx will be determined by the tool properties.
  float PixelToTPIDTool::dEdx(const EventContext& ctx,
                                  const Trk::Track& track,
                                  int& nUsedHits,
                                  int& nUsedIBLOverflowHits) const
  {

    /// Total number of good pixel hits considered in truncated mean calc.    
    /// Clusters will be subjected to various cuts.
    int goodPixelhits = 0; 

    /// passed by ref, so will update here.  
    nUsedHits=0; // divisor in the truncated mean calculation.
    nUsedIBLOverflowHits=0; // number of IBL hits in overflow.
    
    /// Get pixel clusters in this simple struct to abstract away the two EDMs.
    std::vector<PixelCluster> clusters;

    ////second value keeps track if the cluster is in IBL and has at least an overflow hit
    std::multimap<float,int> dEdxMap;

    // Check for track states:
    const Trk::TrackStates* recoTrackStates = track.trackStateOnSurfaces();
    if (recoTrackStates) {
      Trk::TrackStates::const_iterator tsosIter    = recoTrackStates->begin();
      Trk::TrackStates::const_iterator tsosIterEnd = recoTrackStates->end();

      // Loop over track states on surfaces (i.e. generalized hits):
      for (; tsosIter != tsosIterEnd; ++tsosIter) {
        const Trk::MeasurementBase *measurement = (*tsosIter)->measurementOnTrack();
        if (measurement && !(*tsosIter)->type(Trk::TrackStateOnSurface::Outlier)) {
          if (!(*tsosIter)->trackParameters()) {
            msg(MSG::WARNING) << "No track parameters available for a state of type measurement, returning -1" << endmsg;
            msg(MSG::WARNING) << "Don't run this tool on slimmed tracks!" << endmsg;
            return -1;
          }

          const InDet::PixelClusterOnTrack* pixclus = nullptr;
          if (measurement->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
            const Trk::RIO_OnTrack* tmpRio = static_cast<const Trk::RIO_OnTrack*>(measurement);
            if (tmpRio->rioType(Trk::RIO_OnTrackType::PixelCluster)) {
              pixclus = static_cast<const InDet::PixelClusterOnTrack*>(tmpRio);
            }
          }
          if (pixclus) {

            /// Build PixelCluster to abstract away the EDMs.
            PixelCluster cluster;            
            cluster.locx=pixclus->localParameters()[Trk::locX];
            cluster.locy=pixclus->localParameters()[Trk::locY];
            cluster.bec=m_pixelid->barrel_ec(pixclus->identify());
            cluster.layer=m_pixelid->layer_disk(pixclus->identify());
            cluster.eta_module=m_pixelid->eta_module(pixclus->identify());//check eta module to select thickness
            
            float dotProd = (*tsosIter)->trackParameters()->momentum().dot( (*tsosIter)->trackParameters()->associatedSurface().normal() );
            cluster.cosalpha = fabs(dotProd / (*tsosIter)->trackParameters()->momentum().mag());
            //cluster.charge = pixclus->prepRawData()->totalCharge()*cluster.cosalpha; // NB: multiplying by cosalpha!
            cluster.charge = pixclus->prepRawData()->totalCharge();

            /// keep track if this is an ibl cluster with overflow
            int iblOverflow=0;
            if ((m_IBLParameterSvc->containsIBL()) and (cluster.bec==0) and (cluster.layer==0)) { // check if IBL
              
              //loop over ToT and check if anyone is overflow (ToT==14) check for IBL cluster overflow
              int overflowIBLToT = SG::ReadCondHandle<PixelChargeCalibCondData>(m_moduleDataKey, ctx)->getFEI4OverflowToT();
              const std::vector<int>& ToTs = pixclus->prepRawData()->totList();
              
              for (int pixToT : ToTs) {
                if (pixToT >= overflowIBLToT) {
                  //overflow pixel hit -- flag cluster
                  iblOverflow = 1;
                  break; //no need to check other hits of this cluster
                }
              }// end
              cluster.isIBL = true;
              cluster.iblOverflow = iblOverflow; //why int?
            }

            /// Skip if too shallow.  MOVED TO getClusterdEdx()
            //if (std::abs(cluster.cosalpha)<0.16) { continue; }

            /// Add function to insert info into dEdxMap.
            /// Apply all cuts here.  Don't forget abs(cosalpha).
            /// Make them configurable?
            
            float clusterdEdx = getClusterdEdx(cluster, goodPixelhits, nUsedIBLOverflowHits); // returns -1 if bad cluster measurement.

            /// Check if good measurement.
            if (clusterdEdx < 0.0) { continue; }

            cluster.dEdx = clusterdEdx;
            
            /// Apply cluster-level equalization.
            /// Read from trees on CVMFS or from conditions database.
            if(m_equalizeClusterMeasurements){

              /// Insert code from Rebeccas Hicks' QT task here (ATLIDTRKCP-579).
              /// Pulls dE/dx equalization SF from conditons database.
              float SF = 1.;

              /// Apply scale factor and store
              cluster.dEdxEq = clusterdEdx * SF;

            }

            clusters.push_back(cluster);
          } // pixclus iterator
        } // end if measurement found and not outlier. 
      } // tsos iterator
    } // end if reco track states found.

    /// Always calculate raw truncated mean.
    float averagedEdx = getTruncatedMean(clusters, nUsedHits, goodPixelhits);
    
    /// Calculate equalized truncated mean.
    if(m_equalizeClusterMeasurements) {
      int nUsedHitsEq=0; // need separate counter or will double count if calculating both raw and equalized dE/dx
      float averagedEdxEq = getTruncatedMean(clusters, nUsedHitsEq, goodPixelhits, true);

      /// Sanity check that nUsedHits and nUsedHitsEq are the same.
      if (nUsedHitsEq != nUsedHits) {
        ATH_MSG_ERROR("The numberOfUsedHitsdEdx calculated for the raw ("<< nUsedHits <<") and equalized ("<< nUsedHitsEq <<") dE/dx differ!  Should not happen!");
      }

      return averagedEdxEq;
    }

    return averagedEdx;

  }
#endif

  //////////////////
  //////////////////
  //////////////////

#ifdef XAOD_STANDALONE
  /// This is the version ran via a CP alg on the xAOD EDM.
  /// Will return the dE/dx, and will update nUsedHits (the divisor in the truncated mean) and nUsedIBLOverflowHits.
  /// Whether this is the raw or equalized dE/dx will be determined by the tool properties.
  float PixelToTPIDTool::dEdx(const xAOD::TrackParticle& track,
                                  int& nUsedHits,
                                  int& nUsedIBLOverflowHits) const 
  {

    using StatesOnTrack = std::vector<ElementLink<xAOD::TrackStateValidationContainer>>;

    /// Total number of good pixel hits considered in truncated mean calc.    
    /// Clusters will be subjected to various cuts.
    int goodPixelhits = 0;

    /// All pixel hits linked to the track.
    int allPixelHits = 0;

    /// Passed by ref, so will update here.  
    nUsedHits=0; // divisor in truncated mean.
    nUsedIBLOverflowHits=0; // number of IBL hits in overflow.

    /// Get pixel clusters in this simple struct to abstract away the two EDMs.
    std::vector<PixelCluster> clusters;

    /// Second value keeps track if the cluster is in IBL and has at least an overflow hit
    std::multimap<float,int> dEdxMap;

    /// Determine if data or MC.
    /// If data, get the run number for scale factor determination.  
    /// If MC, do not allow m_equalizeClusterMeasurements to be true.
    ///    Not supporting dE/dx equalization for  MC at this time.
    ///    The radiation damage is modeled in MC23, but not MC20.
    ///    Eventually, can apply scale factors to "undo" MC23 rad damage modeling.
    ///    For now, only allow m_equalizeClusterMeasurements == false.
    ////   Still useful to decorate the clusters with their raw dE/dx measurements.
    int runNumber = 0; // won't be used if not in XAOD_STANDALONE, since not applying SFs from trees.
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);        
    if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { //MC
      ATH_MSG_DEBUG("The current event is simulation.");
      if( m_equalizeClusterMeasurements ) {
        ATH_MSG_ERROR("Requested to equalize the dE/dx, but this is not yet supported for MC.");
        ATH_MSG_ERROR("Eventually, can apply scale factors to \"undo\" the radiation modeling in MC23.");
        ATH_MSG_ERROR("Or equalize the MC to the data reference run.");
        
        /// Throw runtime error since not returning a status code.
        throw std::runtime_error("Cannot set EqualizeClusterMeasurements to true for MC (for now).");
      }
    }
    else { // Data
      ATH_MSG_DEBUG("The current event is data.  Getting run number.");
      runNumber =  eventInfo->runNumber();
    }
    
    /// If using SFs from trees, get the closest run.
    /// Ideally, would filter the dataframe in initialize, only keeping the rows from the closest runNumber.
    /// But we don't know the runNumber until execute...
    int closestRunNumber = 0;

    std::shared_ptr<ROOT::RDF::RNode> filtered_df;

    if( m_equalizeClusterMeasurements ) {
      
      /// First, try to find it (lock the map while accessing)
      {
        std::lock_guard<std::mutex> lock(m_mapMutex);

        auto it = m_filteredRDFMap.find(runNumber);
        if (it != m_filteredRDFMap.end()) {
          ATH_MSG_DEBUG("SFs for this run " << runNumber << " already cached!");
          filtered_df = it->second;
        }
      }

      if (!filtered_df) {
        ATH_MSG_DEBUG("SFs for run " << runNumber << " are NOT already cached.  Will filter RDF and cache now.");
        auto df = std::make_shared<ROOT::RDataFrame>(*m_df);
        auto runNumbers = df->Take<int>("runNumber");
        closestRunNumber = *std::min_element(runNumbers.begin(), runNumbers.end(),
                                             [runNumber](int a, int b) {
                                               return std::abs(a - runNumber) < std::abs(b - runNumber); });
        ATH_MSG_INFO("Closest run number:" << closestRunNumber);
        auto filtered = std::make_shared<ROOT::RDF::RNode>(df->Filter([closestRunNumber](int run) { return run == closestRunNumber; }, {"runNumber"}));
        /// Store it in the map (lock again)
        {
          std::lock_guard<std::mutex> lock(m_mapMutex);
          m_filteredRDFMap[runNumber] = filtered;
        }
        filtered_df = filtered;
      }
    }

    /// Declare decorators here
    /// Will cause issues with TrackParticleCreator during reco if included outside of XAOD_STANDALONE.
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float > dEdxHandle(m_clusterdEdxKey); // no ctx?
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float > dEdxEqHandle(m_clusterdEdxEqKey); // no ctx?

    /// Check for track states:
    static const SG::AuxElement::ConstAccessor< StatesOnTrack > trackStateAcc(m_msosLink);
    if( ! trackStateAcc.isAvailable( track ) ) {
      ATH_MSG_INFO("Cannot find TrackState link from xAOD::TrackParticle. Skipping track.");
      return -1;
    }
    const StatesOnTrack& measurementsOnTrack = trackStateAcc(track);

    /// Loop over MSOS.
    for( const ElementLink<xAOD::TrackStateValidationContainer>& msos : measurementsOnTrack) {
      if (not msos.isValid()) {
        continue; //not a valid link.  Can happen if clusters are thinned away via ThinInDetClustersAlg.
      }
      if ((int) (*msos)->detType() != 1) {
        continue; // not a pixel cluster. See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs.h
      }
      allPixelHits++;
      if ( (*msos)->type()!=0) {
        continue; // not fittable.  See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs. Want this?
      }
      
      /// Get the corresponding TrackMeasurementValidation object (cluster/drift tube)
      const ElementLink<xAOD::TrackMeasurementValidationContainer> pixclus = (*msos)->trackMeasurementValidationLink();
      if (not pixclus.isValid()) {
        ATH_MSG_INFO("Invalid link to cluster.");
        continue; //not a valid link
      }
      if (*pixclus == nullptr) {
        ATH_MSG_INFO("pixclus is a nullptr.");
        continue; //not linking to a valid object -- is it necessary?
      }
      
      /// Build PixelCluster to abstract away the EDMs.
      PixelCluster cluster;
      static const SG::AuxElement::ConstAccessor< float > localXAcc("localX");
      if (localXAcc.isAvailable(**pixclus)) {
        cluster.locx = localXAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("localX auxdata is missing!");
        continue;
      }

      static const SG::AuxElement::ConstAccessor< float > localYAcc("localY");
      if (localYAcc.isAvailable(**pixclus)) {
        cluster.locy = localYAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("localY auxdata is missing!");
        continue;
      }

      static const SG::AuxElement::ConstAccessor< int > becAcc("bec");
      if (becAcc.isAvailable(**pixclus)) {
        cluster.bec = becAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("bec auxdata is missing!");
        continue;
      }

      static const SG::AuxElement::ConstAccessor< int > layerAcc("layer");
      if (layerAcc.isAvailable(**pixclus)) {
        cluster.layer = layerAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("layer auxdata is missing!");
        continue;
      }

      static const SG::AuxElement::ConstAccessor< int > etaAcc("eta_module");
      if (etaAcc.isAvailable(**pixclus)) {
        cluster.eta_module = etaAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("eta_module auxdata is missing!");
        continue;
      }

      float msosTheta = (*msos)->localTheta();
      float msosPhi = (*msos)->localPhi();
      float alpha = std::atan(std::hypot(std::tan(msosTheta),std::tan(msosPhi))); //check using correct Theta, phi
      cluster.cosalpha = std::cos(alpha);

      static const SG::AuxElement::ConstAccessor< float > chargeAcc("charge");
      if (chargeAcc.isAvailable(**pixclus)) {
        //cluster.charge = chargeAcc(**pixclus) * cluster.cosalpha; // NB: multiplying by cosalpha!
        cluster.charge = chargeAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("charge auxdata is missing!");
        continue;
      }

      /// Keep track if this is an ibl cluster with overflow
      int iblOverflow=0;
      if ((cluster.bec==0) and (cluster.layer==0)) { // check if IBL
        int overflowIBLToT = 16; // CHECK!
        std::vector<int> ToTs;
        static const SG::AuxElement::ConstAccessor< std::vector<int> > totAcc("rdo_tot");
        if (totAcc.isAvailable(**pixclus)) {
          ToTs = totAcc(**pixclus);
        } else {
          ATH_MSG_WARNING("rdo_tot auxdata is missing!");
          continue;
        }
        
        for (int pixToT : ToTs) {
          if (pixToT >= overflowIBLToT) {
            //overflow pixel hit -- flag cluster
            iblOverflow = 1;
            break; //no need to check other hits of this cluster
          }
        }// end
        cluster.isIBL = true;
        cluster.iblOverflow = iblOverflow;
      }

      /// Skip if too shallow. MOVED TO getClusterdEdx()
      //if (std::abs(cluster.cosalpha)<0.16) { continue; }
      
      /// Get raw cluster dE/dx
      float clusterdEdx = getClusterdEdx(cluster, goodPixelhits, nUsedIBLOverflowHits); // returns -1 if bad cluster measurement.

      /// Check if good measurement.
      if (clusterdEdx < 0.0) { continue; }
      
      /// Store
      cluster.dEdx = clusterdEdx;

      /// Decorate pixel cluster on track with raw dE/dx.
      ATH_MSG_DEBUG("Will decorate  variable " << m_clusterdEdxKey << " with value " << cluster.dEdx);
      dEdxHandle(**pixclus) = cluster.dEdx;

      /// Apply cluster-level equalization.
      /// Read from trees on CVMFS or from conditions database.
      if(m_equalizeClusterMeasurements){
        
        /// Get SF and error from the filtered dataframe ((tree->RDataFrame->filtered RDataFrame for specific run).
        auto result = filtered_df->Filter(
                                          [&cluster](int bec, int layerID, int etaM) {
                                            return bec == cluster.bec && layerID == cluster.layer && etaM == abs(cluster.eta_module); // average over phi & +-z.
                                          },
                                          {"bec", "layerID", "etaM"});

        auto SF_values = result.Take<double>("SF");
        auto SF_error_values = result.Take<double>("SF_error");
        if (SF_values->empty() || SF_error_values->empty()) {
          ATH_MSG_ERROR("Could not find the scale factor matching the (bec, layer, module eta) of this pixel cluster!");
        }
        if (SF_values->size()>1 || SF_error_values->size()>1) {
          ATH_MSG_ERROR("Found multiple scale factors matching the (bec, layer, module eta) of this pixel cluster!");
        }

        double SF = SF_values->at(0);
        double SF_error = SF_error_values->at(0);
        ATH_MSG_DEBUG("Test: found SF " << SF << " with error " << SF_error << " for this pixel cluster.");

        /// Apply scale factor and store
        cluster.dEdxEq = clusterdEdx * SF;

        /// Decorate pixel cluster on track with equalized dE/dx.
        ATH_MSG_DEBUG("Will decorate  variable " << m_clusterdEdxEqKey << " with value " << cluster.dEdxEq);
        dEdxEqHandle(**pixclus) = cluster.dEdxEq;
      }
      
      /// Add cluster to vector for truncated mean calculation
      clusters.push_back(cluster);
    } // MSOS iterator
    
    /// Always calculate raw truncated mean.
    float averagedEdx = getTruncatedMean(clusters, nUsedHits, goodPixelhits);
    
    /// Sanity check that the recalculated raw dE/dx matches what was calculated during reco and stored as a track summary variable.    
    float stored_dEdx { 0 };
    unsigned char stored_numberOfUsedHitsdEdx = 99;
    float epsilon = 1e-3;
    track.summaryValue(stored_dEdx, xAOD::pixeldEdx);
    static const SG::AuxElement::ConstAccessor< unsigned char > nUsedAcc("numberOfUsedHitsdEdx");
    if (nUsedAcc.isAvailable(track)) {
      stored_numberOfUsedHitsdEdx = nUsedAcc(track);
    } else {
      ATH_MSG_WARNING("numberOfUsedHitsdEdx auxdata is missing!");
    }
    if(allPixelHits == 0) {
      ATH_MSG_DEBUG("No pixel clusters found on track, so cannot compare calculated dE/dx with value stored in AOD."
                    << "\nThis can occur when pixel clusters are not saved to the AOD, or if they are thinned.");
    }
    else {
      if ( std::fabs(stored_dEdx - averagedEdx) > epsilon ) {
        ATH_MSG_WARNING("The track dE/dx stored in the AOD as summary variable (" << stored_dEdx
                        << ") does not match the value calculated here (" << averagedEdx << ")!"
                        << "\nThis may be due to the local (x,y) of the cluster migrating from the ESD to xAOD EDM.");
      }
      if ( (int) stored_numberOfUsedHitsdEdx != nUsedHits ) {
        ATH_MSG_WARNING("The numberOfUsedHitsdEdx stored in the AOD ("<< (int) stored_numberOfUsedHitsdEdx
                        << ") does not match the value calculated here ("<< nUsedHits <<")!"
                        << "\nThis may be due to the local (x,y) of the cluster migrating from the ESD to xAOD EDM.");
      }
    }

    /// Calculate equalized truncated mean.
    if(m_equalizeClusterMeasurements) {
      int nUsedHitsEq=0; // need separate counter or will double count if calculating both raw and equalized dE/dx
      float averagedEdxEq = getTruncatedMean(clusters, nUsedHitsEq, goodPixelhits, true);
      
      /// Sanity check that nUsedHits and nUsedHitsEq are the same.
      if (nUsedHitsEq != nUsedHits) {
        ATH_MSG_ERROR("The numberOfUsedHitsdEdx calculated for the raw ("<< nUsedHits <<") and equalized ("<< nUsedHitsEq <<") dE/dx differ!  Should not happen!");
      }
      
      return(averagedEdxEq);
    }

    return averagedEdx;
  }
#endif    

  //////////////////
  //////////////////
  //////////////////
  
  /// All functions below are shared between the two dEdx() functions.
  /// They take PixelClusters as input, a simple struct defined to abstract away the two EDMs.
  /// This prevents the duplication of the truncated mean logic, as well as the cluster (x,y) cuts.
  /// The number of good pixel hits (hits considered for truncated mean calc) is passed by ref & incremented.
  /// As is the number of IBL hits in overflow (again, only if they are considered for the trunc mean calc).

  /// Returns the cluster dE/dx.
  float PixelToTPIDTool::getClusterdEdx(const PixelCluster& cluster,
                                            int& pixelhits,
                                            int& nUsedIBLOverflowHits) const{    
    float dEdxValue;

    /// Remove clusters if track is too shallow.
    if ( std::abs(cluster.cosalpha) < 0.16 ) {
      ATH_MSG_DEBUG("Cluster assigned dE/dx = -1 due to shallow path through sensor: cos(alpha) = " << cluster.cosalpha);
      return -1;
    }

    ///  Apply extra cluster cleaning cuts for improved dE/dx measurements.
    if (m_extraClusterCleaning) {

      /// Add extra cleaning cuts here when ready.
      /// Return -1 if fail.

    }

    /// Now check layer & barrel vs endcap, applying local (x,y) cuts.
    if (cluster.isIBL) { // check if IBL      
      if (((cluster.eta_module >= -10 && cluster.eta_module <= -7) ||
           (cluster.eta_module >= 6 && cluster.eta_module <= 9)) &&
          (fabs(cluster.locy) < 10. &&
           (cluster.locx > -8.33 &&
            cluster.locx < 8.3))) { // check if IBL 3D and good cluster selection

        dEdxValue = cluster.charge * cluster.cosalpha *  m_conversionfactor / m_IBL_3D_sensorthickness;
        pixelhits++;
        if (cluster.iblOverflow == 1) {
          nUsedIBLOverflowHits++;
        }
      } else if ((cluster.eta_module >= -6 && cluster.eta_module <= 5) &&
                 (fabs(cluster.locy) < 20. &&
                  (cluster.locx > -8.33 &&
                   cluster.locx < 8.3))) { // check if IBL planar and good cluster

        dEdxValue = cluster.charge * cluster.cosalpha * m_conversionfactor / m_IBL_PLANAR_sensorthickness;
        pixelhits++;
        if (cluster.iblOverflow == 1) {
          nUsedIBLOverflowHits++;
        }
      } else {
        dEdxValue=-1;
      } // end check which IBL Module
    }
    //PIXEL layer and ENDCAP
    else if(cluster.bec==0 && fabs(cluster.locy)<30. &&  ((cluster.locx>-8.20 && cluster.locx<-0.60) || (cluster.locx>0.50 && cluster.locx<8.10))) {
      dEdxValue = cluster.charge * cluster.cosalpha * m_conversionfactor / m_Pixel_sensorthickness;
      pixelhits++;
    }
    else if (std::abs(cluster.bec)==2 && fabs(cluster.locy)<30. && ((cluster.locx>-8.15 && cluster.locx<-0.55) || (cluster.locx>0.55 && cluster.locx<8.15))) {
      dEdxValue = cluster.charge * cluster.cosalpha * m_conversionfactor / m_Pixel_sensorthickness;
      pixelhits++;
    }
    else{
      dEdxValue=-1;
      ATH_MSG_DEBUG("Returning -1 for a cluster dE/dx. bec: " << cluster.bec << ", layer: " << cluster.layer << ", locx: " << cluster.locx << ", locy: " << cluster.locy);
    }

    return dEdxValue;
  }

  //////////////////
  //////////////////
  //////////////////

  /// Returns the truncated mean over the track. 
  /// If equalize == true, it will use the equalized cluster dE/dx measurement in the calculation.
  /// NB:  nUsedHits is passed by reference and updated.  Do not call this function multiple times with the same counter.
  float PixelToTPIDTool::getTruncatedMean(const std::vector<PixelCluster>& clusters,
                                              int& nUsedHits, 
                                              int pixelhits,
                                              bool equalize) const {

    /// Get the dEdxMap.
    /// First in pair is the dE/dx (raw or equalized).  Second indicates if it's a IBL cluster in with ToT in overflow.
    /// Multimaps  will automatically sort based on the first element in the pair.  Useful for truncated mean alg.
    std::multimap<float,int> dEdxMap;
    for (const auto& cluster : clusters) {
      if(equalize) {
        dEdxMap.insert(std::pair<float, int>(cluster.dEdxEq, cluster.iblOverflow));
      }
      else {
        dEdxMap.insert(std::pair<float, int>(cluster.dEdx, cluster.iblOverflow));
      }
    }

    /// Now calculate the truncated mean.
    float averagedEdx=0.;
    nUsedHits=0;
    int IBLOverflow=0;
    for (std::pair<float,int> itdEdx : dEdxMap) {
      if (itdEdx.second==0) {
        averagedEdx += itdEdx.first;
        nUsedHits++;
      }
      if (itdEdx.second>0) { IBLOverflow++; }

      //break, skipping last or the two last elements depending on total measurements
      if (((int)pixelhits>=5) and ((int)nUsedHits>=(int)pixelhits-2)) { break; }

      //break, IBL Overflow case pixelhits==3 and 4
      if ((int)IBLOverflow>0 and ((int)pixelhits==3) and (int)nUsedHits==1) { break; }
      if ((int)IBLOverflow>0 and ((int)pixelhits==4) and (int)nUsedHits==2) { break; }

      if (((int)pixelhits > 1) and ((int)nUsedHits >=(int)pixelhits-1)) { break; }

      if ((int)IBLOverflow>0 and (int)pixelhits==1) { //only IBL in overflow
        averagedEdx=itdEdx.first;
        break;
      }
    }

    if (nUsedHits>0 or (nUsedHits==0 and(int)IBLOverflow>0 and (int)pixelhits==1)) {
      if (nUsedHits>0) { averagedEdx=averagedEdx/nUsedHits; }

      ATH_MSG_DEBUG("NEW dEdx = " << averagedEdx);
      ATH_MSG_DEBUG("Used hits: " << nUsedHits << ", IBL overflows: " << IBLOverflow );
      ATH_MSG_DEBUG("Original number of measurements = " << pixelhits << "( map size = " << dEdxMap.size() << ")");
      return averagedEdx;
    }
    return -1;
  }

} // namespace CP
