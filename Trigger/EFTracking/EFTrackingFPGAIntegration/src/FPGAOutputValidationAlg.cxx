/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "TH1.h"

#include "FPGAOutputValidationAlg.h"

#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

FPGAOutputValidationAlg::FPGAOutputValidationAlg(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode FPGAOutputValidationAlg::initialize() {
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_tHistSvc.retrieve());

  ATH_CHECK(m_pixelKeys.assign(m_pixelNames.value()));
  ATH_CHECK(m_pixelKeys.initialize(!m_pixelNames.empty()));

  ATH_CHECK(m_stripKeys.assign(m_stripNames.value()));
  ATH_CHECK(m_stripKeys.initialize(!m_stripNames.empty()));

  // Todo: Get proper ranges/granularity
  for (SG::ReadHandleKey<xAOD::PixelClusterContainer>& key : m_pixelKeys) {
    const std::string idX = "/FPGAOutputValidation/" + key.key() + "X";
    const std::string nameX = key.key() + "X";
    const std::string titleX = key.key() + " X";
    auto histX = std::make_unique<TH1F>(nameX.c_str(), titleX.c_str(), 100, -300, 300);
    m_pixelXSharedLocks.emplace_back();
    ATH_CHECK(m_tHistSvc->regShared(idX, std::move(histX), m_pixelXSharedLocks.back()));

    const std::string idY = "/FPGAOutputValidation/" + key.key() + "Y";
    const std::string nameY = key.key() + "Y";
    const std::string titleY = key.key() + " Y";
    auto histY = std::make_unique<TH1F>(nameY.c_str(), titleY.c_str(), 100, -300, 300);
    m_pixelYSharedLocks.emplace_back();
    ATH_CHECK(m_tHistSvc->regShared(idY, std::move(histY), m_pixelYSharedLocks.back()));

    const std::string idZ = "/FPGAOutputValidation/" + key.key() + "Z";
    const std::string nameZ = key.key() + "Z";
    const std::string titleZ = key.key() + " Z";
    auto histZ = std::make_unique<TH1F>(nameZ.c_str(), titleZ.c_str(), 100, -150, 150);
    m_pixelZSharedLocks.emplace_back();
    ATH_CHECK(m_tHistSvc->regShared(idZ, std::move(histZ), m_pixelZSharedLocks.back()));
  }

  for (SG::ReadHandleKey<xAOD::StripClusterContainer>& key : m_stripKeys) {
    const std::string idX = "/FPGAOutputValidation/" + key.key() + "X";
    const std::string nameX = key.key() + "X";
    const std::string titleX = key.key() + " X";
    auto histX = std::make_unique<TH1F>(nameX.c_str(), titleX.c_str(), 100, -300, 300);
    m_stripXSharedLocks.emplace_back();
    ATH_CHECK(m_tHistSvc->regShared(idX, std::move(histX), m_stripXSharedLocks.back()));

    const std::string idY = "/FPGAOutputValidation/" + key.key() + "Y";
    const std::string nameY = key.key() + "Y";
    const std::string titleY = key.key() + " Y";
    auto histY = std::make_unique<TH1F>(nameY.c_str(), titleY.c_str(), 100, -300, 300);
    m_stripYSharedLocks.emplace_back();
    ATH_CHECK(m_tHistSvc->regShared(idY, std::move(histY), m_stripYSharedLocks.back()));

    const std::string idZ = "/FPGAOutputValidation/" + key.key() + "Z";
    const std::string nameZ = key.key() + "Z";
    const std::string titleZ = key.key() + " Z";
    auto histZ = std::make_unique<TH1F>(nameZ.c_str(), titleZ.c_str(), 100, -150, 150);
    m_stripZSharedLocks.emplace_back();
    ATH_CHECK(m_tHistSvc->regShared(idZ, std::move(histZ), m_stripZSharedLocks.back()));
  }

  return StatusCode::SUCCESS;
}

StatusCode FPGAOutputValidationAlg::execute(const EventContext& ctx) const { 
  for (std::size_t index = 0; index < m_pixelKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key = m_pixelKeys[index];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle{key, ctx};

    for (const auto& cluster : *handle) {
      const xAOD::ConstVectorMap<3> globalPosition = cluster->globalPosition();

      m_pixelXSharedLocks[index]->Fill(globalPosition[0]);
      m_pixelYSharedLocks[index]->Fill(globalPosition[1]);
      m_pixelZSharedLocks[index]->Fill(globalPosition[2]);
    }
  }
      
  for (std::size_t index = 0; index < m_stripKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key = m_stripKeys[index];
    SG::ReadHandle<xAOD::StripClusterContainer> handle{key, ctx};

    for (const auto& cluster : *handle) {
      const xAOD::ConstVectorMap<3> globalPosition = cluster->globalPosition();

      m_stripXSharedLocks[index]->Fill(globalPosition[0]);
      m_stripYSharedLocks[index]->Fill(globalPosition[1]);
      m_stripZSharedLocks[index]->Fill(globalPosition[2]);
    }
  }

  return StatusCode::SUCCESS;
}

