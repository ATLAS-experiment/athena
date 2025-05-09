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

    if (m_tightClusterCleaning) {
      ATH_MSG_WARNING("Tight cluster cleaning requested for dE/dx calculation, but feature not yet supported.");
    }

    /// For determining data vs MC, as well as run number.
    ATH_CHECK(m_eventInfo.initialize());

    /// xAOD EDM ///
#ifdef XAOD_STANDALONE

    if (m_equalizeClusterMeasurements && m_equalizeTrackMeasurements) {
      ATH_MSG_ERROR("Can only equalize the dE/dx measurements at cluster-level OR track-level, not both.");
      return StatusCode::FAILURE;
    }
    else if (m_equalizeClusterMeasurements) {
      ATH_MSG_INFO("Will equalize individual cluster dE/dx measurements and return the truncated mean.");
    }
    else if (m_equalizeTrackMeasurements) {
      ATH_MSG_INFO("Will equalize the track-level truncated mean dE/dx from the AOD.");
    }
    else{
      ATH_MSG_ERROR("Must choose to equalize the dE/dx measurements at cluster-level OR track-level.");
      return StatusCode::FAILURE;
    }

    /// Set up scale factors. In XAOD_STANDALONE, read SFs from trees stored on CVMFS
    if(m_equalizeClusterMeasurements || m_equalizeTrackMeasurements) {
      if (m_sfLocalFileName != "") {
        ATH_MSG_WARNING("!! SETTING UP WITH USER SPECIFIED INPUT LOCATION \"" << m_sfLocalFileName << "\"!! FOR DEVELOPMENT USE ONLY !! ");
      }
      ATH_CHECK(initSFsFromTrees());
    }

    /// Initialize decorators, independent of equalization strategy.
    /// Will only use if equalizing at cluster-level, not track-level.
    ANA_CHECK ( m_clusterdEdxKey.initialize() );
    ANA_CHECK ( m_clusterdEdxEqKey.initialize() );
    if(m_equalizeClusterMeasurements) {
      ATH_MSG_INFO("Will decorate PixelClusters with their raw dE/dx using key: " << m_clusterdEdxKey);
      ATH_MSG_INFO("Will decorate PixelClusters with their equalized dE/dx using key: " << m_clusterdEdxEqKey);
    }
#endif

    /// ESD EDM ///
#ifndef XAOD_STANDALONE
    
    /// When NOT in XAOD_STANDALONE, will read SFs from conditions database instead.
    /// See QT from Rebecca Hicks (ATLIDTRKCP-579).

    if (m_equalizeClusterMeasurements) {
      ATH_MSG_INFO("Will equalize individual cluster dE/dx measurements and return the truncated mean.");
    }
    else{
      ATH_MSG_INFO("Will NOT equalize any dE/dx measurement."); // Equalization is optional during reconstruction.
    }

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

    /// Second value keeps track if the cluster is in IBL and has at least an overflow hit
    std::multimap<float,int> dEdxMap;

    /// Check if MC or data
    bool isMC = false;
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo, ctx);        
    if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { //MC
      ATH_MSG_DEBUG("The current event is simulation.");
      isMC = true;
    }
    else { // Data
      ATH_MSG_DEBUG("The current event is data.");
    }
      
    if(isMC && m_equalizeClusterMeasurements) {
      ATH_MSG_ERROR("Requested to equalize the dE/dx, but this is not yet supported for MC.");
      ATH_MSG_ERROR("Eventually, can apply scale factors to \"undo\" the radiation modeling in MC23.");
      ATH_MSG_ERROR("Or equalize the MC to the data reference run.");        
      /// Throw runtime error since not returning a status code.
      throw std::runtime_error("Cannot equalize MC (for now).");
    }

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

            /// If good measurement, update cluster raw cluster dE/dx, passdEdxCutsLoose, and passdEdxCutsTight.
            /// Also, increment goodPixelhits & nUsedIBLOverflowHits
            /// If bad measurement, keep default negative value for dE/dx, don't increment.
            getClusterdEdx(cluster, goodPixelhits, nUsedIBLOverflowHits);

            /// Check if good measurement.
            if (cluster.dEdx < 0.0) { continue; }

            /// Apply cluster-level equalization.
            /// Read conditions database.
            if(m_equalizeClusterMeasurements){

              /// Insert code from Rebeccas Hicks' QT task here (ATLIDTRKCP-579).
              /// Pulls dE/dx equalization SF from conditons database.
              float SF = 1.;

              /// Apply scale factor and store
              cluster.dEdxEq = cluster.dEdx * SF;

            }

            /// Only pushing back clusters with good dE/dx measurements!
            clusters.push_back(cluster);

          } // pixclus iterator
        } // end if measurement found and not outlier. 
      } // tsos iterator
    } // end if reco track states found.

    /// Always calculate raw truncated mean & update number of hits used in truncated mean.
    float averagedEdx = getTruncatedMean(clusters, nUsedHits, goodPixelhits);
    
    /// Calculate equalized truncated mean.
    if(m_equalizeClusterMeasurements) {
      int nUsedHitsEq=0; // need separate counter or will double count if calculating both raw and equalized dE/dx
      float averagedEdxEq = getTruncatedMean(clusters, nUsedHitsEq, goodPixelhits, true);

      /// Sanity check that nUsedHits and nUsedHitsEq are the same.
      if (nUsedHitsEq != nUsedHits) {
        ATH_MSG_ERROR("The numberOfUsedHitsdEdx calculated for the raw ("<< nUsedHits <<") and equalized ("<< nUsedHitsEq <<") dE/dx differ!  Should not happen!");
      }
      
      /// Return equalized truncated mean dE/dx
      return averagedEdxEq;
    }

    /// Return raw truncated mean dE/dx
    return averagedEdx;

  }
