/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// $Id: PMGHFProductionFractionTool.cxx

// Systematics include(s):
#include "PathResolver/PathResolver.h"
#include "PATInterfaces/SystematicRegistry.h"

// Local include(s):
#include "PMGTools/PMGHFProductionFractionTool.h"

#include <xAODMetaData/FileMetaData.h>

// C++
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace PMGTools
{
PMGHFProductionFractionTool::PMGHFProductionFractionTool(const std::string &name):
    asg::AsgMetadataTool(name),
    m_calibrationAreaPath(""),
    m_showerGeneratorMap({}),
    m_showerGeneratorMapFile(""),
    m_showerGenerator(""),
    m_charmFilename(""),
    m_bottomFilename(""),
    m_charmProdFractionWeights({}),
    m_bottomProdFractionWeights({})
{
    // Calibration area path
    declareProperty("CalibrationAreaPath", m_calibrationAreaPath = "xAODBTaggingEfficiency/13TeV/HFReweighting-2022-04-19_v1",
                    "Path to the calibration area");

    // MC Shower generator map
    declareProperty("ShowerGeneratorMapFile", m_showerGeneratorMapFile = "HFProductionFractionsMap.txt",
                    "Input file name for the MC shower generator map");

    // MC Shower generator software
    declareProperty("ShowerGenerator", m_showerGenerator = "",
                    "MC Shower generator software");

    // The fiducial charm/bottom pT cut (in MeV)
    declareProperty("FiducialPtCut", m_fiducialPtCut = 5000,
                    "The fiducial charm/bottom pT cut. The recommended value is 5000 (MeV).");

    // The fiducial charm/bottom eta cut
    declareProperty("FiducialEtaCut", m_fiducialEtaCut = 2.5,
                    "The fiducial charm/bottom eta cut. The recommended value is 2.5.");
}

StatusCode PMGHFProductionFractionTool::initialize() {
  // Tell the user what's happening:
  ATH_MSG_INFO("Initializing " << name() << "...");

  // Check for correct settings
  if (m_showerGenerator.empty())
  {
      ATH_MSG_WARNING("The property `ShowerGenerator' was not set!");
      return StatusCode::FAILURE;
  }

  // read input files and calculate weights
  // populates m_charmProdFractionWeights and m_bottomProdFractionWeights
  if (m_showerGenerator != "000000") {
    ANA_CHECK(setupProductionFractions());

    // setup the SystematicsCache object
    m_Parameters.initialize(affectingSystematics(), 
      [this](const CP::SystematicSet &sys, ParameterSet &prod_fracs) {
        return setSystematicVariation(sys, prod_fracs);
      }
    );

    // Register systematics with the registry
    CP::SystematicRegistry &registry = CP::SystematicRegistry::getInstance();
    if (registry.registerSystematics(*this) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Unknown systematic list");
      return StatusCode::FAILURE;
    }

    // print
    printCurrentProdFractions();

  } else {
    ATH_MSG_WARNING("The property `ShowerGenerator' was set to 000000, which means that the tool will return dummy weights of 1.0");
  }

  // Return gracefully
  return StatusCode::SUCCESS;
}

StatusCode PMGHFProductionFractionTool::readProductionFractionsFile(const std::string & filename,  std::map<CP::SystematicVariation, std::map<unsigned int, float>> &weights) {
  /* Example file structure:
      # Charm production fraction from literature (Eur. Phys. J. C (2016) 76:397)
      411 421 431 4000
      NOSYS 0.2404 0.6086 0.0802 0.0623
      PROD_FRAC_CHARM_EIG_1 0.2405 0.6087 0.0770 0.0650
      PROD_FRAC_CHARM_EIG_2 0.2439 0.6108 0.0779 0.0593
      PROD_FRAC_CHARM_EIG_3 0.2461 0.6013 0.0808 0.0632
  */
  std::ifstream infile(filename);
  std::string line;
  int pdg1 = -1, pdg2 = -1, pdg3 = -1, pdg4 = -1;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    if (line.rfind("#", 0) == 0 || line.empty()) {
      continue;
    }
    if (pdg1 < 0) {
      if (!(iss >> pdg1 >> pdg2 >> pdg3 >> pdg4)) {
        ATH_MSG_ERROR("Invalid formatting: " << line);
        return StatusCode::FAILURE;
      }
    } else {
      std::string sys;
      float f1, f2, f3, f4;
      if (!(iss >> sys >> f1 >> f2 >> f3 >> f4)) {
        ATH_MSG_ERROR("Invalid formatting: " << line);
        return StatusCode::FAILURE;
      }
      if (sys == "NOSYS") {
        sys = "";
      }
      weights.insert({CP::SystematicVariation(sys), {{pdg1, f1}, {pdg2, f2}, {pdg3, f3}, {pdg4, f4}}});
    }
  }

  // Close file
  infile.close();

  // return
  return StatusCode::SUCCESS;
}

