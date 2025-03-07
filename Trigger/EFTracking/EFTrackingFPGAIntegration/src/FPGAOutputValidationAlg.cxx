/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "FPGAOutputValidationAlg.h"

#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

FPGAOutputValidationAlg::FPGAOutputValidationAlg(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode FPGAOutputValidationAlg::initialize() {
  ATH_CHECK(m_pixelKeys.initialize(!m_pixelKeys.empty()));
  ATH_CHECK(m_stripKeys.initialize(!m_stripKeys.empty()));

  ATH_CHECK(m_monitoringTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode FPGAOutputValidationAlg::execute(const EventContext& ctx) const { 
  for (std::size_t index = 0; index < m_pixelKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key = m_pixelKeys[index];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle{key, ctx};

    Monitored::Group(
      m_monitoringTool,
      std::move(Monitored::Collection(key.key() + "_LOCALPOSITION_X", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localPosition<2>()[0];
      })),
      std::move(Monitored::Collection(key.key() + "_LOCALPOSITION_Y", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localPosition<2>()[1];
      })),
      std::move(Monitored::Collection(key.key() + "_LOCALCOVARIANCE_XX", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localCovariance<2>()(0, 0);
      })),
      std::move(Monitored::Collection(key.key() + "_LOCALCOVARIANCE_YY", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localCovariance<2>()(1, 1);
      })),
      std::move(Monitored::Collection(key.key() + "_OMEGA_X", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->omegaX();
      })),
      std::move(Monitored::Collection(key.key() + "_OMEGA_Y", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->omegaY();
      })),
      std::move(Monitored::Collection(key.key() + "_GLOBALPOSITION_X", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->globalPosition()[0];
      })),
      std::move(Monitored::Collection(key.key() + "_GLOBALPOSITION_Y", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->globalPosition()[1];
      })),
      std::move(Monitored::Collection(key.key() + "_GLOBALPOSITION_Z", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->globalPosition()[2];
      })),
      std::move(Monitored::Collection(key.key() + "_CHANNELS_IN_PHI", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->channelsInPhi();
      })),
      std::move(Monitored::Collection(key.key() + "_CHANNELS_IN_ETA", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->channelsInEta();
      })),
      std::move(Monitored::Collection(key.key() + "_WIDTH_IN_ETA", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->widthInEta();
      })),
      std::move(Monitored::Collection(key.key() + "_TOTAL_TOT", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->totalToT();
      }))
    );
  }
      
  for (std::size_t index = 0; index < m_stripKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key = m_stripKeys[index];
    SG::ReadHandle<xAOD::StripClusterContainer> handle{key, ctx};

    Monitored::Group(
      m_monitoringTool,
      std::move(Monitored::Collection(key.key() + "_LOCALPOSITION_X", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->localPosition<2>()[0];
      })),
      std::move(Monitored::Collection(key.key() + "_LOCALPOSITION_Y", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->localPosition<2>()[1];
      })),
      std::move(Monitored::Collection(key.key() + "_LOCALCOVARIANCE_XX", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->localCovariance<2>()(0, 0);
      })),
      std::move(Monitored::Collection(key.key() + "_LOCALCOVARIANCE_YY", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->localCovariance<2>()(1, 1);
      })),
      std::move(Monitored::Collection(key.key() + "_GLOBALPOSITION_X", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->globalPosition()[0];
      })),
      std::move(Monitored::Collection(key.key() + "_GLOBALPOSITION_Y", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->globalPosition()[1];
      })),
      std::move(Monitored::Collection(key.key() + "_GLOBALPOSITION_Z", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->globalPosition()[2];
      })),
      std::move(Monitored::Collection(key.key() + "_CHANNELS_IN_PHI", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->channelsInPhi();
      }))
    );
  }

  return StatusCode::SUCCESS;
}

