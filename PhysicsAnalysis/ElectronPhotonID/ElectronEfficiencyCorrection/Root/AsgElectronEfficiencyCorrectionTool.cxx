/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

/**
  @class AthElectronEfficiencyCorrectionTool
  @brief Calculate the egamma scale factors
  */

// Include this class's header
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "ElectronEfficiencyCorrection/ElRecomFileHelpers.h"
// Library includes
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <string>
#include <utility>
// xAOD includes
#include "xAODEgamma/Electron.h"
#include "xAODEventInfo/EventInfo.h"
// PAT includes
#ifndef XAOD_STANDALONE
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif
#include "AthContainers/AuxElement.h"
#include "PATCore/PATCoreEnums.h"
#include "PathResolver/PathResolver.h"
#include "xAODMetaData/FileMetaData.h"
// ROOT includes
#include "TH2F.h"
#include "TSystem.h"

namespace {
// Helper function Calculate and return at the spot
CP::CorrectionCode
HelperFunc(double& sf, const double input)
{
  sf = sf + input;
  return CP::CorrectionCode::Ok;
}
}

namespace correlationModel {
enum model
{
  COMBMCTOYS = 0,
  MCTOYS = 1,
  FULL = 2,
  SIMPLIFIED = 3,
  TOTAL = 4,
  SYST = 5
};
}
AsgElectronEfficiencyCorrectionTool::AsgElectronEfficiencyCorrectionTool(
  const std::string& myname)
  : asg::AsgMetadataTool(myname)
  , m_affectedSys()
  , m_appliedSystematics(nullptr)
  , m_correlation_model(correlationModel::SIMPLIFIED)
  , m_sysSubstring("")
  , m_dataType(PATCore::ParticleDataType::Full)
  , m_nCorrSyst(0)
  , m_nUncorrSyst(0)
  , m_UncorrRegions(nullptr)
  , m_nSimpleUncorrSyst(0)
  , m_toysBasename{}
  , m_corrVarUp{}
  , m_corrVarDown{}
  , m_uncorrVarUp{}
  , m_uncorrVarDown{}
  , m_accessors{std::make_unique<Accessors>(*this)}
{
  // Create an instance of the underlying ROOT tool
  m_rootTool = std::make_unique<Root::TElectronEfficiencyCorrectionTool>(
      ("T" + (this->name())).c_str());
  // Declare the needed properties
  declareProperty(
    "CorrectionFileNameList",
    m_corrFileNameList,
    "List of file names that store the correction factors for simulation.");
  declareProperty("MapFilePath",
                  m_mapFile = "ElectronEfficiencyCorrection/2015_2025/rel22.2/"
                              "2024_Consolidated_Prerecom_v1/map1.txt",
                  "Full path to the map file");
  declareProperty(
    "RecoKey", m_recoKey = "", "Key associated with reconstruction");
  declareProperty(
    "IdKey", m_idKey = "", "Key associated with identification working point");
  declareProperty(
    "IsoKey", m_isoKey = "", "Key associated with isolation working point");
  declareProperty(
    "TriggerKey", m_trigKey = "", "Key associated with trigger working point");
  declareProperty("ForceDataType",
                  m_dataTypeOverwrite = -1,
                  "Force the DataType of the electron to specified value (to "
                  "circumvent problem of incorrect DataType for forward "
                  "electrons in some old releases)");
  declareProperty(
    "ResultPrefix", m_resultPrefix = "", "The prefix string for the result");
  declareProperty("ResultName", m_resultName = "", "The string for the result");
  declareProperty("CorrelationModel",
                  m_correlation_model_name = "SIMPLIFIED",
                  "Uncertainty correlation model. At the moment TOTAL, FULL, "
                  "SIMPLIFIED, SYST, MCTOYS and COMBMCTOYS are implemented. "
                  "SIMPLIFIED is the default option.");
  declareProperty("NumberOfToys",
                  m_number_of_toys = 100,
                  "Number of ToyMC replica, affecting MCTOYS and COMBMCTOYS "
                  "correlation models only.");
  declareProperty("MCToySeed",
                  m_seed_toys = 0,
                  "Seed for ToyMC replica, affecting MCTOYS and COMBMCTOYS "
                  "correlation models only.");
  declareProperty("MCToyScale",
                  m_scale_toys = 1,
                  "Scales Toy systematics up by this factor, affecting MCTOYS "
                  "and COMBMCTOYS correlation models only.");
  declareProperty(
    "UncorrEtaBinsUser",
    m_uncorrSimplfEtaBinsUser = { 0.0, 1.37, 4.9 },
    "Custom Eta/Pt binning for the SIMPLIFIED correlation model.");
  declareProperty(
    "UncorrEtBinsUser",
    m_uncorrSimplfEtBinsUser = { 4000,
                                 7000,
                                 10000,
                                 15000,
                                 20000,
                                 25000,
                                 30000,
                                 60000,
                                 80000,
                                 13600000 },
    "Custom Eta/Pt binning for the SIMPLIFIED correlation model.");
  declareProperty("EventInfoCollectionName",
                  m_eventInfoCollectionName = "EventInfo",
                  "The EventInfo Collection Name");
  declareProperty("UseRandomRunNumber", m_useRandomRunNumber = true);
  declareProperty("DefaultRandomRunNumber", m_defaultRandomRunNumber = 999999);
}

