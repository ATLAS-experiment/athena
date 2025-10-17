//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationTool.h"

namespace CP {
  
  PixelDEdxEqualizationTool::PixelDEdxEqualizationTool(const std::string& tool_name)
    : asg::AsgTool(tool_name),
      m_maxEta(2.5) {
  }
  
  PixelDEdxEqualizationTool::~PixelDEdxEqualizationTool() = default;

  StatusCode PixelDEdxEqualizationTool::initialize() {
    ATH_MSG_INFO("Initializing PixelDEdxEqualizationTool");

    /// Determine equalization strategy
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

    /// Set up scale factors. Read SFs from trees stored in ASG calibration area by default.
    if (m_sfLocalFileName != "") {
        ATH_MSG_WARNING("!! SETTING UP WITH USER SPECIFIED INPUT LOCATION \"" << m_sfLocalFileName << "\"!! FOR DEVELOPMENT USE ONLY !! ");
    }
    else {
      ATH_MSG_INFO("Using default calibration file from ASG area:" << m_sfFileName);
    }
    ATH_CHECK(initSFsFromTrees());

    return StatusCode::SUCCESS;
  }

  //////////////////
  //////////////////
  //////////////////

  /// Initialize SFs from ROOT TTrees from ASG calibration area.
  /// Read into an RDataFrame.  Will filter to get SFs from closest run later.
  StatusCode PixelDEdxEqualizationTool::initSFsFromTrees()  {
    
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

  ////////////////////////////
  /// Get SF from this run ///
  ////////////////////////////

  std::shared_ptr<std::vector<TrackSFRecord>> PixelDEdxEqualizationTool::getRunTrackSFs(const int runNumber) const {

    // First look in the cache
    // Use a shared lock for read-only access
    {
      std::shared_lock readLock(m_cacheMutex);
      auto it = m_cachedTrackSFData.find(runNumber);
      if (it != m_cachedTrackSFData.end()) {
        ATH_MSG_DEBUG("Track SF data for run " << runNumber << " already cached!");
        return it->second;
      }
    } // release shared lock for reading cache.
    
    // SF data not found in the cache, so prepare to find it in the dataframe.
    ATH_MSG_INFO("Track SF data for run " << runNumber << " not cached. Will filter and cache now."); // worst case, prints once per thread.

    // Find closest run number in m_df
    int closestRunNumber = -1;

    // Even though all threads should only be reading m_df, lock to be safe.
    {
      std::shared_lock readLock(m_dfMutex); // Allow multiple simultaneous reads
      auto runNumbers = m_df->Take<int>("runNumber");
      closestRunNumber = *std::min_element(runNumbers.begin(), runNumbers.end(),
                                               [runNumber](int a, int b) {
                                                 return std::abs(a - runNumber) < std::abs(b - runNumber);
                                               });
    } // release shared lock for reading SF RDF.

    if(runNumber!=closestRunNumber) {
      // If MC, make sure the closest run number is the actual run number.
      if(runNumber==284500 || runNumber==300000 || runNumber==310000 || //MC20a/d/e
         runNumber==410000 || runNumber==450000 || runNumber==470000 || runNumber==495000) { //MC23a/d/e/g
        ATH_MSG_WARNING("Could not find track-level SFs for this MC sub-campaign!"); // worst case, prints once per thread.

        // Cache and return an empty pointer.
        // Use a unique lock since writing to cache
        std::shared_ptr<std::vector<TrackSFRecord>> emptyPtr = std::make_shared<std::vector<TrackSFRecord>>();
        {
          std::unique_lock writeLock(m_cacheMutex);
          m_cachedTrackSFData[runNumber] = emptyPtr;
        } // release unique lock for writing to cache.
        return emptyPtr;
      }
      else { // data
        ATH_MSG_WARNING("Could not find track-level SFs for this exact run, so using closest run: " << closestRunNumber); // worst case, prints once per thread.
      }
    }


    // Create vector of TrackSFRecord .
    auto records = std::make_shared<std::vector<TrackSFRecord>>();

    // Filter SF RDF by closestRunNumber
    // Out of an abundance of caution, will use a unique_lock
    {
      std::unique_lock dfWriteLock(m_dfMutex); // Lock for exclusive access when filtering

      std::string expr = "runNumber == " + std::to_string(closestRunNumber);
      auto filtered = m_df->Filter(expr);
    
      // Trigger evaluation to get vectors for needed columns
      auto etaLows = filtered.Take<double>("etaLow");
      auto etaHighs = filtered.Take<double>("etaHigh");
      auto sfYes = filtered.Take<double>("SF_IBLOFYes");
      auto sfNo = filtered.Take<double>("SF_IBLOFNo");

      // Fill vector of TrackSFRecord
      records->reserve(etaLows->size());
      for (size_t i = 0; i < etaLows->size(); ++i) {
        records->emplace_back(TrackSFRecord{
            etaLows->at(i),
            etaHighs->at(i),
            sfYes->at(i),
            sfNo->at(i)
          });
      }
    } // release unique lock for filtering SF RDF

    // Now cache the results
    // Use a unique lock since writing
    {
      std::unique_lock writeLock(m_cacheMutex);
      auto [it, inserted] = m_cachedTrackSFData.emplace(runNumber, records);
      if (!inserted) {
        // Another thread beat us — reuse theirs
        return it->second;
      }
    } // release unique write lock for updating the cache
    
    return records;
  }

  std::shared_ptr<std::vector<ClusterSFRecord>> PixelDEdxEqualizationTool::getRunClusterSFs(const int runNumber) const {

    // First look in the cache.
    // Use a shared lock for read-only access
    {
      std::shared_lock readLock(m_cacheMutex);
      auto it = m_cachedClusterSFData.find(runNumber);
      if (it != m_cachedClusterSFData.end()) {
        ATH_MSG_DEBUG("Cluster SF data for run " << runNumber << " already cached!");
        return it->second;
      }
    } // release shared lock for reading cache.

    // SF data not found in the cache, so prepare to find it in the dataframe.
    ATH_MSG_INFO("SF data for run " << runNumber << " not cached. Will filter and cache now."); // worst case, prints once per thread.
    
    // Find closest run number in m_df
    int closestRunNumber = -1;

    // Even though all threads should only be reading m_df, lock to be safe.
    {
      std::shared_lock readLock(m_dfMutex); // Allow multiple simultaneous reads
      auto runNumbers = m_df->Take<int>("runNumber");
      closestRunNumber = *std::min_element(runNumbers.begin(), runNumbers.end(),
                                           [runNumber](int a, int b) {
                                             return std::abs(a - runNumber) < std::abs(b - runNumber);
                                           });
    } // release shared lock for reading SF RDF.

    if(runNumber!=closestRunNumber) {
      if(runNumber==284500 || runNumber==300000 || runNumber==310000 || //MC20a/d/e
         runNumber==410000 || runNumber==450000 || runNumber==470000 || runNumber==495000) { //MC23a/d/e/g
        ATH_MSG_WARNING("Could not find cluster-level SFs for this MC sub-campaign!"); // worst case, prints once per thread.

        // Cache and return an empty pointer.
        // USe a unique lock for writing to cache.
        std::shared_ptr<std::vector<ClusterSFRecord>> emptyPtr = std::make_shared<std::vector<ClusterSFRecord>>();
        {
          std::unique_lock writeLock(m_cacheMutex);
          m_cachedClusterSFData[runNumber] = emptyPtr;
        } // release unique lock for writing to cache.
        return emptyPtr;
      }
      else { // data
        ATH_MSG_WARNING("Could not find cluster-level SFs for this exact run, so using closest run: " << closestRunNumber); // worst case, prints once per thread.
      }
    }

    // Create vector of TrackSFRecord.
    auto records = std::make_shared<std::vector<ClusterSFRecord>>();

    // Filter by closestRunNumber
    // Out of an abundance of caution, use a unique lock.
    {
      std::unique_lock dfWriteLock(m_dfMutex); // Lock for exclusive access when filtering

      std::string expr = "runNumber == " + std::to_string(closestRunNumber);
      auto filtered = m_df->Filter(expr);
    
      // Trigger evaluation to get vectors for needed columns
      auto becs = filtered.Take<int>("bec");
      auto layers = filtered.Take<int>("layerID");
      auto etas = filtered.Take<int>("etaM");
      auto sfs = filtered.Take<double>("SF");
      auto sf_errors = filtered.Take<double>("SF_error");
    
      // Build vector of SFRecords
      records->reserve(becs->size());
      for (size_t i = 0; i < becs->size(); ++i) {
        records->emplace_back(ClusterSFRecord{
            becs->at(i), layers->at(i), etas->at(i), sfs->at(i), sf_errors->at(i)
          });
      }
    } // release unique lock for filtering SF DF

    // Now cache the results
    // Use a unique lock since writing.
    {
      std::unique_lock writeLock(m_cacheMutex);
      auto [it, inserted] = m_cachedClusterSFData.emplace(runNumber, records);
      if (!inserted) {
        // Another thread beat us — reuse theirs
        return it->second;
      }
    } // release unique lock for updating the cache
    
    return records;
  }

  //////////////////////
  /// Track Level EQ ///
  //////////////////////

  double PixelDEdxEqualizationTool::getTrackdEdxSF(const xAOD::TrackParticle& track, const int runNumber) const {

    unsigned char stored_numberOfIBLOverflowsdEdx = 99;
    static const SG::AuxElement::ConstAccessor<unsigned char> nIBLOFAcc("numberOfIBLOverflowsdEdx");
      if (!nIBLOFAcc.isAvailable(track)) {
        ATH_MSG_ERROR("numberOfIBLOverflowsdEdx auxdata is missing!  Returning SF = -1.");
        return -1.0;
      }
      stored_numberOfIBLOverflowsdEdx = nIBLOFAcc(track);
    
    // Retrieve cached SF data for the given run
    std::shared_ptr<std::vector<TrackSFRecord>> sfRecords = getRunTrackSFs(runNumber);
    if (!sfRecords || sfRecords->empty()) {
      return -1.0;
    }
    
    // Get absolute eta, capped to the max bin range
    double absEta = std::abs(track.eta());
    if (absEta > m_maxEta) {
      absEta = m_maxEta - 0.001;
    }

    // Choose which SF to use based on IBL overflow status
    bool hasIBLOF = static_cast<int>(stored_numberOfIBLOverflowsdEdx) > 0;
    
    double SF = -1.;
    int matchCount = 0;
    
    for (const TrackSFRecord& rec : *sfRecords) {
      if (rec.etaLow <= absEta && absEta <= rec.etaHigh) {
        ++matchCount;
        if (matchCount == 1) {
        SF = hasIBLOF ? rec.SF_IBLOFYes : rec.SF_IBLOFNo;
        }
      }
    }
    
    if (matchCount == 0) {
      ATH_MSG_ERROR("Could not find scale factor matching eta & IBLOF status!"
                    << "\nRun: " << runNumber << ", |eta| = " << absEta
                    << ", IBLOF: " << stored_numberOfIBLOverflowsdEdx);
    return -1.;
    }
    
    if (matchCount > 1) {
      ATH_MSG_ERROR("Multiple SFs matched eta & IBLOF status! This should not happen.");
      return -1.;
    }

    ATH_MSG_DEBUG("Found SF " << SF << " for track with |eta|=" << absEta << ", IBLOF=" << hasIBLOF);
    return SF;
  }

  
  ////////////////////////
  /// Cluster Level EQ ///
  ////////////////////////

  double PixelDEdxEqualizationTool::getClusterdEdxSF(const PixelDEdx::PixelClusterStruct& cluster, const int runNumber) const {

    // Get the cached vector of SF records for this run
    std::shared_ptr<std::vector<ClusterSFRecord>> sfRecordsPtr = getRunClusterSFs(runNumber);

    if (!sfRecordsPtr || sfRecordsPtr->empty()) {
        return -1.0;
    }

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

    // Find matching SF record
    double SF = -1.;
    int matchCount = 0;
    for (const auto& rec : *sfRecordsPtr) {
      if (rec.bec == sfBECBin && rec.layerID == cluster.layer && rec.etaM == sfEtaBin) {
        ++matchCount;
        if (matchCount == 1) {
          SF = rec.SF;  // return the scale factor found
        }
      }
    }

    if (matchCount == 0) {
      ATH_MSG_ERROR("No matching SF record found for cluster (bec=" << sfBECBin <<
                    ", layer=" << cluster.layer << ", etaM=" << sfEtaBin << "). Returning SF = -1.0.");
      return -1.;
    }
    
    if (matchCount > 1) {
      ATH_MSG_ERROR("Multiple SF records found for cluster (bec=" << sfBECBin <<
                    ", layer=" << cluster.layer << ", etaM=" << sfEtaBin << "). Returning SF = -1.0.");
      return -1.;
    }
    
    ATH_MSG_DEBUG("Found SF = " << SF << " for cluster (bec=" << sfBECBin << ", layer=" << cluster.layer << ", etaM=" << sfEtaBin << ")");
    return SF;
  }

} // namespace CP
