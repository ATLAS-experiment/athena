//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationTool.h"

namespace CP {
  
  PixelDEdxEqualizationTool::PixelDEdxEqualizationTool(const std::string& tool_name)
    : asg::AsgTool(tool_name),
      m_filename(""),
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

    /// Get name of ROOT file with SF trees.
    /// By default, read SFs from trees stored in ASG calibration area.
    /// Users can override by providing a local file.
    if (m_sfLocalFileName != "") {
      m_filename = m_sfLocalFileName;
      ATH_MSG_WARNING("!! SETTING UP WITH USER SPECIFIED INPUT LOCATION \"" << m_sfLocalFileName << "\"!! FOR DEVELOPMENT USE ONLY !! ");
    }
    else {
      m_filename = PathResolverFindCalibFile( m_sfFileName );
      /// Make sure PathResolverFindCalibFile found the file
      if (m_filename.empty()) {
        ATH_MSG_ERROR("Could not find SF file: " << m_filename);
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Using default calibration file from ASG area:" << m_filename);
    }

    return StatusCode::SUCCESS;
  }
  
  ////////////////////////////
  /// Get SF from this run ///
  ////////////////////////////

   // Template helper function for common logic
  template<typename RecordType>
  const std::vector<RecordType>& PixelDEdxEqualizationTool::getRunSFs(
                                                                      const int runNumber,
                                                                      std::map<int, std::vector<RecordType>>& cache,
                                                                      const std::string& treeName) const {
    
    // Use a shared lock for read-only access
    {
        std::shared_lock readLock(m_cacheMutex);
        auto it = cache.find(runNumber);
        if (it != cache.end()) {
            ATH_MSG_DEBUG("SF data for run " << runNumber << " already cached!");
            return it->second;
        }
    } // release shared lock for reading cache.

    // Not already cached, prepare to find it in the dataframe
    ATH_MSG_INFO("SF data for run " << runNumber << " not cached. Will filter and cache now."); 

    // Open the SF tree file
    auto file = std::unique_ptr<TFile>(TFile::Open(m_filename.c_str(), "READ"));
    if (!file || file->IsZombie()) {
        ATH_MSG_ERROR("Failed to open ROOT file.");
        {
            std::unique_lock writeLock(m_cacheMutex);
            auto [it, _] = cache.emplace(runNumber, std::vector<RecordType>{});
            return it->second;
        }
    }

    ROOT::RDataFrame df(treeName.c_str(), file.get());

    // Extract run numbers
    auto runNumbers = df.Take<int>("runNumber");

    // Get closest run number
    int closestRunNumber = *std::min_element(runNumbers.begin(), runNumbers.end(),
                                             [runNumber](int a, int b) {
                                                 return std::abs(a - runNumber) < std::abs(b - runNumber);
                                             });

    // Handle closest run number logic (similar as before)
    if(runNumber != closestRunNumber) {
      if (runNumber == 284500 || runNumber == 300000 || runNumber == 310000 || //MC20
          runNumber == 410000 || runNumber == 450000 || runNumber == 470000 || runNumber == 495000) { //MC23
        ATH_MSG_WARNING("Could not find SFs for this MC sub-campaign!");
        {
          std::unique_lock writeLock(m_cacheMutex);
          auto [it, _] = cache.emplace(runNumber, std::vector<RecordType>{});
          return it->second;
        }
      } else {
        ATH_MSG_WARNING("Using closest run: " << closestRunNumber);
      }
    }

    // Filter the dataframe and extract records using the provided callback
    std::string expr = "runNumber == " + std::to_string(closestRunNumber);
    auto filtered = df.Filter(expr);
    
    // Initialize records
    std::vector<RecordType> records;

    // Fill records to trigger evaluation
    if constexpr (std::is_same<RecordType, TrackSFRecord>::value) {
      auto etaLows = filtered.Take<double>("etaLow");
      auto etaHighs = filtered.Take<double>("etaHigh");
      auto sfYes = filtered.Take<double>("SF_IBLOFYes");
      auto sfNo = filtered.Take<double>("SF_IBLOFNo");
      records.reserve(etaLows->size());
      for (size_t i = 0; i < etaLows->size(); ++i) {
        records.emplace_back(TrackSFRecord{
            etaLows->at(i),
            etaHighs->at(i),
            sfYes->at(i),
            sfNo->at(i)
          });
      }
    }
    else if constexpr (std::is_same<RecordType, ClusterSFRecord>::value) {
      auto becs = filtered.Take<int>("bec");
      auto layers = filtered.Take<int>("layerID");
      auto etas = filtered.Take<int>("etaM");
      auto sfs = filtered.Take<double>("SF");
      auto sf_errors = filtered.Take<double>("SF_error");

      // Build vector of SFRecords
      records.reserve(becs->size());
      for (size_t i = 0; i < becs->size(); ++i) {
        records.emplace_back(ClusterSFRecord{
            becs->at(i), layers->at(i), etas->at(i), sfs->at(i), sf_errors->at(i)
          });
      }
    }

    // Cache the results
    {
        std::unique_lock writeLock(m_cacheMutex);
        // check if another thread beat us.  If so, do not overwrite.
        auto [it, inserted] = cache.try_emplace(runNumber, std::move(records));
        return it->second;
    }
  }

  // Specific implementation for TrackSFs
  const std::vector<TrackSFRecord>& PixelDEdxEqualizationTool::getRunTrackSFs(const int runNumber) const {
    return getRunSFs<TrackSFRecord>(runNumber, m_cachedTrackSFData, m_trackSFTreeName.value());
  }

  // Specific implementation for ClusterSFs
  const std::vector<ClusterSFRecord>& PixelDEdxEqualizationTool::getRunClusterSFs(const int runNumber) const {
    return getRunSFs<ClusterSFRecord>(runNumber, m_cachedClusterSFData, m_clusterSFTreeName.value());
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
    const auto& sfRecords = getRunTrackSFs(runNumber);
    if (sfRecords.empty()) {
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
    
    for (const TrackSFRecord& rec : sfRecords) {
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
    const auto& sfRecords = getRunClusterSFs(runNumber);

    if (sfRecords.empty()) {
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
    for (const auto& rec : sfRecords) {
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