AsgElectronEfficiencyCorrectionTool::~AsgElectronEfficiencyCorrectionTool()
{
  if (m_UncorrRegions) {
    delete m_UncorrRegions;
  }
}

StatusCode
AsgElectronEfficiencyCorrectionTool::initialize()
{
  // Forward the message level
  m_rootTool->msg().setLevel(this->msg().level());

  if (m_corrFileNameList.empty() && m_recoKey.empty() && m_idKey.empty() &&
      m_trigKey.empty() && m_isoKey.empty()) {
    ATH_MSG_ERROR("CorrectionFileNameList as well as SFKeys are empty! Please "
                  "configure it properly...");
    return StatusCode::FAILURE;
  }
  /*
   * When metadata are available
   * m_dataType will be whatever the metadata says i.e Full or Fast
   * Its default value is Full.
   * The user can overwrite all these ,using a flag,
   * and force a specific dataType
   */
  if (m_dataTypeOverwrite != -1) {
    if (m_dataTypeOverwrite !=
          static_cast<int>(PATCore::ParticleDataType::Full) &&
        m_dataTypeOverwrite !=
          static_cast<int>(PATCore::ParticleDataType::Fast)) {
      ATH_MSG_ERROR("Unsupported Particle Data Type Overwrite"
                    << m_dataTypeOverwrite);
      return StatusCode::FAILURE;
    }
    m_dataType =
      static_cast<PATCore::ParticleDataType::DataType>(m_dataTypeOverwrite);
  }
  // Find the relevant input files
  // Fill the vector with filename using keys if the user
  // has not passed the full filename as a property
  if (m_corrFileNameList.empty()) {
    if (getFile(m_recoKey, m_idKey, m_isoKey, m_trigKey).isFailure()) {
      ATH_MSG_ERROR("No Root file input specified, and not available map file");
      return StatusCode::FAILURE;
    }
  }
  // Resolve the paths to the input files for the full Geant4 simualtion
  // corrections
  for (auto& ifile : m_corrFileNameList) {

    std::string filename = PathResolverFindCalibFile(ifile);
    if (filename.empty()) {
      ATH_MSG_ERROR("Could NOT resolve file name " << ifile);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG(" Path found = " << filename);
    }
    m_rootTool->addFileName(filename);
    // Determine the systematics substring according to the name of the input
    // file
    if (ifile.find("efficiencySF.") != std::string::npos) {
      m_sysSubstring = "Trigger_";
    }
    if (ifile.find("efficiencySF.offline") != std::string::npos) {
      m_sysSubstring = "ID_";
    }
    if (ifile.find("efficiencySF.offline.RecoTrk") != std::string::npos) {
      m_sysSubstring = "Reco_";
    }
    if (ifile.find("efficiencySF.offline.Fwd") != std::string::npos) {
      m_sysSubstring = "FwdID_";
    }
    if (ifile.find("efficiencySF.Isolation") != std::string::npos) {
      m_sysSubstring = "Iso_";
    }
    if (ifile.find("efficiency.") != std::string::npos) {
      m_sysSubstring = "TriggerEff_";
    }
    if (ifile.find("efficiencySF.ChargeID") != std::string::npos) {
      m_sysSubstring = "ChargeIDSel_";
    }
    if (m_sysSubstring.empty()) {
      ATH_MSG_ERROR("Could NOT find systematics Substring file name "
                    << m_sysSubstring);
      return StatusCode::FAILURE;
    }
  }
  //
  // Find the proper correlation Model
  if (m_correlation_model_name == "COMBMCTOYS") {
    m_correlation_model = correlationModel::COMBMCTOYS;
  } else if (m_correlation_model_name == "MCTOYS") {
    m_correlation_model = correlationModel::MCTOYS;
  } else if (m_correlation_model_name == "FULL") {
    m_correlation_model = correlationModel::FULL;
  } else if (m_correlation_model_name == "SIMPLIFIED") {
    m_correlation_model = correlationModel::SIMPLIFIED;
    // a few checks on the binning that the user might have specified
    if (m_uncorrSimplfEtaBinsUser.empty() || m_uncorrSimplfEtBinsUser.empty() ||
        m_uncorrSimplfEtBinsUser.size() < 2 ||
        m_uncorrSimplfEtaBinsUser.size() < 2) {
      ATH_MSG_ERROR("Something went wrong when specifying bins for the "
                    "SIMPLIFIED correlation model ");
      return StatusCode::FAILURE;
    }
  } else if (m_correlation_model_name == "TOTAL") {
    m_correlation_model = correlationModel::TOTAL;
  } else if (m_correlation_model_name == "SYST") {
    m_correlation_model = correlationModel::SYST;
  } else {
    ATH_MSG_ERROR("Unknown correlation model " + m_correlation_model_name);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Correlation model: " + m_correlation_model_name
                << " Enum " << m_correlation_model);

  // Histogram of simplified uncorrelated regions
  if (m_correlation_model == correlationModel::SIMPLIFIED) {
    m_UncorrRegions = new TH2F("UncorrRegions",
                               "UncorrRegions",
                               m_uncorrSimplfEtBinsUser.size() - 1,
                               &(m_uncorrSimplfEtBinsUser[0]),
                               m_uncorrSimplfEtaBinsUser.size() - 1,
                               &(m_uncorrSimplfEtaBinsUser[0]));
    m_UncorrRegions->SetDirectory(nullptr);

    // bins not entries here
    m_nSimpleUncorrSyst = (m_uncorrSimplfEtaBinsUser.size() - 1) *
                          (m_uncorrSimplfEtBinsUser.size() - 1);
  }
  // Finish the preaparation of the underlying tool
  if (m_seed_toys != 0) {
    m_rootTool->setSeed(m_seed_toys);
  }
  //
  if (m_correlation_model == correlationModel::COMBMCTOYS) {
    m_rootTool->bookCombToyMCScaleFactors(m_number_of_toys);
  }
  //
  if (m_correlation_model == correlationModel::MCTOYS) {
    m_rootTool->bookToyMCScaleFactors(m_number_of_toys);
  }
  // We need to initialize the underlying ROOT TSelectorTool
  if (0 == m_rootTool->initialize()) {
    ATH_MSG_ERROR(
      "Could not initialize the TElectronEfficiencyCorrectionTool!");
    return StatusCode::FAILURE;
  }

  // get Nsyst
  m_nCorrSyst = m_rootTool->getNSyst();
  std::map<float, std::vector<float>> tmp;
  m_nUncorrSyst = m_rootTool->getNbins(tmp);
  // copy over to vector for better access
  for (const auto& i : tmp) {
    m_pteta_bins.emplace_back(i.first, i.second);
  }

  // Check if the input contains uncorr/stat necessary
  // to run non-total systematic model
  if (m_correlation_model != correlationModel::TOTAL &&
      m_rootTool->uncorrEmpty(m_dataType) ) {
    ATH_MSG_ERROR("Called correlation model "+m_correlation_model_name+
                  " but input does not contain necessary histograms.");
    return StatusCode::FAILURE;
  }

  // Initialize the systematics
  if (InitSystematics() != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("(InitSystematics() != StatusCode::SUCCESS)");
    return StatusCode::FAILURE;
  }
  // Add the recommended systematics to the registry
  if (registerSystematics() != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("(registerSystematics() != StatusCode::SUCCESS)");
    return StatusCode::FAILURE;
  }
  // Configure for nominal systematics
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Could not configure for nominal settings");
    return StatusCode::FAILURE;
  }

  if (m_useRandomRunNumber) {
    resetAccessor (m_accessors->randomrunnumber, *this, "RandomRunNumber");
  }
  ATH_CHECK (initializeColumns());
  return StatusCode::SUCCESS;
}

