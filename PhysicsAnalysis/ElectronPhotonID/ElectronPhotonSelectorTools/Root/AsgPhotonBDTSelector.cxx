/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronPhotonSelectorTools/AsgPhotonBDTSelector.h"

#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/ReadDecorHandle.h"

#include "TEnv.h"

#include "EGSelectorConfigurationMapping.h"
#include "EgammaAnalysisHelpers/AsgEGammaConfigHelper.h"
#include "xAODCaloEvent/CaloCluster.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace {

  enum PhotonBDTIsEMBits : unsigned int {
    // Eta Out of Range
    FailOutOfRange      = 1u << 0, // 1
  
    // Fail preselections
    FailPreselectionF1     = 1u << 1, // 2
    FailPreselectionE277   = 1u << 2, // 4
  
    // Fail BDT score
    FailBDTScore           = 1u << 3, // 8
  
    // Cannot compute score or the score is missing
    FailMissingScore       = 1u << 4 // 16
  };

} // end of anonymous namespace

namespace PhotonIDBDT {

//=============================================================================
// Initialise the tool: load config and retrieve BDT calculator
//=============================================================================
StatusCode AsgPhotonBDTSelector::initialize() {
  // Load the configuration file and parse it
  ATH_CHECK(loadConfig());
  // Register the cuts in the AcceptInfo
  m_cutPosHasScore          = m_acceptInfo.addCut("HasScore", "Photon has BDT score decoration");
  m_cutPosPreF1             = m_acceptInfo.addCut("PreselectionF1", "Photon passes preselection on f1");
  m_cutPosPreE277           = m_acceptInfo.addCut("PreselectionE277", "Photon passes preselection on e277");
  m_cutPosPassPreselection  = m_acceptInfo.addCut("PassPreselection", "Photon passes all preselections");
  m_cutPosInRange           = m_acceptInfo.addCut("InRange", "Photon kinematics within eta range and binned in Et");
  m_cutPosScore             = m_acceptInfo.addCut("BDTScore", "Passes the BDT score cut");
  // Check if it went well
  if (m_cutPosScore < 0 || m_cutPosInRange < 0 || m_cutPosHasScore < 0 ||
      m_cutPosPreF1 < 0 || m_cutPosPreE277 < 0 || m_cutPosPassPreselection < 0) {
    ATH_MSG_ERROR("Failed to register cuts in AcceptInfo");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_ContainerName.initialize());
  ATH_CHECK(m_decoratorScore.initialize());

#ifndef XAOD_STANDALONE
  if(m_suppressInputDeps){
    // The user has promised that this will be produced by the same alg.
    // Tell the scheduler to ignore it to avoid circular dependencies.
    renounce(m_decoratorScore);
  }
#endif

  return StatusCode::SUCCESS;
}

//=============================================================================
// Load and parse the configuration file
//=============================================================================
StatusCode AsgPhotonBDTSelector::loadConfig() {
  // If we specified the WP, look for the corresponding config file in the mapping
  if (!m_workingPoint.empty()) {
    m_configFile = AsgConfigHelper::findConfigFile(
      m_workingPoint, EgammaSelectors::PhotonBDTPointToConfFile
    );
    ATH_MSG_INFO("Photon ID BDT working point: " << getOperatingPointName());
  }

  if (m_configFile.empty()) {
    ATH_MSG_ERROR("Empty configFile. WorkingPoint: " << m_workingPoint);
    return StatusCode::FAILURE;
  }

  const std::string configFile = PathResolverFindCalibFile(m_configFile);
  if (configFile.empty()) {
    ATH_MSG_ERROR("Could not locate config via PathResolver: " << m_configFile);
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Using config file: " << m_configFile << " (resolved: " << configFile << ")");

  // Parse config file
  TEnv env;
  env.ReadFile(configFile.c_str(), kEnvLocal);

  // Load WP binning
  m_etaBins    = AsgConfigHelper::HelperFloat("CutBinEta",    env);
  m_etBinsGeV  = AsgConfigHelper::HelperFloat("CutBinEtGeV",  env);
  // Load preselection cuts on f1 and e277 variables
  m_cutF1Conv    = AsgConfigHelper::HelperFloat("CutF1Conv", env);
  m_cutF1Unconv  = AsgConfigHelper::HelperFloat("CutF1Unconv", env);
  m_cutE277Conv  = AsgConfigHelper::HelperFloat("CutE277Conv", env);
  m_cutE277Unconv= AsgConfigHelper::HelperFloat("CutE277Unconv", env);
  // Load BDT score cuts
  m_cutConv    = AsgConfigHelper::HelperFloat("BDTCutConv",   env);
  m_cutUnconv  = AsgConfigHelper::HelperFloat("BDTCutUnconv", env);

  // Validate binning
  const unsigned nEta = (m_etaBins.size() >= 2) ? (m_etaBins.size() - 1) : 0;
  const unsigned nEt  = (m_etBinsGeV.size() >= 2) ? (m_etBinsGeV.size() - 1) : 0;

  if (nEta == 0 || nEt == 0) {
    ATH_MSG_ERROR("Need at least 2 edges for eta and Et binning.");
    return StatusCode::FAILURE;
  }

  const unsigned nExpected = nEta * nEt;
  if (m_cutConv.size() != nExpected || m_cutUnconv.size() != nExpected) {
    ATH_MSG_ERROR("Size mismatch between eta and Et binning and BDT cut maps: expected " << nExpected
                  << " (= " << nEta << "*" << nEt << ")"
                  << " got BDTCutConv=" << m_cutConv.size()
                  << " BDTCutUnconv=" << m_cutUnconv.size());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Return the name of the operating point 
//=============================================================================
std::string AsgPhotonBDTSelector::getOperatingPointName() const
{
  return m_workingPoint;
}

//=============================================================================
// Return accept info object describing the cuts
//=============================================================================
const asg::AcceptInfo& AsgPhotonBDTSelector::getAcceptInfo() const {
  return m_acceptInfo;
}

//=============================================================================
// Accept and execute methods
//=============================================================================

asg::AcceptData AsgPhotonBDTSelector::accept(const xAOD::IParticle* part) const
{
  return accept(Gaudi::Hive::currentContext(), part);
}

asg::AcceptData AsgPhotonBDTSelector::accept(const EventContext& ctx,
                                             const xAOD::IParticle* part) const
{
  if (!part) return makeReject(m_acceptInfo);

  if (const auto* ph = dynamic_cast<const xAOD::Photon*>(part)) {
    return accept(ctx, ph);
  }
  if (const auto* eg = dynamic_cast<const xAOD::Egamma*>(part)) {
    return accept(ctx, eg);
  }
  return makeReject(m_acceptInfo);
}

asg::AcceptData AsgPhotonBDTSelector::accept(const EventContext& ctx,
                                             const xAOD::Egamma* eg) const
{
  if (!eg) return makeReject(m_acceptInfo);

  const auto* ph = dynamic_cast<const xAOD::Photon*>(eg);
  if (!ph) return makeReject(m_acceptInfo);

  return accept(ctx, ph);
}

asg::AcceptData AsgPhotonBDTSelector::accept(const EventContext& ctx,
                                             const xAOD::Photon* ph) const
{
  if (!ph) return makeReject(m_acceptInfo);
  return acceptBDT(ctx, *ph, nullptr);
}

asg::AcceptData AsgPhotonBDTSelector::accept(const EventContext&,
                                             const xAOD::Electron*) const
{
  // This tool is photon-only
  return makeReject(m_acceptInfo);
}


StatusCode AsgPhotonBDTSelector::execute(const EventContext& ctx,
					 const xAOD::Egamma* eg,
					 unsigned int& isEM) const
{
  isEM = 0u;
  if (!eg) return StatusCode::SUCCESS;;

  const auto* ph = dynamic_cast<const xAOD::Photon*>(eg);
  if (!ph) {
    isEM = 1u; // or define a bit for wrong type
    return StatusCode::SUCCESS;
  }

  (void) acceptBDT(ctx, *ph, &isEM);
  return StatusCode::SUCCESS;
}

//=============================================================================
// Helpers for cut applications
//=============================================================================
bool AsgPhotonBDTSelector::isConverted(const xAOD::Photon& ph) const {
  return xAOD::EgammaHelpers::isConvertedPhoton(&ph, m_excludeTRT);
}


bool AsgPhotonBDTSelector::findBin(const float absEta, const float etGeV,
                                   size_t& iEta, size_t& iEt) const {
  // bins defined as [edge_i, edge_{i+1})
  // Eta binning
  auto itEta = std::upper_bound(m_etaBins.begin(), m_etaBins.end(), absEta);
  if (itEta == m_etaBins.begin() || itEta == m_etaBins.end()) return false; // Eta out of range
  iEta = (itEta - m_etaBins.begin()) - 1; // regular bin

  // ET binning
  auto itEt  = std::upper_bound(m_etBinsGeV.begin(), m_etBinsGeV.end(), etGeV);
  if (itEt == m_etBinsGeV.begin()) { iEt = 0; } // underflow: first bin
  else if (itEt == m_etBinsGeV.end()) { iEt = m_etBinsGeV.size() - 2; } // overflow: last bin
  else { iEt = (itEt - m_etBinsGeV.begin()) - 1; } // regular bin
  
  return true;
}

float AsgPhotonBDTSelector::getCut(const bool converted, const size_t iEta, const size_t iEt) const {
  const size_t nEta = m_etaBins.size() - 1;
  const size_t idx = iEt * nEta + iEta;
  const float cut = converted ? m_cutConv.at(idx) : m_cutUnconv.at(idx);
  return cut;
}

asg::AcceptData AsgPhotonBDTSelector::makeReject(const asg::AcceptInfo& info) {
  asg::AcceptData acc(&info);
  for (unsigned i = 0; i < info.getNCuts(); ++i) acc.setCutResult(i, false);
  return acc;
}

float AsgPhotonBDTSelector::getShowerShape(const xAOD::Photon& ph, xAOD::EgammaParameters::ShowerShapeType t, const char *name) const {
  float out = 0.f;
  if (!ph.showerShapeValue(out, t)) {
    ATH_MSG_ERROR("AsgPhotonBDTSelector: missing shower shape variable '" << name);
    // Fail loudly
    throw std::runtime_error(std::string("AsgPhotonBDTSelector: missing shower shape ") + name);
  }
  return out;
}

//=============================================================================
// Accept method: apply cuts and return accept data
//=============================================================================
asg::AcceptData AsgPhotonBDTSelector::acceptBDT(const EventContext& ctx, const xAOD::Photon& ph, unsigned int* isEM) const {
  // Helper for isEM word
  auto setBit = [&](unsigned int bit) {
    if (isEM) *isEM |= bit;
  };
  // I assume that the photon exists and is valid

  // start to retrieve the acceptor
  // Start with all cuts failed 
  asg::AcceptData acc = makeReject(m_acceptInfo);

  // Ok now we assume that we have the score
  SG::ReadDecorHandle<xAOD::EgammaContainer, float> decoratorScore{m_decoratorScore, ctx};
  const float score = decoratorScore(ph);
  acc.setCutResult(m_cutPosHasScore, true);

  // Now we check the photon kinematics from cluster and the binning
  const xAOD::CaloCluster* cluster = ph.caloCluster();
  if (!cluster) {
    setBit(FailOutOfRange);
    return acc;
  }
  const float absEta = std::abs(cluster->eta());
  const float etGeV  = cluster->pt() * 1e-3f;

  size_t iEta=0, iEt=0;
  if (!findBin(absEta, etGeV, iEta, iEt)) {
    setBit(FailOutOfRange); // failOutOfRange
    return acc;
  }
  // If we are here, the photon is in the correct eta range
  acc.setCutResult(m_cutPosInRange, true);

  // check if the photon is converted
  const bool conv = isConverted(ph);

  // Check F1 and e277 preselection cuts
  bool passF1 = false, passE277 = false, passPre = false;
  // Before trying to access the shower shape variables, we check if they are available. 
  // If not, we can either fail or reapply the WP based on the score and isEM word (if enabled and available)
  if (m_reapplyWPIfNoShowerShapes) {
    float tmp = 0.f;
    const bool hasF1 = ph.showerShapeValue(tmp, xAOD::EgammaParameters::f1);
    const bool hasE277 = ph.showerShapeValue(tmp, xAOD::EgammaParameters::e277);
    if (!hasF1 || !hasE277) {
      // Check if isEM decoration is available
      const SG::AuxElement::Accessor<int> accIsEM(m_isEMDecoration);
      if (accIsEM.isAvailable(ph)) {
        const int previousIsEM = accIsEM(ph);
        passF1 = !(previousIsEM & FailPreselectionF1);
        passE277 = !(previousIsEM & FailPreselectionE277);
        passPre = passF1 && passE277;
      }
      else {
        ATH_MSG_ERROR("Missing f1 and e277 shower shapes and isEM decoration, cannot reapply WP. Rejecting photon.");
        acc.setCutResult(m_cutPosPreF1, false);
        acc.setCutResult(m_cutPosPreE277, false);
        acc.setCutResult(m_cutPosPassPreselection, false);
        setBit(FailPreselectionF1);
        setBit(FailPreselectionE277);
        return acc;
      }
    }
  }
  else {
    // If we are missing shower shapes and we are not reapplying the WP, we throw an error
    float f1 = 0.f, e277 = 0.f;
    f1 = getShowerShape(ph, xAOD::EgammaParameters::f1, "f1");
    e277 = getShowerShape(ph, xAOD::EgammaParameters::e277, "e277");

    const float cutF1   = conv ? m_cutF1Conv.at(0)   : m_cutF1Unconv.at(0);
    const float cutE277 = conv ? m_cutE277Conv.at(0) : m_cutE277Unconv.at(0); 
    passF1   = (f1   > cutF1);
    passE277 = (e277 >  cutE277);
    passPre = passF1 && passE277;
  }
  
  // Decorate the accept data with the results of the preselection cuts
  acc.setCutResult(m_cutPosPreF1, passF1);
  acc.setCutResult(m_cutPosPreE277, passE277);
  acc.setCutResult(m_cutPosPassPreselection, passPre);

  // Set bits for failed preselections
  if (!passF1) setBit(FailPreselectionF1);
  if (!passE277) setBit(FailPreselectionE277);
  // If failed preselection, reject and return
  if (!passPre) return acc;

  // Check the BDT score cut
  const float cut = getCut(conv, iEta, iEt);
  const bool passBDT = (score > cut);
  acc.setCutResult(m_cutPosScore, (score > cut));
  if (!passBDT) setBit(FailBDTScore);

  return acc;
}

} // namespace PhotonIDBDT
