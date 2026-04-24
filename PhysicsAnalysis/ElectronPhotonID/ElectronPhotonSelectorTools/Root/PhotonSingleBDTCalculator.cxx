/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronPhotonSelectorTools/PhotonSingleBDTCalculator.h"

#include "PathResolver/PathResolver.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODEgamma/Photon.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "TFile.h"
#include "TTree.h"

#include "MVAUtils/BDT.h"

#include <algorithm>
#include <cmath>

namespace PhotonIDBDT {

PhotonSingleBDTCalculator::PhotonSingleBDTCalculator(const std::string& name)
  : asg::AsgTool(name) {}

PhotonSingleBDTCalculator::~PhotonSingleBDTCalculator() = default;

StatusCode PhotonSingleBDTCalculator::initialize() {
  ATH_CHECK(loadBDT());
  return StatusCode::SUCCESS;
}

StatusCode PhotonSingleBDTCalculator::loadBDT() {
  if (m_modelFile.empty()) {
    ATH_MSG_ERROR("ModelFile property is empty.");
    return StatusCode::FAILURE;
  }

  const std::string resolved = PathResolver::find_calib_file(m_modelFile);
  if (resolved.empty()) {
    ATH_MSG_ERROR("Could not resolve model file: " << m_modelFile);
    return StatusCode::FAILURE;
  }

  m_file.reset(TFile::Open(resolved.c_str(), "READ"));
  if (!m_file || m_file->IsZombie()) {
    ATH_MSG_ERROR("Failed to open model file: " << resolved);
    return StatusCode::FAILURE;
  }

  m_tree = dynamic_cast<TTree*>(m_file->Get(m_bdtTreeName.value().c_str()));
  if (!m_tree) {
    ATH_MSG_ERROR("Could not find TTree '" << m_bdtTreeName << "' in file " << resolved);
    return StatusCode::FAILURE;
  }
  m_tree->SetCacheSize(0);
  m_bdt = std::make_unique<MVAUtils::BDT>(m_tree);

  ATH_MSG_DEBUG("Loaded BDT from " << resolved << " tree=" << m_bdtTreeName);
  return StatusCode::SUCCESS;
}

StatusCode PhotonSingleBDTCalculator::computeScore(const std::vector<float>& vars, float& score) const {
  if (!m_bdt) {
    ATH_MSG_ERROR("BDT not loaded (nullptr).");
    return StatusCode::FAILURE;
  }

  // Classification score
  score = m_bdt->GetClassification(vars);

  return StatusCode::SUCCESS;
}

} // namespace PhotonIDBDT