CP::CorrectionCode
AsgElectronEfficiencyCorrectionTool::getEfficiencyScaleFactor(
  const xAOD::Electron& inputObject,
  double& efficiencyScaleFactor) const
{
  const xAOD::EventInfo* eventInfo = nullptr;
  if (evtStore()->retrieve(eventInfo, m_eventInfoCollectionName).isFailure()) {
    ATH_MSG_ERROR("Could not retrieve EventInfo object!");
    return CP::CorrectionCode::Error;
  }
  return getEfficiencyScaleFactor(columnar::ElectronId(inputObject), efficiencyScaleFactor, columnar::EventInfoId (*eventInfo));
}

CP::CorrectionCode
AsgElectronEfficiencyCorrectionTool::getEfficiencyScaleFactor(
  columnar::ElectronId inputObject,
  double& efficiencyScaleFactor,
  columnar::EventInfoId eventInfo) const
{
  const Accessors& acc = *m_accessors;

  efficiencyScaleFactor = 1;
  // Retrieve the proper random Run Number
  unsigned int runNumber = m_defaultRandomRunNumber;
  if (m_useRandomRunNumber) {
    if (!acc.randomrunnumber.isAvailable(eventInfo)) {
      ATH_MSG_WARNING(
        "Pileup tool not run before using ElectronEfficiencyTool! SFs do not "
        "reflect PU distribution in data");
      return CP::CorrectionCode::Error;
    }
    runNumber = acc.randomrunnumber(eventInfo);
  }
  //
  // Get the result
  //
  double cluster_eta(-9999.9);

  auto cluster = acc.caloClusterAcc (inputObject) [0].value();

  // we need to use different variables for central and forward electrons
  if (acc.accAuthor.isAvailable(inputObject) &&
      acc.accAuthor(inputObject) == xAOD::EgammaParameters::AuthorFwdElectron) {
    cluster_eta = acc.clusterEtaAcc (cluster);
  } else {
    cluster_eta = acc.clusterEtaBEAcc (cluster, 2);
  }

  // use et from cluster because it is immutable under syst variations of
  // electron energy scale
  const double energy = acc.clusterEAcc(cluster);
  const double parEta = acc.m_eta(inputObject);
  const double coshEta = std::cosh(parEta);
  double et = (coshEta != 0.) ? energy / coshEta : 0.;
  // allow for a 5% margin at the lowest pT bin boundary (i.e. increase et by 5%
  // for sub-threshold electrons). This assures that electrons that pass the
  // threshold only under syst variations of energy get a scale factor assigned.
  auto itr_pt = m_pteta_bins.begin();
  if (itr_pt != m_pteta_bins.end() && et < itr_pt->first) {
    et = et * 1.05;
  }
  return getEfficiencyScaleFactor(et, cluster_eta, runNumber,
                                  efficiencyScaleFactor);
}