#endif

  //////////////////
  //////////////////
  //////////////////

#ifdef XAOD_STANDALONE
  /// This is the version ran via a CP alg on the xAOD EDM.
  /// Will return the dE/dx, and will update nUsedHits (the divisor in the truncated mean) and nUsedIBLOverflowHits.
  /// Whether this is the track- or cluster-equalized dE/dx will be determined by the tool properties.
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

    /// Get the raw track-level truncated mean dE/dx from the AOD.
    /// If equalizing at the track level, will apply single SF to this measurement.
    /// If equalizing at the cluster level, will do a sanity check that calculations match between reco and here.
    float stored_dEdx { 0 };
    unsigned char stored_numberOfUsedHitsdEdx = 99;
    unsigned char stored_numberOfIBLOverflowsdEdx = 99;
    track.summaryValue(stored_dEdx, xAOD::pixeldEdx);
    static const SG::AuxElement::ConstAccessor< unsigned char > nUsedAcc("numberOfUsedHitsdEdx");
    if (nUsedAcc.isAvailable(track)) {
      stored_numberOfUsedHitsdEdx = nUsedAcc(track);
    } else {
      ATH_MSG_WARNING("numberOfUsedHitsdEdx auxdata is missing!");
    }
    static const SG::AuxElement::ConstAccessor< unsigned char > nIBLOFAcc("numberOfIBLOverflowsdEdx");
    if (nIBLOFAcc.isAvailable(track)) {
      stored_numberOfIBLOverflowsdEdx = nIBLOFAcc(track);
    } else {
      ATH_MSG_WARNING("numberOfIBLOverflowsdEdx auxdata is missing!");
    }

    /// Determine if data or MC.
    /// If data, get the run number for scale factor determination.  
    /// If MC, throw error.
    ///    Not supporting dE/dx equalization for MC at this time.
    ///    The radiation damage is modeled in MC23, but not MC20.
    ///    Eventually, can apply scale factors to "undo" MC23 rad damage modeling.
    bool isMC = false;
    int runNumber = 0; // won't be used if not in XAOD_STANDALONE, since not applying SFs from trees.
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);        
    if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { //MC
      ATH_MSG_DEBUG("The current event is simulation.");
      isMC = true;
    }
    else { // Data
      runNumber =  eventInfo->runNumber();
      ATH_MSG_INFO("The current event is data with run number: " << runNumber);
    }
    
    if(isMC && m_equalizeClusterMeasurements) {
      ATH_MSG_ERROR("Requested to equalize the dE/dx, but this is not yet supported for MC.");
      ATH_MSG_ERROR("Eventually, can apply scale factors to \"undo\" the radiation modeling in MC23.");
      ATH_MSG_ERROR("Or equalize the MC to the data reference run.");        
      /// Throw runtime error since not returning a status code.
      throw std::runtime_error("Cannot equalize MC (for now).");
    }
    
    ///////////////
    /// Get SFs ///
    ///////////////

    /// Find closest run in SF RDF & filter to keep just these.
    /// Don't know the runNumber until execute, so trying to make this efficient.
    /// Cache the filtered RDF for this run in the map.
    int closestRunNumber = 0;
    std::shared_ptr<ROOT::RDF::RNode> filtered_df;

    /// First, try to find it (lock the map while accessing)
    {
      std::lock_guard<std::mutex> lock(m_mapMutex);
      
      auto it = m_filteredRDFMap.find(runNumber);
      if (it != m_filteredRDFMap.end()) {
        ATH_MSG_DEBUG("SFs for this run " << runNumber << " already cached!");
        filtered_df = it->second;
      }
    }
    /// If not already cached, filter the dataframe and cache it now.
    if (!filtered_df) {
      ATH_MSG_INFO("SFs for run " << runNumber << " are NOT already cached.  Will filter RDF and cache now.");
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

    //////////////////////
    /// Track Level EQ ///
    //////////////////////

    if(m_equalizeTrackMeasurements) {

      /// Determine which SF eta bin to use.  SFs take advantage of phi & +-z symmetry.
      /// Only derive SFs for |eta| < 2.5.
      /// If |eta|>2.5, apply last SF.
      double absEta = abs(track.eta());
      if(absEta > 2.5) {
        absEta = 2.49;
      }
      auto result = filtered_df->Filter(
                                         [absEta](double etaLow, double etaHigh) {
                                           return etaLow <= absEta && absEta <= etaHigh;
                                         },
                                         {"etaLow", "etaHigh"}
                                         );

      auto SF_values = ( (int) stored_numberOfIBLOverflowsdEdx > 0) ? result.Take<double>("SF_IBLOFYes") : result.Take<double>("SF_IBLOFNo");
      if (SF_values->empty()) {
        ATH_MSG_ERROR("Could not find the scale factor matching the eta & IBLOF status of this track!");
        ATH_MSG_ERROR("Run: " << runNumber << ", |eta| = " << absEta << ", IBLOF: " << stored_numberOfIBLOverflowsdEdx);
        throw std::runtime_error("Cannot equalize track dE/dx.");
      }
      if (SF_values->size()>1) {
        ATH_MSG_ERROR("Found multiple scale factors matching the eta & IBLOF of this track!");
        throw std::runtime_error("Cannot equalize track dE/dx.");
      }

      double SF = SF_values->at(0);
      ATH_MSG_DEBUG("Test: found SF " << SF << " for this track.");

      /// Return the equalized dE/dx
      float averagedEdxEq = stored_dEdx * SF;
      nUsedHits = stored_numberOfUsedHitsdEdx; // not updated since not recalculating truncated mean.
      nUsedIBLOverflowHits = stored_numberOfIBLOverflowsdEdx; // not updated since not recalculating truncated mean.
      ATH_MSG_DEBUG("Returning track-level equalized truncated mean dE/dx: " << averagedEdxEq);
      return averagedEdxEq;
    }

    ////////////////////////
    /// Cluster Level EQ ///
    ////////////////////////

    /// Declare decorators here
    /// Will cause issues with TrackParticleCreator during reco if included outside of XAOD_STANDALONE.
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float > dEdxHandle(m_clusterdEdxKey);
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, float > dEdxEqHandle(m_clusterdEdxEqKey);

    /// Check for track states:
    static const SG::AuxElement::ConstAccessor< StatesOnTrack > trackStateAcc(m_msosLink);
    if( ! trackStateAcc.isAvailable( track ) ) {
      ATH_MSG_INFO("Requested cluster-level equalaization, but cannot find TrackState link from xAOD::TrackParticle."); // FIXME downgrade to DEBUG?
      ATH_MSG_INFO("Could be missing or thinned away. Skipping track."); // FIXME downgrade to DEBUG?
      /// Return an invalid value for the equalized truncated mean dE/dx.
      /// Do not update nUsedHits or nUsedIBLOverflowHits.
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
        continue; // not fittable.  See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs. Want this?  FIXME
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
      float alpha = std::atan(std::hypot(std::tan(msosTheta),std::tan(msosPhi))); //check using correct Theta, phi. TODO
      cluster.cosalpha = std::cos(alpha);

      static const SG::AuxElement::ConstAccessor< float > chargeAcc("charge");
      if (chargeAcc.isAvailable(**pixclus)) {
        cluster.charge = chargeAcc(**pixclus);
      } else {
        ATH_MSG_WARNING("charge auxdata is missing!");
        continue;
      }

      /// Keep track if this is an ibl cluster with overflow
      int iblOverflow=0;
      if ((cluster.bec==0) and (cluster.layer==0)) { // check if IBL
        int overflowIBLToT = 16; // see getFEI4OverflowToT() in PixelChargeCalibCondData.h
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

      /// If good measurement, update cluster raw cluster dE/dx, passdEdxCutsLoose, and passdEdxCutsTight.
      /// Also, increment goodPixelhits & nUsedIBLOverflowHits
      /// If bad measurement, keep default negative value for dE/dx, don't increment.
      getClusterdEdx(cluster, goodPixelhits, nUsedIBLOverflowHits);

      /// Decorate pixel cluster on track with raw dE/dx, whether it's a good dE/dx measurement or not.
      /// Will be negative default value if cluster fails the cleaning cuts.
      /// By default, only loose cuts on the local (x,y) and track angle are applied.
      /// If requested, tight cuts are applied.
      ATH_MSG_DEBUG("Will decorate  variable " << m_clusterdEdxKey << " with value " << cluster.dEdx);
      dEdxHandle(**pixclus) = cluster.dEdx;

      /// Apply cluster-level equalization if it's a good measurement.
      /// Otherwise, leave negative default value for cluster.dEdxEq.
      /// Read from trees on CVMFS or from conditions database.
      if(cluster.dEdx > 0.0){

        /// Get bec (barrel vs endcap) & eta bin for the SF.
        /// Cluster SFs take advantage of phi and +-z symmetry.
        /// For pixel barrel layers, eta_module is symmetric, so the module with eta_module==0 is actually centered at eta = 0.
        /// All pixel disk modules have module_eta==0.  
        int sfBECBin = abs(cluster.bec); // for endcap disks
        int sfEtaBin = abs(cluster.eta_module); // for barrel layers
        /// For the IBL, the eta=0 point is actually between the modules with eta_module==-1 and eta_module=0.
        /// So need to map (-1 -> 0), (-2 ->1), etc.
        if (cluster.bec==0 && cluster.layer==0) { // if IBL.
          if (cluster.eta_module<0) {
            sfEtaBin = abs(cluster.eta_module+1);
          }
          /// Also, due to low stats, the 3D sensor modules are combined into  a single SF.
          /// This includes the 4 outermost modules on each side (eta_module>=6 or eta_module<=-7)
          if (sfEtaBin>=6) {
            sfEtaBin = 6;
          }
        }
        
        /// Get SF and error from the filtered dataframe ((tree->RDataFrame->filtered RDataFrame for specific run).
        auto result = filtered_df->Filter(
                                          [cluster, sfBECBin, sfEtaBin](int bec, int layerID, int etaM) {
                                            return bec == sfBECBin && layerID == cluster.layer && etaM == sfEtaBin; // average over phi & +-z.
                                          },
                                          {"bec", "layerID", "etaM"});

        auto SF_values = result.Take<double>("SF");
        auto SF_error_values = result.Take<double>("SF_error");
        if (SF_values->empty() || SF_error_values->empty()) {
          ATH_MSG_ERROR("Could not find the scale factor matching the (bec, layer, module eta) of this pixel cluster!");
          throw std::runtime_error("Cannot equalize cluster dE/dx.");
        }
        if (SF_values->size()>1 || SF_error_values->size()>1) {
          ATH_MSG_ERROR("Found multiple scale factors matching the (bec, layer, module eta) of this pixel cluster!");
          throw std::runtime_error("Cannot equalize cluster dE/dx.");
        }

        double SF = SF_values->at(0);
        double SF_error = SF_error_values->at(0);
        ATH_MSG_DEBUG("Test: found SF " << SF << " with error " << SF_error << " for this pixel cluster.");

        /// Apply scale factor and store
        cluster.dEdxEq = cluster.dEdx * SF;
      }

      /// Decorate pixel cluster on track with equalized dE/dx.
      /// Will be negative if it's a bad measurement.
      ATH_MSG_DEBUG("Will decorate  variable " << m_clusterdEdxEqKey << " with value " << cluster.dEdxEq);
      dEdxEqHandle(**pixclus) = cluster.dEdxEq;
      
      /// Check if good measurement.
      if (cluster.dEdx < 0.0) { continue; }

      /// Add cluster to vector for truncated mean calculation only if it's a good measurement!
      clusters.push_back(cluster);

    } // MSOS iterator
    
    /// Always calculate raw truncated mean.
    float averagedEdx = getTruncatedMean(clusters, nUsedHits, goodPixelhits);
    
    /// Sanity check that the recalculated raw dE/dx matches what was calculated during reco and stored as a track summary variable.
    float epsilon = 1e-3;
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

    int nUsedHitsEq=0; // need separate counter or will double count if calculating both raw and equalized dE/dx
    float averagedEdxEq = getTruncatedMean(clusters, nUsedHitsEq, goodPixelhits, true);
      
    /// Sanity check that nUsedHits and nUsedHitsEq are the same.
    if (nUsedHitsEq != nUsedHits) {
      ATH_MSG_ERROR("The numberOfUsedHitsdEdx calculated for the raw ("<< nUsedHits <<") and equalized ("<< nUsedHitsEq <<") dE/dx differ!  Should not happen!");
    }
      
    return(averagedEdxEq);
  }
#endif    

  //////////////////
  //////////////////
  //////////////////
  
  /// All functions below are shared between the two dEdx() functions.
  /// They take PixelClusters as input, a simple struct defined to abstract away the two EDMs.
  /// This prevents the duplication of the truncated mean logic, as well as the cluster & track angle cuts.
  /// The number of good pixel hits (hits considered for truncated mean calc) is passed by ref & incremented.
  /// As is the number of IBL hits in overflow (again, only if they are considered for the trunc mean calc).

  /// If good measurement, update cluster raw cluster dE/dx, passdEdxCutsLoose, and passdEdxCutsTight.
  /// Also, increment goodPixelhits & nUsedIBLOverflowHits.
  /// If bad measurement, keep default negative value for dE/dx, don't increment.
  void PixelToTPIDTool::getClusterdEdx(PixelCluster& cluster,
                                       int& pixelhits,
                                       int& nUsedIBLOverflowHits) const{    
    float dEdxValue;

    //////////////////
    /// Loose cuts ///
    //////////////////

    /// Remove clusters if track is too shallow.
    if ( std::abs(cluster.cosalpha) < 0.16 ) {
      ATH_MSG_DEBUG("Path through sensor is too shallow for good dE/dx measurement: cos(alpha) = " << cluster.cosalpha);
      /// Do not update cluster.dEdx from default negative value.
      /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
      return;
    }

    /// Now check layer & barrel vs endcap, applying local (x,y) cuts.
    if (cluster.isIBL) { // check if IBL      
      if (((cluster.eta_module >= -10 && cluster.eta_module <= -7) ||
           (cluster.eta_module >= 6 && cluster.eta_module <= 9)) &&
          (fabs(cluster.locy) < 10. &&
           (cluster.locx > -8.33 &&
            cluster.locx < 8.3))) { // check if IBL 3D and good cluster selection

        dEdxValue = cluster.charge * cluster.cosalpha *  m_conversionfactor / m_IBL_3D_sensorthickness;
        cluster.passdEdxCutsLoose = true;
      } else if ((cluster.eta_module >= -6 && cluster.eta_module <= 5) &&
                 (fabs(cluster.locy) < 20. &&
                  (cluster.locx > -8.33 &&
                   cluster.locx < 8.3))) { // check if IBL planar and good cluster

        dEdxValue = cluster.charge * cluster.cosalpha * m_conversionfactor / m_IBL_PLANAR_sensorthickness;
        cluster.passdEdxCutsLoose = true;
      } else { // IBL cluster fails
        /// Do not update cluster.dEdx from default negative value.
        /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
        return;
      }
    }
    /// PIXEL BARREL
    else if(cluster.bec==0 && fabs(cluster.locy)<30. &&  ((cluster.locx>-8.20 && cluster.locx<-0.60) || (cluster.locx>0.50 && cluster.locx<8.10))) {
      dEdxValue = cluster.charge * cluster.cosalpha * m_conversionfactor / m_Pixel_sensorthickness;
      cluster.passdEdxCutsLoose = true;
    }
    /// PIXEL ENDCAP
    else if (std::abs(cluster.bec)==2 && fabs(cluster.locy)<30. && ((cluster.locx>-8.15 && cluster.locx<-0.55) || (cluster.locx>0.55 && cluster.locx<8.15))) {
      dEdxValue = cluster.charge * cluster.cosalpha * m_conversionfactor / m_Pixel_sensorthickness;
      cluster.passdEdxCutsLoose = true;
    }
    else{ // Cluster fails.
      ATH_MSG_DEBUG("Cluster fails loose cuts. bec: " << cluster.bec << ", layer: " << cluster.layer << ", locx: " << cluster.locx << ", locy: " << cluster.locy);
      /// Do not update cluster.dEdx from default negative value.
      /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
      return;
    }

    /// Should not pass this point if cluster.passdEdxCutsLoose == false.
    
    //////////////////
    /// Tight cuts ///
    //////////////////

    ///  Apply extra cluster cleaning cuts for improved dE/dx measurements.
    if (m_tightClusterCleaning) {

      /// TODO!
      /// Add extra cleaning cuts here when ready.
      /// Exploring cuts on cluster size to remove e.g. delta rays.

      /// Now set cluster.passdEdxCutsTight appropriately.

    }

    ////////////////////////////////////
    // Update counters & assign dE/dx //
    ////////////////////////////////////

    if (m_tightClusterCleaning) { // Applying tight cluster cleaning on top of the (nominal) loose cuts on (x,y) and cos(alpha).
      if(cluster.passdEdxCutsLoose && cluster.passdEdxCutsTight) { // technically shouldn't have to check cluster.passdEdxCutsLoose 
        /// Update counters & assign dE/dx.
        pixelhits++;
        if (cluster.isIBL && cluster.iblOverflow>1) {
          nUsedIBLOverflowHits++;
        }
        cluster.dEdx = dEdxValue;
        return;
      }
      else {
        /// Do not update cluster.dEdx from default negative value.
        /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
        return;
      }
    }
    else { // Only applying (nominal) loose cuts on (x,y) and cos(alpha).
      if(cluster.passdEdxCutsLoose) { // technically shouldn't have to check cluster.passdEdxCutsLoose 
        /// Update counters & assign dE/dx.
        pixelhits++;
        if (cluster.isIBL && cluster.iblOverflow>0) {
          nUsedIBLOverflowHits++;
        }
        cluster.dEdx = dEdxValue;
        return;
      }
      else {
        /// Do not update cluster.dEdx from default negative value.
        /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
        return;
      }
    }
  }

  //////////////////
  //////////////////
  //////////////////

  /// Returns the truncated mean over the track. 
  /// If equalize == true, it will use the equalized cluster dE/dx measurement in the calculation.
  /// NB:  nUsedHits (divisor of trunc mean) is passed by reference and updated.  Do not call this function multiple times with the same counter.
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

      ATH_MSG_DEBUG("Truncated mean dEdx = " << averagedEdx);
      ATH_MSG_DEBUG("Used hits: " << nUsedHits << ", IBL overflows: " << IBLOverflow );
      ATH_MSG_DEBUG("Number of good measurements = " << pixelhits << "( map size = " << dEdxMap.size() << ")"); // FIXME! Check pixelHits == dEdxMap.size()
      return averagedEdx;
    }
    return -1;
  }

} // namespace CP
