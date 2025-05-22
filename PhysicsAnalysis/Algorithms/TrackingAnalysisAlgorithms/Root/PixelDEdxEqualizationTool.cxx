#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationTool.h"

namespace {

  // Some functions

  // Accessors

} // namespace

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
      //filename = PathResolverFindCalibFile( m_sfFileName );
      ATH_MSG_ERROR("PathResolver not availabnle in AnalysisBase?"); //FIXME!
      return StatusCode::FAILURE;
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

  //////////////////
  /// Get SF DF ///
  /////////////////

  std::shared_ptr<ROOT::RDF::RNode> PixelDEdxEqualizationTool::getFilteredSFDF(const int runNumber) const {
    /// Find closest run in SF RDF & filter to keep just these.
    /// Don't know the runNumber until execute, so trying to make this efficient.
    /// Cache the filtered RDF for this run in the map.
    int closestRunNumber = 0;
    std::shared_ptr<ROOT::RDF::RNode> filtered_df;

    /// First, try to find it (lock the map while accessing)
    {
      std::lock_guard<std::mutex> lock(m_mapMutex);
      //std::shared_lock lock(m_mapMutex);
      
      auto it = m_filteredRDFMap.find(runNumber);
      if (it != m_filteredRDFMap.end()) {
        ATH_MSG_DEBUG("SFs for this run " << runNumber << " already cached!");
        filtered_df = it->second;
      }
    }
    /// If not already cached, filter the dataframe and cache it now.
    if (!filtered_df) {
      ATH_MSG_INFO("SFs for run " << runNumber << " are NOT already cached.  Will filter RDF and cache now.");
      //auto df = std::make_shared<ROOT::RDataFrame>(*m_df);
      //auto runNumbers = df->Take<int>("runNumber");
      auto runNumbers = m_df->Take<int>("runNumber");
      closestRunNumber = *std::min_element(runNumbers.begin(), runNumbers.end(),
                                           [runNumber](int a, int b) {
                                             return std::abs(a - runNumber) < std::abs(b - runNumber); });
      ATH_MSG_INFO("Closest run number:" << closestRunNumber);
      //auto filtered = std::make_shared<ROOT::RDF::RNode>(df->Filter([closestRunNumber](int run) { return run == closestRunNumber; }, {"runNumber"}));
      std::string expr = "runNumber == " + std::to_string(closestRunNumber);
      auto filtered = std::make_shared<ROOT::RDF::RNode>(m_df->Filter(expr));

      /// Store it in the map (lock again)
      {
        std::lock_guard<std::mutex> lock(m_mapMutex);
        //std::unique_lock lock(m_mapMutex);
        m_filteredRDFMap[runNumber] = filtered;
      }
      filtered_df = filtered;
    }
    return filtered_df;
  }

  //////////////////////
  /// Track Level EQ ///
  //////////////////////
  
  double PixelDEdxEqualizationTool::getTrackdEdxSF(const xAOD::TrackParticle& track, const int runNumber) const {

    unsigned char stored_numberOfIBLOverflowsdEdx = 99;
    static const SG::AuxElement::ConstAccessor< unsigned char > nIBLOFAcc("numberOfIBLOverflowsdEdx");
    if (nIBLOFAcc.isAvailable(track)) {
      stored_numberOfIBLOverflowsdEdx = nIBLOFAcc(track);
    } else {
      ATH_MSG_WARNING("numberOfIBLOverflowsdEdx auxdata is missing!");
    }

    /// Get filtered RDataFrame
    auto filtered_df = getFilteredSFDF(runNumber);

    /// Determine which SF eta bin to use.  SFs take advantage of phi & +-z symmetry.
    /// Only derive SFs for |eta| < 2.5.
    /// If |eta|>2.5, apply last SF.
    double absEta = abs(track.eta());
    //if(absEta > 2.5) { // FIXME instead of hardcoding, maybe check if absEta larger than highest etaHigh...
    //  absEta = 2.49;
    //}
    if(absEta > m_maxEta) { // just use highest bin.  
      absEta = m_maxEta - 0.001;
    }
    auto result = filtered_df->Filter(
                                      [absEta](double etaLow, double etaHigh) {
                                        return etaLow <= absEta && absEta <= etaHigh;
                                      },
                                      {"etaLow", "etaHigh"}
                                      );
    /*
    auto SF_values = ( (int) stored_numberOfIBLOverflowsdEdx > 0) ? result.Take<double>("SF_IBLOFYes") : result.Take<double>("SF_IBLOFNo");
    if (SF_values->empty()) {
      ATH_MSG_ERROR("Could not find the scale factor matching the eta & IBLOF status of this track!"
                    << "\nRun: " << runNumber << ", |eta| = " << absEta << ", IBLOF: " << stored_numberOfIBLOverflowsdEdx
                    << "\nCannot equalize track dE/dx.");
      return -1.;
    }
    if (SF_values->size()>1) {
      ATH_MSG_ERROR("Found multiple scale factors matching the eta & IBLOF of this track!"
                    << "\nCannot equalize track dE/dx.");
      return -1.;
    }
    
    double SF = SF_values->at(0);
    ATH_MSG_DEBUG("Test: found SF " << SF << " for this track.");
    
    return SF;
    */
    const std::string sf_column = (static_cast<int>(stored_numberOfIBLOverflowsdEdx) > 0) ? "SF_IBLOFYes" : "SF_IBLOFNo";

    double SF = -1.;
    int matchCount = 0;

    result.Foreach([&](double val) {
      ++matchCount;
      if (matchCount == 1) {
        SF = val;
      }
    }, {sf_column});
    
    if (matchCount == 0) {
      ATH_MSG_ERROR("Could not find the scale factor matching the eta & IBLOF status of this track!"
                    << "\nRun: " << runNumber << ", |eta| = " << absEta << ", IBLOF: " << stored_numberOfIBLOverflowsdEdx
                    << "\nCannot equalize track dE/dx.");
      return -1.;
}
    if (matchCount > 1) {
      ATH_MSG_ERROR("Found multiple scale factors matching the eta & IBLOF of this track!"
                    << "\nCannot equalize track dE/dx.");
      return -1.;
    }
    
    ATH_MSG_DEBUG("Test: found SF " << SF << " for this track.");
    return SF;

  }
  ////////////////////////
  /// Cluster Level EQ ///
  ////////////////////////

  double PixelDEdxEqualizationTool::getClusterdEdxSF(const PixelDEdx::PixelClusterStruct& cluster, const int runNumber) const {

    /// Get filtered RDataFrame
    auto filtered_df = getFilteredSFDF(runNumber);

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
    /*
    auto SF_values = result.Take<double>("SF");
    auto SF_error_values = result.Take<double>("SF_error");
    if (SF_values->empty() || SF_error_values->empty()) {
      ATH_MSG_ERROR("Could not find the scale factor matching the (bec, layer, module eta) of this pixel cluster!"
                    << "\nCannot equalize cluster dE/dx.");
      return -1.;
    }
    if (SF_values->size()>1 || SF_error_values->size()>1) {
      ATH_MSG_ERROR("Found multiple scale factors matching the (bec, layer, module eta) of this pixel cluster!"
                    << "\nCannot equalize cluster dE/dx.");
      return -1.;
    }

    double SF = SF_values->at(0);
    double SF_error = SF_error_values->at(0);
    ATH_MSG_DEBUG("Test: found SF " << SF << " with error " << SF_error << " for this pixel cluster.");
    
    /// Apply scale factor and store
    return SF;
    */
    double SF = -1.;
    double SF_error = -1.;
    int matchCount = 0;

    result.Foreach([&](double sf, double sfErr) {
      ++matchCount;
      if (matchCount == 1) {
        SF = sf;
        SF_error = sfErr;
      }
    }, {"SF", "SF_error"});

    if (matchCount == 0) {
      ATH_MSG_ERROR("Could not find the scale factor matching the (bec, layer, module eta) of this pixel cluster!"
                    << "\nCannot equalize cluster dE/dx.");
      return -1.;
    }
    if (matchCount > 1) {
      ATH_MSG_ERROR("Found multiple scale factors matching the (bec, layer, module eta) of this pixel cluster!"
                    << "\nCannot equalize cluster dE/dx.");
      return -1.;
    }

    ATH_MSG_DEBUG("Test: found SF " << SF << " with error " << SF_error << " for this pixel cluster.");

    return SF;
  }

} // namespace CP