CP::CorrectionCode
AsgElectronEfficiencyCorrectionTool::getEfficiencyScaleFactor(
    const double et,  /*in MeV*/
    const double cluster_eta, /*cluster*/
    const unsigned int runNumber,
    double& efficiencyScaleFactor) const
{
  // We pass only one variation per time
  // The applied systematic is always one systematic.
  // Either is relevant and acquires a value
  // or stays 0.
  double sys(0);

  // Let's try to figure already what we are to do
  bool doSFOnly = appliedSystematics().empty();
  bool doToys = (m_correlation_model == correlationModel::MCTOYS ||
                 m_correlation_model == correlationModel::COMBMCTOYS);
  bool isTotal = (m_correlation_model == correlationModel::TOTAL);
  bool isFull = (m_correlation_model == correlationModel::FULL);
  bool isSimplified = (m_correlation_model == correlationModel::SIMPLIFIED);

  // Lets see if we have an uncorrelated syst variation passed
  int unCorrSign = 0;
  // or a correlated one
  int indexCorrelated = -999;
  int correlatedSign = 0;
  if (!(doSFOnly || doToys || isTotal) && (isFull || isSimplified)) {
    const auto& sysName = appliedSystematics().begin()->name();
    bool isUncorr = (sysName.find("UncorrUnc") != std::string::npos);
    int currentUncorReg = -999;
    if (isUncorr) {
      // Can we find an uncorrelated region?
      if (isFull) {
        currentUncorReg = currentUncorrSystRegion(cluster_eta, et);
      } else if (isSimplified) {
        currentUncorReg = currentSimplifiedUncorrSystRegion(cluster_eta, et);
      }
      if (currentUncorReg < 0) {
        return CP::CorrectionCode::OutOfValidityRange;
      }
      // And  use it to if we got the "right" syst variation
      if (appliedSystematics().matchSystematic(
              m_uncorrVarDown[currentUncorReg])) {
        unCorrSign = -1;
      } else if (appliedSystematics().matchSystematic(
                     m_uncorrVarUp[currentUncorReg])) {
        unCorrSign = 1;
      }
    } else if (m_nCorrSyst != 0) {//if we have 0 we do not care ...
      if (sysName.find("CorrUnc") != std::string::npos) {
        // given the name convention we
        const auto varNumEnd = sysName.rfind("__");
        const auto varNumBegin = sysName.rfind("NP") + 2;
        const int varIndex =
            std::stoi(sysName.substr(varNumBegin, (varNumEnd - varNumBegin)));
        if (appliedSystematics().matchSystematic(m_corrVarUp[varIndex])) {
          indexCorrelated = varIndex;
          correlatedSign = 1;
        } else if (appliedSystematics().matchSystematic(
                       m_corrVarDown[varIndex])) {
          indexCorrelated = varIndex;
          correlatedSign = -1;
        }
      }  // find CorrUncertainty in name
    } // not Uncorr and we have CorrSyst
  } // Not (SF or toys or total)

  // Now lets do the call
  // For now we more or less calculate on demand only
  // the Correlated and the toys we can see if we want
  // top opt also the "TOTAL"
  Root::TElectronEfficiencyCorrectionTool::Result result;
  const int status =
      m_rootTool->calculate(m_dataType, runNumber, cluster_eta, et, /* in MeV */
                            result, isTotal);

  // if status 0 something went wrong
  if (!status) {
    efficiencyScaleFactor = 1;
    return CP::CorrectionCode::OutOfValidityRange;
  }
  // At this point we have the SF
  efficiencyScaleFactor = result.SF;
  //And if all we need we can return
  if (doSFOnly) {
    return CP::CorrectionCode::Ok;
  }

  // First the logic if the user requested toys
  if (doToys) {
    auto toy = appliedSystematics().getToyVariationByBaseName(m_toysBasename);
    toy.second = m_scale_toys;
    sys = result.toys[toy.first - 1] * m_scale_toys;
    // return here for Toy variations
    efficiencyScaleFactor = sys;
    return CP::CorrectionCode::Ok;
  }
  // The "TOTAL" single uncertainty uncorrelated+correlated
  else if (isTotal) {
    sys = result.Total;
    if (appliedSystematics().matchSystematic(m_uncorrVarUp[0])) {
      return HelperFunc(efficiencyScaleFactor, sys);
    }
    if (appliedSystematics().matchSystematic(m_uncorrVarDown[0])) {
      return HelperFunc(efficiencyScaleFactor, -1 * sys);
    }
  }
  // Then the uncorrelated part for the SiMPLIFIED/FULL models
  else if (unCorrSign!=0) {
    sys = unCorrSign * result.UnCorr;
    return HelperFunc(efficiencyScaleFactor, sys);
  }

  // If we reach this point
  // it means we need to do the correlated part
  // for the FULL/SIMPLIFIED models.
  // First if there are not correlated systematic
  if (m_nCorrSyst == 0) {

    if (appliedSystematics().matchSystematic(m_corrVarUp[0])) {
      sys = std::sqrt(result.Total * result.Total -
                      result.UnCorr * result.UnCorr);  // total
                                                       // -stat
      return HelperFunc(efficiencyScaleFactor, sys);
    }
    if (appliedSystematics().matchSystematic(m_corrVarDown[0])) {
      sys = -1 * std::sqrt(result.Total * result.Total -
                           result.UnCorr * result.UnCorr);  // total
                                                            // -stat
      return HelperFunc(efficiencyScaleFactor, sys);
    }
  }
 //or if we had
 if (correlatedSign != 0) {
    sys = correlatedSign * result.Corr[indexCorrelated];
    return HelperFunc(efficiencyScaleFactor, sys);
  }
  return CP::CorrectionCode::Ok;
}

