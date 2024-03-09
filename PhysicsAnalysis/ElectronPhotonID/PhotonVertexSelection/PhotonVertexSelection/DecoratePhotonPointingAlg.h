/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHOTONVERTEXSELECTION_DECORATEPHOTONPOINTINGALG_H
#define PHOTONVERTEXSELECTION_DECORATEPHOTONPOINTINGALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/ToolHandle.h>
#include <PhotonVertexSelection/IPhotonPointingTool.h>
#include <xAODEgamma/EgammaContainerFwd.h>

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
  SG::ReadHandleKey<xAOD::EgammaContainer> m_photonContainerKey{
      this, "PhotonContainerKey", "Photons", "The input Photons container (it can be also Electrons)"};

  ToolHandle<CP::IPhotonPointingTool> m_pointingTool{
      this, "PhotonPointingTool",
      "CP::PhotonPointingTool/PhotonPointingTool", ""};
};

#endif  // PHOTONVERTEXSELECTION_DECORATEPHOTONPOINTINGALG_H