StatusCode PMGHFProductionFractionTool::setupProductionFractions() {
  // Step 0: find available files
  // ________________________________________________________________

  // Charm production fractions data file
  std::string mapFilename = PathResolverFindCalibFile((fs::path(m_calibrationAreaPath) / fs::path(m_showerGeneratorMapFile)).string());
  if (mapFilename.empty()) {
    ATH_MSG_ERROR("Input file not found " << m_showerGeneratorMapFile);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Found input file " << mapFilename);

  // Find all available MC generators
  std::ifstream infile(mapFilename);
  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    if (line.rfind("#", 0) == 0 || line.empty()) {
      continue;
    }

    std::string id, name;
    if (!(iss >> id >> name)) {
      ATH_MSG_ERROR("Invalid formatting: " << line);
      return StatusCode::FAILURE;
    } if (id == "4") {
      m_charmFilename = std::move(name);
    } else if (id == "5") {
      m_bottomFilename = std::move(name);
    } else {
      m_showerGeneratorMap[id] = std::move(name);
    }
  }

  // Close file
  infile.close();

  // Check if the requested MC shower version is available
  if (m_showerGeneratorMap.find(m_showerGenerator) == m_showerGeneratorMap.end()) {
    ATH_MSG_ERROR("MC shower generator " << m_showerGenerator << " not found!");
    ATH_MSG_INFO("Available generators are:");
    for (auto &gen : m_showerGeneratorMap) {
      ATH_MSG_INFO(gen.first);
    }
    return StatusCode::FAILURE;
  }

  // Step 1: read HF production fractions from literature
  // ________________________________________________________________

  // Charm production fractions data file
  std::string charmFilename = PathResolverFindCalibFile((fs::path(m_calibrationAreaPath) / fs::path(m_charmFilename)).string());
  if (charmFilename.empty()) {
    ATH_MSG_ERROR("Input file not found " << m_charmFilename);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Found input file " << charmFilename);

  // Bottom production fractions data file
  std::string bottomFilename = PathResolverFindCalibFile((fs::path(m_calibrationAreaPath) / fs::path(m_bottomFilename)).string());
  if (bottomFilename.empty()) {
    ATH_MSG_ERROR("Input file not found " << m_bottomFilename);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Found input file " << bottomFilename);

  // Configure the production fractions from files
  ATH_CHECK(readProductionFractionsFile(charmFilename, m_charmProdFractionWeights));
  ATH_CHECK(readProductionFractionsFile(bottomFilename, m_bottomProdFractionWeights));

  // Step 2: read HF production fractions for a given MC shower
  // ________________________________________________________________

  // Read the config files from cvmfs
  std::string filename = PathResolverFindCalibFile((fs::path(m_calibrationAreaPath) / fs::path(m_showerGeneratorMap[m_showerGenerator])).string());
  if (filename.empty()) {
    ATH_MSG_ERROR("Input file not found " << m_showerGeneratorMap[m_showerGenerator]);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Found input file " << filename);

  // Configure the weights from the file
  /* Example file structure:
      #Sherpa(v2.2.1) (DSID 410250)
      #PDG ID | Production Fraction
      511 0.27243
      521 0.273029
      531 0.0893853
      5000 0.365034
      411 0.142884
      421 0.379585
      431 0.113005
      4000 0.364526
  */
  infile = std::ifstream(filename);
  std::map<unsigned int, float> charm;
  std::map<unsigned int, float> bottom;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    if (line.rfind("#", 0) == 0 || line.empty()) {
      continue;
    }

    int pdgId;
    float fraction;
    if (!(iss >> pdgId >> fraction)) {
      ATH_MSG_ERROR("Invalid formatting: " << line);
      return StatusCode::FAILURE;
    }
    if (pdgId == 4000 || pdgId < 500) {
      ATH_MSG_DEBUG("Charm meson " << pdgId << " has a fraction of " << fraction);
      charm[pdgId] = fraction;
    } else {
      ATH_MSG_DEBUG("Bottom meson " << pdgId << " has a fraction of " << fraction);
      bottom[pdgId] = fraction;
    }
  }

  // Close file
  infile.close();

  // Step 3: calculate the weight as `w = f(literature) / f(MC)'
  // ________________________________________________________________

  // charm weights
  for (auto &sys : m_charmProdFractionWeights) {
    for (auto &kv : charm) {
      m_charmProdFractionWeights[sys.first][kv.first] /= kv.second;
    }
  }

  // bottom weights
  for (auto &sys : m_bottomProdFractionWeights) {
    for (auto &kv : bottom) {
      m_bottomProdFractionWeights[sys.first][kv.first] /= kv.second;
    }
  }

  // Return gracefully
  return StatusCode::SUCCESS;
}

bool PMGHFProductionFractionTool::fromBdecay(const xAOD::TruthParticle *particle) const {
  for (unsigned int i = 0; i < particle->nParents(); i++) {
    if (particle->parent(i)->isHeavyHadron()) {
      if (particle->parent(i)->isBottomHadron()) {
        return true;
      } else {
        return fromBdecay(particle->parent(i));
      }
    }
  }
  return false;
}

const xAOD::TruthParticle *PMGHFProductionFractionTool::getInitialParticle(const xAOD::TruthParticle *tp) const {
  if (tp->nParents() < 1) {
    return tp;
  }

  const xAOD::TruthParticle *out = nullptr;
  for (unsigned int i = 0; i < tp->nParents(); i++) {
    if (tp->parent(i) && (tp->parent(i)->absPdgId() == tp->absPdgId())) {
      const xAOD::TruthParticle *initialParticle = getInitialParticle(tp->parent(i));
      if (initialParticle && out && (initialParticle != out)) {
        // this should never happen, but if it does crash the program
        throw std::runtime_error("Contradictory information in the truth decay chain!");
      }
      out = initialParticle;
    }
  }

  if (out) {
    return out;
  }
  return tp;
}

float PMGHFProductionFractionTool::getWeight(const xAOD::TruthParticleContainer *truthParticles, const ParameterSet &prod_fracs) const {
  // Clear vectors
  std::set<const xAOD::TruthParticle *> bottomHadrons;
  std::set<const xAOD::TruthParticle *> charmHadrons;

  // Find truth particles hadrons
  for (const xAOD::TruthParticle *tp : *truthParticles) {
    // if (!(tp->status() == 2 || tp->status() == 1)) {
    //   continue;
    // }
    if (tp->isBottomHadron())
    {
      if (tp->pt() >= m_fiducialPtCut && std::abs(tp->eta()) < m_fiducialEtaCut) {
        bottomHadrons.insert(getInitialParticle(tp));
      }
    }
    if (tp->isCharmHadron()) {
      if (!fromBdecay(getInitialParticle(tp)) && tp->pt() >= m_fiducialPtCut && std::abs(tp->eta()) < m_fiducialEtaCut) {
        charmHadrons.insert(getInitialParticle(tp));
      }
    }
  }

  // The weight
  float weight = 1.0;

  // Calculate the charm weight
  for (auto *tp : charmHadrons) {
    int pdgId = tp->absPdgId();
    float w = 1.0;
    if (tp->isCharmMeson() && prod_fracs.charmWeights.find(pdgId) != prod_fracs.charmWeights.end()) {
      w = prod_fracs.charmWeights.at(pdgId);
    } else if (pdgId == 4122 || // Lambda_c+
               pdgId == 4232 || // Xi_c+
               pdgId == 4132 || // Xi_c0
               pdgId == 4332) { // Omega_c0
      w = prod_fracs.charmWeights.at(4000);
    }
    ATH_MSG_DEBUG("Charm weight for pdgId " << pdgId << ", pT " << tp->pt() << ", eta " << tp->eta() << ": " << w);
    weight *= w;
  }

  // Calculate the bottom weight
  for (auto *tp : bottomHadrons) {
    int pdgId = tp->absPdgId();
    float w = 1.0;
    if (tp->isBottomMeson() && prod_fracs.bottomWeights.find(pdgId) != prod_fracs.bottomWeights.end()) {
      w = prod_fracs.bottomWeights.at(pdgId);
    } else if (pdgId == 5122 || // Lambda_b0
               pdgId == 5132 || // Xi_b-
               pdgId == 5232 || // Xi_b0
               pdgId == 5332) { // Omega_b-
      w = prod_fracs.bottomWeights.at(5000);
    }
    ATH_MSG_DEBUG("Bottom weight for pdgId " << pdgId << ", pT " << tp->pt() << ", eta " << tp->eta() << ": " << w);
    weight *= w;
  }
  return weight;
}

float PMGHFProductionFractionTool::getSysWeight(const xAOD::TruthParticleContainer *truthParticles, const CP::SystematicSet &sys) const {
  if (m_showerGenerator == "000000") { return 1.0; }
  const ParameterSet *prod_fracs = nullptr;
  ANA_CHECK_THROW (m_Parameters.get(sys, prod_fracs));
  return getWeight(truthParticles, *prod_fracs);
}

void PMGHFProductionFractionTool::printCurrentProdFractions() const {
  // MC Shower
  ATH_MSG_INFO("The tool is currently configured to give weights for " << m_showerGenerator);

  // Charm production fraction weights
  ATH_MSG_INFO("Currently set weights for charm production fractions:");
  for (auto const &generator : m_charmProdFractionWeights) {
    ATH_MSG_INFO("sys: " << generator.first.name());
    for (auto const &kv : generator.second) {
      ATH_MSG_INFO(" - " << kv.first << " " << kv.second);
    }
  }

  // Bottom production fraction weights
  ATH_MSG_INFO("Currently set weights for bottom production fractions:");
  for (auto const &generator : m_bottomProdFractionWeights) {
    ATH_MSG_INFO("sys: " << generator.first.name());
    for (auto const &kv : generator.second) {
      ATH_MSG_INFO(" - " << kv.first << " " << kv.second);
    }
  }
}

StatusCode PMGHFProductionFractionTool::setSystematicVariation(const CP::SystematicSet &systConfig, ParameterSet &param) const {
  if (systConfig.size() > 1) {
    ATH_MSG_ERROR("Multiple systematic variations in one SystematicsSet are not supported");
    return StatusCode::FAILURE;
  }

  // set charm and bottom weights to nominal by default
  if (m_charmProdFractionWeights.find(CP::SystematicVariation("")) == m_charmProdFractionWeights.end()) {
    ATH_MSG_ERROR("Nominal charm weights not found");
    return StatusCode::FAILURE;
  }

  if (m_bottomProdFractionWeights.find(CP::SystematicVariation("")) == m_bottomProdFractionWeights.end()) {
    ATH_MSG_ERROR("Nominal bottom weights not found");
    return StatusCode::FAILURE;
  }

  auto charmWeights = m_charmProdFractionWeights.at(CP::SystematicVariation(""));
  auto bottomWeights = m_bottomProdFractionWeights.at(CP::SystematicVariation(""));

  // if a sys variation is passed that affects the weights, choose the appropriate weight set
  if (systConfig.size() == 1) {
    const CP::SystematicVariation &current_variation = *(systConfig.begin());
    if (m_charmProdFractionWeights.find(current_variation) != m_charmProdFractionWeights.end()) {
      charmWeights = m_charmProdFractionWeights.at(current_variation);
    }

    if (m_bottomProdFractionWeights.find(current_variation) != m_bottomProdFractionWeights.end()) {
      bottomWeights = m_bottomProdFractionWeights.at(current_variation);
    }
  }

  // set the ParameterSet (weights set) for use in weight calculation
  param.charmWeights = std::move(charmWeights);
  param.bottomWeights = std::move(bottomWeights);

  return StatusCode::SUCCESS;
}

CP::SystematicSet PMGHFProductionFractionTool::affectingSystematics() const
{
  if (m_charmProdFractionWeights.empty() || m_bottomProdFractionWeights.empty()) {
    ATH_MSG_ERROR("The production fraction weights were not properly initialized!");
    throw std::runtime_error("Charm or bottom production fraction weight map is empty!");
  }
  CP::SystematicSet result;
  for (const auto &kv : m_charmProdFractionWeights) {
    if(!kv.first.empty())
      result.insert(kv.first);
  }
  for (const auto &kv : m_bottomProdFractionWeights) {
    if(!kv.first.empty())
      result.insert(kv.first);
  }

  /// Systematics set of the weight systematics
  return result;
}

} // namespace PMGTools
