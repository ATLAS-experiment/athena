/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonVertexSelection/DecoratePhotonPointingAlg.h"
#include "PhotonVertexSelection/PhotonVertexHelpers.h"
#include "xAODEgamma/EgammaContainer.h"
#include "AthContainers/ConstDataVector.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgTools/CurrentContext.h"

DecoratePhotonPointingAlg::DecoratePhotonPointingAlg(const std::string& name,
                                                     ISvcLocator* svcLoc)
    : EL::AnaAlgorithm(name, svcLoc) {}

StatusCode DecoratePhotonPointingAlg::initialize() {
  ATH_CHECK(m_photonContainerKey.initialize());
  ATH_CHECK(m_eventInKey.initialize());
  
  ATH_CHECK(m_pointingTool.retrieve());
  ATH_CHECK(m_photonVtxTool.retrieve());

  std::string baseName = m_photonContainerKey.key();

  m_caloPointingZ = baseName + "." + m_caloPointingZDecorName;
  m_zCommon = baseName + "." + m_zCommonDecorName;
  m_zCommonError = baseName + "." + m_zCommonErrorDecorName;

  ATH_CHECK(m_caloPointingZ.initialize());
  ATH_CHECK(m_zCommon.initialize());
  ATH_CHECK(m_zCommonError.initialize());

  return StatusCode::SUCCESS;
}

StatusCode DecoratePhotonPointingAlg::execute() {

  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<xAOD::EgammaContainer> egammas(m_photonContainerKey, ctx);
  if (!egammas.isValid()) {
    ATH_MSG_ERROR("Invalid egamma container from " << m_photonContainerKey.key());
    return StatusCode::FAILURE;
  }
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInKey, ctx);
  ATH_CHECK(eventInfo.isValid());

  SG::WriteDecorHandle<xAOD::EgammaContainer, float> caloPointingZDec(m_caloPointingZ, ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> zCommonDec(m_zCommon, ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> zCommonErrorDec(m_zCommonError, ctx);

  ATH_CHECK(m_pointingTool->updatePointingAuxdata(*egammas));
  ATH_CHECK(m_photonVtxTool->decorateInputs(*egammas));

  for(const xAOD::Egamma* photon : *egammas){
    std::pair<float, float> caloPointing = m_pointingTool->getCaloPointing(photon);

    caloPointingZDec(*photon) = caloPointing.first;

    ConstDataVector<DataVector<xAOD::Egamma>> photons_selected(SG::VIEW_ELEMENTS);
    photons_selected.push_back(photon);
    std::pair<float, float> zCommon = xAOD::PVHelpers::getZCommonAndError(&(*eventInfo), photons_selected.asDataVector(), 2e3);
    zCommonDec(*photon) = zCommon.first;
    zCommonErrorDec(*photon) = zCommon.second;
  }

  return StatusCode::SUCCESS;
}
