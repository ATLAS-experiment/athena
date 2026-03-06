/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronPhotonSelectorTools/PhotonBDTCalculator.h"
#include "xAODEgamma/Photon.h"
#include <limits>

namespace {
  // Internal helper function
  inline StatusCode getShowerShape(const xAOD::Photon& ph,
                                            xAOD::EgammaParameters::ShowerShapeType t,
                                            float& out)
  {
    if (!ph.showerShapeValue(out, t)) { return StatusCode::FAILURE; }
    return StatusCode::SUCCESS;
  }
} // end anonymous namespace

namespace PhotonIDBDT {

PhotonBDTCalculator::PhotonBDTCalculator(const std::string& name)
  : asg::AsgTool(name)
{}

PhotonBDTCalculator::~PhotonBDTCalculator() = default;

StatusCode PhotonBDTCalculator::initialize() {
  // Base tools for BDT evaluation
  ATH_CHECK(m_toolConv.retrieve());
  ATH_CHECK(m_toolUnconv.retrieve());

  return StatusCode::SUCCESS;
}

// Helpers to compute input variables for converted  photons
StatusCode PhotonBDTCalculator::fillVariablesConv(const xAOD::Photon& ph, std::vector<float>& vars) const {
    vars.clear();
    vars.reserve(m_reserveVarsConv.value());
    // Get photon kinematics
    const float eta = ph.eta();
    const float ptGeV = ph.pt() * 1e-3f; // convert to GeV
    const float ptGeV_capped = std::min(ptGeV, 700.f); // Cap pt at 700 GeV
    // Get shower shape variables
    float reta = 0.f, rphi = 0.f, weta2 = 0.f, fracs1 = 0.f, weta1 = 0.f, wtots1 = 0.f, rhad = 0.f, rhad1 = 0.f, eratio = 0.f, deltaE = 0.f;
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Reta,   reta));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Rphi,   rphi));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::weta2,  weta2));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::fracs1, fracs1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::weta1,  weta1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::wtots1, wtots1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Rhad,   rhad));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Rhad1,  rhad1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Eratio, eratio));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::DeltaE, deltaE));
    // Fill variables in the order expected by the BDT tool
    vars.push_back(ptGeV_capped);
    vars.push_back(eta);
    vars.push_back(reta);
    vars.push_back(rphi);
    vars.push_back(weta2);
    vars.push_back(fracs1);
    vars.push_back(weta1);
    vars.push_back(wtots1);
    vars.push_back(rhad);
    vars.push_back(rhad1);
    vars.push_back(eratio);
    vars.push_back(deltaE);
    // The end!
    return StatusCode::SUCCESS;
}

StatusCode PhotonBDTCalculator::fillVariablesUnconv(const xAOD::Photon& ph, std::vector<float>& vars) const {
    vars.clear();
    vars.reserve(m_reserveVarsUnconv.value());
    // Get photon kinematics
    const float eta = ph.eta();
    const float ptGeV = ph.pt() * 1e-3f; // convert to GeV
    const float ptGeV_capped = std::min(ptGeV, 700.f); // Cap pt at 700 GeV
    // Get shower shape variables
    float reta = 0.f, rphi = 0.f, weta2 = 0.f, fracs1 = 0.f, weta1 = 0.f, wtots1 = 0.f, rhad = 0.f, rhad1 = 0.f, eratio = 0.f, deltaE = 0.f;
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Reta,   reta));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Rphi,   rphi));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::weta2,  weta2));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::fracs1, fracs1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::weta1,  weta1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::wtots1, wtots1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Rhad,   rhad));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Rhad1,  rhad1));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::Eratio, eratio));
    ATH_CHECK(getShowerShape(ph, xAOD::EgammaParameters::DeltaE, deltaE));
    // Fill variables in the order expected by the BDT tool
    vars.push_back(ptGeV_capped);
    vars.push_back(eta);
    vars.push_back(reta);
    vars.push_back(rphi);
    vars.push_back(weta2);
    vars.push_back(fracs1);
    vars.push_back(weta1);
    vars.push_back(wtots1);
    vars.push_back(rhad);
    vars.push_back(rhad1);
    vars.push_back(eratio);
    vars.push_back(deltaE);
    // The end!
    return StatusCode::SUCCESS;
}

bool PhotonBDTCalculator::isConverted(const xAOD::Photon& ph) const {
  return xAOD::EgammaHelpers::isConvertedPhoton(&ph, m_excludeTRT.value());
}

StatusCode PhotonBDTCalculator::decorate(const xAOD::Photon& ph) const {
  const SG::AuxElement::Decorator<float> decScore(m_decorationName);
  float score = 0.f;
  ATH_CHECK(getScore(ph, score));
  decScore(ph) = score;
  return StatusCode::SUCCESS;
}

StatusCode PhotonBDTCalculator::getScore(const xAOD::Photon& ph, float& score) const {
  const SG::AuxElement::Accessor<float> accScore(m_decorationName);
  if (!m_forceRecompute && accScore.isAvailable(ph)) {
    score = accScore(ph);
    return StatusCode::SUCCESS;
  }
  std::vector<float> vars;
  if (isConverted(ph)) {
    ATH_CHECK(fillVariablesConv(ph, vars));
    ATH_CHECK(m_toolConv->computeScore(vars, score));
  } else {
    ATH_CHECK(fillVariablesUnconv(ph, vars));
    ATH_CHECK(m_toolUnconv->computeScore(vars, score));
  }
  return StatusCode::SUCCESS;
}

double PhotonBDTCalculator::evaluate(const xAOD::Photon* photon) const
{
  if (!photon) {
    throw std::invalid_argument("PhotonBDTCalculator::evaluate called with nullptr photon");
  }

  float score = 0.f;
  const StatusCode sc = getScore(*photon, score);
  if (sc.isFailure()) {
    // Fails loudly if it cannot be computed
    throw std::runtime_error("PhotonBDTCalculator::evaluate failed to compute BDT score");
  }
  return static_cast<double>(score);
}

} // namespace PhotonIDBDT