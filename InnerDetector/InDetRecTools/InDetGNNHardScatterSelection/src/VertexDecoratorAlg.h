/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HSGNN_VERTEXDECORATORALG
#define HSGNN_VERTEXDECORATORALG

#include <utility>
#include <vector>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODTracking/VertexContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>

#include <InDetTruthVertexValidation/IInDetVertexTruthMatchTool.h>
#include <InDetGNNHardScatterSelection/GNNTool.h>
#include <TrackVertexAssociationTool/TrackVertexAssociationTool.h>

/**
 * @class VertexDecoratorAlg
 *
 * The VertexDecoratorAlg class is responsible for adding decorations to
 * primary vertices needed by the GNN-based Hard Scatter selection algorithm.
 * The decorations include links to electrons, muons, jets, and photons and additional
 * derived quantities needed by the InDetGNNHardScatterSelection::GNNTool.
 * 
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */

namespace InDetGNNHardScatterSelection
{

  class VertexDecoratorAlg final : public AthHistogramAlgorithm{
public:
    VertexDecoratorAlg(const std::string& name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
private:

    // Members for configurable properties
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexInKey{
        this, "vertexIn", "PrimaryVertices", "containerName to read"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInKey{
        this, "eventIn", "EventInfo", "containerName to read"};
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronsInKey{
        this, "electronsIn", "Electrons", "containerName to read"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsInKey{
        this, "muonsIn", "Muons", "containerName to read"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsInKey{
        this, "jetsIn", "AntiKt4EMTopoJets", "containerName to read"};
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonsInKey{
        this, "photonsIn", "Photons", "containerName to read"};

    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_ntrk {
      this, "decor_ntrkKey", "ntrk", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_sumPt {
      this, "decor_sumPtKey", "sumPt", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_chi2Over_ndf {
      this, "decor_chi2Over_ndfKey", "chi2Over_ndf", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_z_asym {
      this, "decor_z_asymmetryKey", "z_asymmetry", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_weighted_z_asym {
      this, "decor_weighted_z_asym", "weighted_z_asymmetry", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_weighted_z_kurt {
      this, "decor_weighted_z_kurt", "z_kurtosis", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_z_skew {
      this, "decor_z_skew", "z_skewness", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_photon_deltaz {
      this, "decor_photon_deltaz", "photon_deltaz", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_photon_deltaPhi {
      this, "decor_photon_deltaPhi", "photon_deltaPhi", ""};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_actualInterPerXing {
      this, "decor_actualInterPerXing", "actualIntPerXing", ""};

    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_photonLinksKey{
      this, "photonLinks", "", "" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_jetLinksKey{
      this, "jetLinks", "", "" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_electronLinksKey{
      this, "electronLinks", "", "" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_muonLinksKey{
      this, "muonLinks", "", "" };

    SG::ReadDecorHandleKey<xAOD::VertexContainer> m_deltaZKey{
      this, "deltaZKey", "", "" };
    SG::ReadDecorHandleKey<xAOD::VertexContainer> m_deltaPhiKey{
      this, "deltaPhiKey", "", "" };

    // additional ReadHandles for the Photons to declare dependencies
    // to the scheduler
    SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_caloPointingZKey{
      this, "caloPointingZKey", "", "" };
    SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_zCommonKey{
      this, "zCommonKey", "", "" };
    SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_zCommonErrorKey{
      this, "zCommonErrorKey", "", "" };

    // Internal members
    ToolHandle<CP::TrackVertexAssociationTool> m_trkVtxAssociationTool{
        this, "TrackVertexAssociationTool", 
        "CP::TrackVertexAssociationTool/TrackVertexAssociationTool", 
        "Track vertex association tool to use"};

    ToolHandle<GNNTool> m_gnnTool{
        this, "gnnTool",
        "InDetGNNHardScatterSelection/GNNTool",
        "GNN tool to use"};
  };
} // namespace InDetGNNHardScatterSelection

#endif
