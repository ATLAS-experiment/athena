/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHOTONVERTEXSELECTION_DECORATEPHOTONPOINTINGALG_H
#define PHOTONVERTEXSELECTION_DECORATEPHOTONPOINTINGALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/ToolHandle.h>
#include <AsgTools/PropertyWrapper.h>
#include <PhotonVertexSelection/IPhotonPointingTool.h>
#include <PhotonVertexSelection/IPhotonVertexSelectionTool.h>
#include <xAODEgamma/EgammaContainerFwd.h>
#include <xAODEventInfo/EventInfo.h>

/**
 * @brief An algorithm to decorate photons (also electrons) with pointing variables.
 * 
 * This algorithm uses the PhotonPointingTool to decorate photons with pointing
 * variables.
 * 
 */
class DecoratePhotonPointingAlg : public EL::AnaAlgorithm {
 public:
  DecoratePhotonPointingAlg(const std::string& name,
                            ISvcLocator* svcLoc = nullptr);

  virtual StatusCode initialize() override;
  StatusCode execute() override;

 private:

  //Write decoration handle keys
  SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_caloPointingZ;
  SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_zCommon;
  SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_zCommonError;

  Gaudi::Property<std::string> m_caloPointingZDecorName{this,"caloPointingZDecorName","caloPointingZ",""};
  Gaudi::Property<std::string> m_zCommonDecorName{this,"zCommonDecorName","zCommon",""};
  Gaudi::Property<std::string> m_zCommonErrorDecorName{this,"zCommonErrorDecorName","zCommonError",""};

  SG::ReadHandleKey<xAOD::EgammaContainer> m_photonContainerKey{
      this, "PhotonContainerKey", "Photons", "The input Photons container (it can be also Electrons)"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInKey{
      this, "EventInfoKey", "EventInfo", "containerName to read"};

  ToolHandle<CP::IPhotonPointingTool> m_pointingTool{
      this, "PhotonPointingTool",
      "CP::PhotonPointingTool/PhotonPointingTool", ""};

  ToolHandle<CP::IPhotonVertexSelectionTool> m_photonVtxTool{
      this, "PhotonVertexSelectionTool",
      "CP::PhotonVertexSelectionTool/PhotonVertexSelectionTool",
      "Photon vertex selection tool to use"};
};

#endif  // PHOTONVERTEXSELECTION_DECORATEPHOTONPOINTINGALG_H