CP::CorrectionCode
AsgElectronEfficiencyCorrectionTool::applyEfficiencyScaleFactor(
  const xAOD::Electron& inputObject) const
{
  double efficiencyScaleFactor = 1.0;
  CP::CorrectionCode result =
    getEfficiencyScaleFactor(inputObject, efficiencyScaleFactor);
  const static SG::AuxElement::Decorator<float> dec(m_resultPrefix +
                                                    m_resultName + "SF");
  dec(inputObject) = efficiencyScaleFactor;
  return result;
}

/*
 * Systematics Interface
 */
/// returns: bool indicating if affected by the variation
bool
AsgElectronEfficiencyCorrectionTool::isAffectedBySystematic(
  const CP::SystematicVariation& systematic) const
{
  if (systematic.empty()) {
    return false;
  }
  CP::SystematicSet sys = affectingSystematics();
  if (m_correlation_model == correlationModel::MCTOYS ||
      m_correlation_model == correlationModel::COMBMCTOYS) {
    return (sys.begin()->ensembleContains(systematic));
  } else {
    return (sys.find(systematic) != sys.end());
  }
}
/// returns: the list of all systematics this tool can be affected by
CP::SystematicSet
AsgElectronEfficiencyCorrectionTool::affectingSystematics() const
{
  return m_affectedSys;
}
// Register the systematics with the registry and add them to the recommended
// list
StatusCode
AsgElectronEfficiencyCorrectionTool::registerSystematics()
{
  CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();

  if (registry.registerSystematics(*this) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR(
      "Failed to add systematic to list of recommended systematics.");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
/// returns: the list of all systematics this tool recommends to use
CP::SystematicSet
AsgElectronEfficiencyCorrectionTool::recommendedSystematics() const
{
  return affectingSystematics();
}
/// Apply one variation at a time
StatusCode
AsgElectronEfficiencyCorrectionTool::applySystematicVariation(
  const CP::SystematicSet& systConfig)
{
  // First, check if this configuration exists in the filtered map/registy
  auto itr = m_systFilter.find(systConfig);

  if (itr != m_systFilter.end()) {
    CP::SystematicSet& mySysConf = itr->second;
    m_appliedSystematics = &mySysConf;
  }
  // if not, we should register it, after it passes sanity checks
  else {
    // If it's a new input set, we need to filter it
    CP::SystematicSet affectingSys = affectingSystematics();
    CP::SystematicSet filteredSys;
    if (!CP::SystematicSet::filterForAffectingSystematics(
          systConfig, affectingSys, filteredSys)) {
      ATH_MSG_ERROR(
        "Unsupported combination of systematic variations passed to the tool!");
      return StatusCode::FAILURE;
    }
    // Does filtered make sense,  only one per time
    if (filteredSys.size() > 1) {
      ATH_MSG_ERROR(
        "More than one systematic variation passed at the same time");
      return StatusCode::FAILURE;
    }

    if (filteredSys.empty() && !systConfig.empty()) {
      ATH_MSG_DEBUG("systematics : ");
      for (const auto& syst : systConfig) {
        ATH_MSG_DEBUG(syst.name());
      }
      ATH_MSG_DEBUG(" Not supported ");
    }
    // insert to the registy
    itr = m_systFilter.insert(std::make_pair(systConfig, filteredSys)).first;
    // And return directly
    CP::SystematicSet& mySysConf = itr->second;
    m_appliedSystematics = &mySysConf;
  }
  return StatusCode::SUCCESS;
}

/*
 * Helper Methods
 */
StatusCode
AsgElectronEfficiencyCorrectionTool::InitSystematics()
{
  const std::string prefix = "EL_EFF_" + m_sysSubstring;
  const std::string prefixUncorr = prefix + m_correlation_model_name + "_";
  // Toys
  if (m_correlation_model == correlationModel::COMBMCTOYS) {
    m_toysBasename = prefix + "COMBMCTOY";
    m_affectedSys.insert(
      (CP::SystematicVariation::makeToyEnsemble(m_toysBasename)));
  } else if (m_correlation_model == correlationModel::MCTOYS) {
    m_toysBasename = prefix + "MCTOY";
    m_affectedSys.insert(
      (CP::SystematicVariation::makeToyEnsemble(m_toysBasename)));
    // Correlated for the different models (1 or Full set)
  } else if (m_correlation_model != correlationModel::TOTAL) {
    if (m_nCorrSyst == 0) {
      auto varUp = CP::SystematicVariation(prefix + "CorrUncertainty", 1);
      auto varDown = CP::SystematicVariation(prefix + "CorrUncertainty", -1);
      m_corrVarUp.push_back(varUp);
      m_corrVarDown.push_back(varDown);
      m_affectedSys.insert(varUp);
      m_affectedSys.insert(varDown);
    } else
      for (int i = 0; i < m_nCorrSyst; ++i) {
        auto varUp =
          CP::SystematicVariation(prefix + Form("CorrUncertaintyNP%d", i), 1);
        auto varDown =
          CP::SystematicVariation(prefix + Form("CorrUncertaintyNP%d", i), -1);
        m_corrVarUp.push_back(varUp);
        m_corrVarDown.push_back(varDown);
        m_affectedSys.insert(varUp);
        m_affectedSys.insert(varDown);
      }
  }
  // Different tratement for the uncorrelated per model
  if (m_correlation_model == correlationModel::TOTAL) {
    auto varUp = CP::SystematicVariation(prefixUncorr + "1NPCOR_PLUS_UNCOR", 1);
    auto varDown =
      CP::SystematicVariation(prefixUncorr + "1NPCOR_PLUS_UNCOR", -1);
    m_uncorrVarUp.push_back(varUp);
    m_uncorrVarDown.push_back(varDown);
    m_affectedSys.insert(varUp);
    m_affectedSys.insert(varDown);
  } else if (m_correlation_model == correlationModel::FULL) {
    for (int i = 0; i < m_nUncorrSyst; ++i) {
      auto varUp = CP::SystematicVariation(
        prefixUncorr + Form("UncorrUncertaintyNP%d", i), 1);
      auto varDown = CP::SystematicVariation(
        prefixUncorr + Form("UncorrUncertaintyNP%d", i), -1);
      m_uncorrVarUp.push_back(varUp);
      m_uncorrVarDown.push_back(varDown);
      m_affectedSys.insert(varUp);
      m_affectedSys.insert(varDown);
    }
  } else if (m_correlation_model == correlationModel::SIMPLIFIED) {
    for (int i = 0; i < m_nSimpleUncorrSyst; ++i) {
      auto varUp = CP::SystematicVariation(
        prefixUncorr + Form("UncorrUncertaintyNP%d", i), 1);
      auto varDown = CP::SystematicVariation(
        prefixUncorr + Form("UncorrUncertaintyNP%d", i), -1);
      m_uncorrVarUp.push_back(varUp);
      m_uncorrVarDown.push_back(varDown);
      m_affectedSys.insert(varUp);
      m_affectedSys.insert(varDown);
    }
  }
  return StatusCode::SUCCESS;
}

int
AsgElectronEfficiencyCorrectionTool::currentSimplifiedUncorrSystRegion(
  const double cluster_eta,
  const double et) const
{
  int ptbin = std::as_const(*m_UncorrRegions).GetXaxis()->FindBin(et) - 1;
  if (ptbin < 0 ||
      ptbin >= std::as_const(*m_UncorrRegions).GetXaxis()->GetNbins()) {
    ATH_MSG_WARNING(
      " Found electron with Et = "
      << et / 1000. << " GeV, where you specified boundaries of ["
      << std::as_const(*m_UncorrRegions).GetXaxis()->GetBinLowEdge(1) << ","
      << std::as_const(*m_UncorrRegions)
           .GetXaxis()
           ->GetBinUpEdge(
             std::as_const(*m_UncorrRegions).GetXaxis()->GetNbins())
      << "] for the SIMPLIFIED correlation model ");
    return -1;
  }
  int etabin =
    std::as_const(*m_UncorrRegions).GetYaxis()->FindBin(std::abs(cluster_eta)) -
    1;
  if (etabin < 0 ||
      etabin >= std::as_const(*m_UncorrRegions).GetYaxis()->GetNbins()) {
    ATH_MSG_WARNING(
      " Found electron with |eta| = "
      << std::abs(cluster_eta) << ", where you specified boundaries of ["
      << std::as_const(*m_UncorrRegions).GetYaxis()->GetBinLowEdge(1) << ","
      << std::as_const(*m_UncorrRegions)
           .GetYaxis()
           ->GetBinUpEdge(
             std::as_const(*m_UncorrRegions).GetYaxis()->GetNbins())
      << "] for the SIMPLIFIED correlation model ");
    return -1;
  }
  int reg = ((etabin)*m_UncorrRegions->GetNbinsX() + ptbin);
  return reg;
}

int
AsgElectronEfficiencyCorrectionTool::currentUncorrSystRegion(
  const double cluster_eta,
  const double et) const
{
  int etabin = 0;
  int reg = 0;
  bool found = false;
  float cluster_eta_electron = 0;
  auto itr_ptBEGIN = m_pteta_bins.begin();
  auto itr_ptEND = m_pteta_bins.end();
  for (; itr_ptBEGIN != itr_ptEND; ++itr_ptBEGIN) {
    auto itr_ptBEGINplusOne = itr_ptBEGIN;
    ++itr_ptBEGINplusOne;
    // Find the pt bin : Larger or equal from the current and the next one is
    // the last or the next one is larger.
    if (et >= itr_ptBEGIN->first &&
        (itr_ptBEGINplusOne == itr_ptEND || et < itr_ptBEGINplusOne->first)) {
      if ((itr_ptBEGIN->second).at(0) >= 0) {
        cluster_eta_electron = std::abs(cluster_eta);
      } else {
        cluster_eta_electron = (cluster_eta);
      };
      for (unsigned int etab = 0; etab < ((itr_ptBEGIN->second).size());
           ++etab) {
        unsigned int etabnext = etab + 1;
        // Find the eta bin : Larger or equal from the current and the next one
        // is the last or the next one is larger:.
        if ((cluster_eta_electron) >= (itr_ptBEGIN->second).at(etab) &&
            (etabnext == itr_ptBEGIN->second.size() ||
             cluster_eta_electron < itr_ptBEGIN->second.at(etabnext))) {
          found = true;
          break;
        }
        // We did not find it. Increment eta and continue looking
        etabin++;
      }
    }
    if (found) {
      break;
    }
    // Add the full size of the "passed" eta row
    reg += (itr_ptBEGIN->second).size();
  }
  if (!found) {
    ATH_MSG_WARNING("No index for the uncorrelated systematic was found, "
                    "returning the maximum index");
    return m_nCorrSyst;
  }
  reg = reg + etabin;
  return reg;
}

int
AsgElectronEfficiencyCorrectionTool::systUncorrVariationIndex(
  const xAOD::Electron& inputObject) const
{
  return systUncorrVariationIndex (columnar::ElectronId (inputObject));
}

int
AsgElectronEfficiencyCorrectionTool::systUncorrVariationIndex(
  columnar::ElectronId inputObject) const
{
  const Accessors& acc = *m_accessors;
  int currentSystRegion = -999;
  double cluster_eta(-9999.9);
  double et(0.0);

  et = acc.m_pt(inputObject);
  const auto cluster = acc.caloClusterAcc (inputObject) [0].value();
  cluster_eta = acc.clusterEtaBEAcc (cluster, 2);
  switch (m_correlation_model) {
    case correlationModel::SIMPLIFIED: {
      currentSystRegion = currentSimplifiedUncorrSystRegion(cluster_eta, et);
      break;
    }
    case correlationModel::FULL: {
      currentSystRegion = currentUncorrSystRegion(cluster_eta, et);
      break;
    }
    default: {
      // not there for the other models
      break;
    }
  }
  return currentSystRegion;
}

/// Gets the correction filename from map
StatusCode
AsgElectronEfficiencyCorrectionTool::getFile(const std::string& recokey,
                                             const std::string& idkey,
                                             const std::string& isokey,
                                             const std::string& trigkey)
{

  std::string mapFileName = PathResolverFindCalibFile(m_mapFile);
  std::string key =
    ElRecomFileHelpers::convertToOneKey(recokey, idkey, isokey, trigkey);
  std::string value = ElRecomFileHelpers::getValueByKey(mapFileName, key);

  if (!value.empty()) {
    m_corrFileNameList.push_back(value);
  } else {
    if (mapFileName.empty()) {
      ATH_MSG_ERROR(
        "Map file does not exist, Please set the path and version properly..");
    } else {
      ATH_MSG_ERROR(
        "Key "
        << key
        << " does not exist in the map file, Please configure it properly..");
    }
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Full File Name is " + value);
  return StatusCode::SUCCESS;
}
/*
 * Metadata methods.
 * The tool operates in MC .
 * The default type is Full sim.
 *
 * The user can  override  the MC type
 * in which case we avoid doing anything.
 *
 * The typical scenario is :
 * For every IncidentType::BeginInputFile we can check for metadata
 * and try to employ them in determining the simulation Flavor.
 * If we found metadata
 * set m_metadata_retrieved= True else set it to False
 *
 * EndInputFile should not do something
 *
 * The beginEvent should kick in only if the beginInputFile has
 * failed to find metadata and the user has no preference.
 * Asg/base class has our back in cases where the 1st beginEvent
 * happens before the 1st beginInputFile.
 * For now we can not do something meaningfull in this method
 * for this tool so can stay as a skeleton for the future
 */
StatusCode
AsgElectronEfficiencyCorrectionTool::beginInputFile()
{

  // User forced a particular dataType
  if (m_dataTypeOverwrite != -1)
    return StatusCode::SUCCESS;

  PATCore::ParticleDataType::DataType dataType_metadata;
  const StatusCode status = get_simType_from_metadata(dataType_metadata);
  // Metadata got retrieved
  if (status == StatusCode::SUCCESS) {
    m_metadata_retrieved = true;
    ATH_MSG_DEBUG("metadata from new file: "
                  << (dataType_metadata == PATCore::ParticleDataType::Data
                        ? "data"
                        : (dataType_metadata == PATCore::ParticleDataType::Full
                             ? "full simulation"
                             : "fast simulation")));

    if (dataType_metadata != PATCore::ParticleDataType::Data) {
      if (m_dataTypeOverwrite == -1) {
        m_dataType = dataType_metadata;
      } else {
        ATH_MSG_DEBUG(
          "Applying SF corrections to data while they make sense only for MC");
      }
    }
  } else { // not able to retrieve metadata
    m_metadata_retrieved = false;
    ATH_MSG_DEBUG("not able to retrieve metadata");
  }
  return StatusCode::SUCCESS;
}

StatusCode
AsgElectronEfficiencyCorrectionTool::beginEvent()
{

  if (m_dataTypeOverwrite != -1)
    return StatusCode::SUCCESS;
  if (m_metadata_retrieved)
    return StatusCode::SUCCESS;

  m_metadata_retrieved = true;
  return StatusCode::SUCCESS;
}

StatusCode
AsgElectronEfficiencyCorrectionTool::get_simType_from_metadata(
  PATCore::ParticleDataType::DataType& result) const
{

#ifndef XAOD_STANDALONE
  // Determine MC/Data
  std::string dataType("");
  if ((AthAnalysisHelper::retrieveMetadata(
         "/TagInfo", "project_name", dataType, inputMetaStore()))
        .isSuccess()) {
    if (!(dataType == "IS_SIMULATION")) {
      result = PATCore::ParticleDataType::Data;
      ATH_MSG_DEBUG("Running on data");
      return StatusCode::SUCCESS;
    }
    // Determine Fast/FullSim
    if (dataType == "IS_SIMULATION") {
      std::string simType("");
      ATH_CHECK(AthAnalysisHelper::retrieveMetadata("/Simulation/Parameters",
                                                    "SimulationFlavour",
                                                    simType,
                                                    inputMetaStore()));
      std::transform(simType.begin(), simType.end(), simType.begin(), ::toupper);
      result = (simType.find("ATLFAST") == std::string::npos)
                 ? PATCore::ParticleDataType::Full
                 : PATCore::ParticleDataType::Fast;
      return StatusCode::SUCCESS;
    }
  }
#endif
  // Here's how things will work dual use, when file metadata is available in
  // files
  if (inputMetaStore()->contains<xAOD::FileMetaData>("FileMetaData")) {
    const xAOD::FileMetaData* fmd = nullptr;
    ATH_CHECK(inputMetaStore()->retrieve(fmd, "FileMetaData"));

    std::string simType("");
    const bool s = fmd->value(xAOD::FileMetaData::simFlavour, simType);
    if (!s) {
      ATH_MSG_DEBUG("no sim flavour from metadata: must be data");
      result = PATCore::ParticleDataType::Data;
      return StatusCode::SUCCESS;
    } else {
      ATH_MSG_DEBUG("sim type = " + simType);
       std::transform(simType.begin(), simType.end(), simType.begin(), ::toupper);
      result = (simType.find("ATLFAST") == std::string::npos)
                 ? PATCore::ParticleDataType::Full
                 : PATCore::ParticleDataType::Fast;
      return StatusCode::SUCCESS;
    }
  } else { // no metadata in the file
    ATH_MSG_DEBUG("no metadata found in the file");
    return StatusCode::FAILURE;
  }
}



void AsgElectronEfficiencyCorrectionTool::callSingleEvent (columnar::ElectronRange electrons, columnar::EventInfoId event) const
{
  const Accessors& acc = *m_accessors;
  for (columnar::ElectronId electron : electrons)
  {
    double sf = 0;
    switch (getEfficiencyScaleFactor(electron, sf, event).code())
    {
      case CP::CorrectionCode::Ok:
        acc.m_sfDec(electron) = sf;
        acc.m_validDec(electron) = true;
        break;
      case CP::CorrectionCode::OutOfValidityRange:
        acc.m_sfDec(electron) = sf;
        acc.m_validDec(electron) = false;
        break;
      default:
        throw std::runtime_error("Error in getEfficiencyScaleFactor");
    }
  }
}

void AsgElectronEfficiencyCorrectionTool::callEvents (columnar::EventContextRange events) const
{
  const Accessors& acc = *m_accessors;
  for (columnar::EventContextId event : events)
  {
    auto eventInfo = acc.m_eventInfo(event);
    callSingleEvent (acc.m_electrons(event), eventInfo);
  }
}
