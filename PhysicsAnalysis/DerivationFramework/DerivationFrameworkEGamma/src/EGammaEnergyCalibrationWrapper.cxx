/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Author: Chengxi Yang (cxyang@berkeley.edu)

#include "DerivationFrameworkEGamma/EGammaEnergyCalibrationWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

StatusCode EGammaEnergyCalibrationWrapper::initialize()
{
  ATH_CHECK(m_electronContainerKey.initialize());
  ATH_CHECK(m_photonContainerKey.initialize());
  ATH_CHECK(m_electronEnergyDecoKey.initialize());
  ATH_CHECK(m_photonEnergyDecoKey.initialize());
  ATH_CHECK(m_eventInfo_key.initialize());
  ATH_CHECK(m_MVACalibSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode EGammaEnergyCalibrationWrapper::addBranches(const EventContext& ctx) const
{
  // Process electrons
  SG::ReadHandle<xAOD::EgammaContainer> electrons{m_electronContainerKey, ctx};
  if (!electrons.isValid()) {
    ATH_MSG_ERROR("Cannot retrieve electron container " << m_electronContainerKey.key());
    return StatusCode::FAILURE;
  }

  SG::WriteDecorHandle<xAOD::EgammaContainer, float> electronEnergyDeco{m_electronEnergyDecoKey, ctx};
  SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key, ctx);
  if (!evt.isValid()) {
    ATH_MSG_ERROR("Cannot retrieve EventInfo " << m_eventInfo_key.key());
    return StatusCode::FAILURE;
  }

  for (const xAOD::Egamma* eg : *electrons) {
    if (!eg) continue;
    const xAOD::CaloCluster* cluster = eg->caloCluster();
    float value = 0;
    if (!cluster) {
      ATH_MSG_ERROR("Electron object without CaloCluster, storing zero");
      electronEnergyDeco(*eg) = value;
      continue;
    }

    double calibratedEnergy = 0.;
    StatusCode sc = StatusCode::FAILURE;

    egammaMVACalib::GlobalEventInfo gei;
    gei.eventInfo = evt.cptr();
    sc = m_MVACalibSvc->getEnergy(*cluster, *eg, calibratedEnergy, gei);

    if (sc.isFailure()) {
      ATH_MSG_WARNING("MVACalibSvc failed for electron at eta=" << cluster->eta()
                       << " phi=" << cluster->phi());
    } else {
      value = static_cast<float>(calibratedEnergy);
    }
    electronEnergyDeco(*eg) = value;
  }

  // Process photons
  SG::ReadHandle<xAOD::EgammaContainer> photons{m_photonContainerKey, ctx};
  if (!photons.isValid()) {
    ATH_MSG_ERROR("Cannot retrieve photon container " << m_photonContainerKey.key());
    return StatusCode::FAILURE;
  }

  SG::WriteDecorHandle<xAOD::EgammaContainer, float> photonEnergyDeco{m_photonEnergyDecoKey, ctx};
  for (const xAOD::Egamma* eg : *photons) {
    if (!eg) continue;
    const xAOD::CaloCluster* cluster = eg->caloCluster();
    float value = 0;
    if (!cluster) {
      ATH_MSG_ERROR("Photon object without CaloCluster");
      photonEnergyDeco(*eg) = value;
      continue;
    }

    double calibratedEnergy = 0.;
    StatusCode sc = StatusCode::FAILURE;
    
    egammaMVACalib::GlobalEventInfo gei;
    gei.eventInfo = evt.cptr();
    sc = m_MVACalibSvc->getEnergy(*cluster, *eg, calibratedEnergy, gei);

    if (sc.isFailure()) {
      ATH_MSG_WARNING("MVACalibSvc failed for photon at eta=" << cluster->eta()
                       << " phi=" << cluster->phi());
    } else {
      value = static_cast<float>(calibratedEnergy);
    }
    photonEnergyDeco(*eg) = value;
  }

  return StatusCode::SUCCESS;
}

} // namespace DerivationFramework