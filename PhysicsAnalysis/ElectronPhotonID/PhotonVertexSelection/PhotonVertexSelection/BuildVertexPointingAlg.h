/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHOTONVERTEXSELECTION_BUILDVERTEXPOINTINGALG_H
#define PHOTONVERTEXSELECTION_BUILDVERTEXPOINTINGALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <PATCore/IAsgSelectionTool.h>

#include <xAODTracking/VertexContainerFwd.h>
#include <xAODEgamma/EgammaContainerFwd.h>
#include <xAODEventInfo/EventInfo.h>

#include "AsgDataHandles/ReadHandleKey.h"
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>

#include <string>


/**
 * @brief An algorithm to build a vertex from photon pointing
 *
 * This algorithm is needed by @c VertexSelectionToolBDT.
 *
 */
class BuildVertexPointingAlg : public EL::AnaAlgorithm {
 public:
  BuildVertexPointingAlg(const std::string& name,
                         ISvcLocator* svcLoc = nullptr);
  virtual StatusCode initialize();
  virtual StatusCode execute();

 private:
  SG::ReadHandleKey<xAOD::EgammaContainer> m_photonContainerKey{
      this, "PhotonContainerKey", "Photons",
      "The input photons (or electrons) container"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey",
                                                    "EventInfo", ""};
  SG::WriteHandleKey<xAOD::VertexContainer> m_pointingVertexContainerKey{
      this, "PointingVertexContainerKey", "PhotonPointingVertices", ""};

  // additional information written to the vertex as decorations
  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_nphotons_good_key{
    this, "NGoodPhotonsKey", m_pointingVertexContainerKey, "nphotons_good", ""};
  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_z_common_nphotons_key{
    this, "ZCommonPhotonsKey", m_pointingVertexContainerKey, "z_common_nphotons", ""};
  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_photons_px_key {
    this, "PhotonsPxKey", m_pointingVertexContainerKey, "photons_px", ""};
  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_photons_py_key {
    this, "PhotonsPyKey", m_pointingVertexContainerKey, "photons_py", ""};
  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_photons_pz_key {
    this, "PhotonsPzKey", m_pointingVertexContainerKey, "photons_pz", ""};
  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_photon_links_key {
    this, "PhotonLinksKey", m_pointingVertexContainerKey, "PhotonLinks", ""};

  // decoration read from Photon by xAOD::PVHelpers::getZCommonAndError function
  // the user should not change them
  SG::ReadDecorHandleKey<xAOD::EgammaContainer> m_photon_zvertex_key{
    this, "PhotonZVertexKey", m_photonContainerKey, "zvertex"};
  SG::ReadDecorHandleKey<xAOD::EgammaContainer> m_photon_errz_key{
    this, "PhotonErrZKey", m_photonContainerKey, "errz"};
  SG::ReadDecorHandleKey<xAOD::EgammaContainer> m_photon_HPV_zvertex_key{
    this, "PhotonHPVZVertexKey", m_photonContainerKey, "HPV_zvertex"};
  SG::ReadDecorHandleKey<xAOD::EgammaContainer> m_photon_HPV_errz_key{
    this, "PhotonHPVErrZKey", m_photonContainerKey, "HPV_errz"};

  ToolHandle<IAsgSelectionTool> m_selectionTool{
      this, "selectionTool", "CP::AsgPtEtaSelectionTool/PhotonSelectionTool",
      "the selection tool we apply to photons to compute pointing variables"};

  ToolHandle<IAsgSelectionTool> m_goodPhotonSelectionTool{
      this, "goodPhotonSelectionTool", "",
      "the selection tool to count good photons"};

  Gaudi::Property<float> m_convPtCut {this, "convPtCut", 2E3, "the conversion pt cut"};
  Gaudi::Property<int> m_nphotons_to_use {this, "nphotonsToUse", -1, "the number of photons to use for the pointing computation"};

 void selectPhotons(const xAOD::EgammaContainer& original_photons,
    ConstDataVector<DataVector<xAOD::Egamma>>& photons_selected) const;


};

#endif  // PHOTONVERTEXSELECTION_BUILDVERTEXPOINTINGALG_H