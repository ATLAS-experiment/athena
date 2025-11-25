/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EGELECTRONAMBIGUITYTOOL_H
#define DERIVATIONFRAMEWORK_EGELECTRONAMBIGUITYTOOL_H

#include "GaudiKernel/ToolHandle.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "AthContainers/ConstDataVector.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

#include <string>

namespace DerivationFramework {

  class EGElectronAmbiguityTool : public extends<AthAlgTool, IAugmentationTool>
  {

  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::ElectronContainer> m_containerName{
      this,
        "ContainerName",
        "Electrons",
        "SG key of electron container"
        };
    SG::ReadHandleKey<xAOD::VertexContainer> m_VtxContainerName{
      this,
      "VtxContainerName",
      "PrimaryVertices",
      "SG key of vertex container"
    };
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tpContainerName{
      this,
      "tpContainerName",
      "InDetTrackParticles",
      "SG key of track particles container"
    };

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tpCName{
      this,
      "tpCName",
      "GSFTrackParticles",
      "SG key of TrackParticleInputContainer"
    };

    // Write decoration handle keys
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_drv{ this, "DFCommonSimpleConvRadius", m_containerName, "DFCommonSimpleConvRadius", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dphiv{ this, "DFCommonSimpleConvPhi", m_containerName, "DFCommonSimpleConvPhi", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dmee{ this, "DFCommonSimpleMee", m_containerName, "DFCommonSimpleMee", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dmeeVtx{ this, "DFCommonSimpleMeeAtVtx", m_containerName, "DFCommonSimpleMeeAtVtx", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dsep{ this, "DFCommonSimpleSeparation", m_containerName, "DFCommonSimpleSeparation", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dambi{ this, "DFCommonAddAmbiguity", m_containerName, "DFCommonAddAmbiguity", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dtrv{ this, "DFCommonProdTrueRadius", m_containerName, "DFCommonProdTrueRadius", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dtpv{ this, "DFCommonProdTruePhi", m_containerName, "DFCommonProdTruePhi", "" };

    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_dtzv{ this, "DFCommonProdTrueZ", m_containerName, "DFCommonProdTrueZ", "" };

    struct DecorHandles {
      DecorHandles (const EGElectronAmbiguityTool& tool, const EventContext& ctx);
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> drv;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dphiv;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dmee;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dmeeVtx;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dsep;
      SG::WriteDecorHandle<xAOD::ElectronContainer, int>   dambi;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dtrv;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dtpv;
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> dtzv;
    };
    friend struct DecorHandles;

    StatusCode decorateSimple(
                              DecorHandles& dh,
                              std::unique_ptr<ConstDataVector<xAOD::TrackParticleContainer>>& tpC,
                              const xAOD::Electron* ele,
                              const xAOD::Vertex* pvtx) const;

    Gaudi::Property<bool> m_isMC{this, "isMC", false};

    // cuts to select the electron to run on
    Gaudi::Property<double> m_elepTCut{
      this, "pTCut", 9000., "minimum pT for an electron to be studied"};
    Gaudi::Property<std::string> m_idCut{
      this, "idCut", "DFCommonElectronsLHLoose", "minimal quality for an electron to be studied"};

    // cuts to select the other track
    Gaudi::Property<unsigned int> m_nSiCut{
      this, "nSiCut", 7, "minimum number of Si hits in the other track"};
    Gaudi::Property<double> m_dctCut{
      this, "DCTCut", 0.02, "second separation cut"};
    Gaudi::Property<double> m_sepCut{
      this, "SeparationCut", 1., "first separation cut"};
    Gaudi::Property<double> m_dzCut{
      this, "dzsinTCut", 0.5, "max dz sinTheta between ele and other tracks"};

    // cuts to define the various types :
    // ambi = -1 : no other track, 0 : other track exists but no good gamma reco,
    // 1 : gamma*, 2 : material conversion
    Gaudi::Property<double> m_rvECCut{
      this, "radiusCut", 20, "minimum radius to be classified as external conversion"};
    Gaudi::Property<double> m_meeAtVtxECCut{
      this, "meeAtVtxCut", 100, "maximal mass at vertex to be classified as external conversion"};
    Gaudi::Property<double> m_meeICCut{
      this, "meeCut", 100, "maximal mass at primary vertex to be classified as gamma*"};
  };
}

#endif // DERIVATIONFRAMEWORK_EGCONVERSIONINFOTOOL_H
