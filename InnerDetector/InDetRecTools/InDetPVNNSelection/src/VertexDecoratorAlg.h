/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef PVNN_VERTEXDECORATORALG
#define PVNN_VERTEXDECORATORALG

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
#include <InDetPVNNSelection/PVNNTool.h>
#include <TrackVertexAssociationTool/TrackVertexAssociationTool.h>
#include <xAODParticleEvent/CompositeParticleContainer.h>

/**
 * @class VertexDecoratorAlg
 *
 * The VertexDecoratorAlg class is responsible for adding decorations to
 * primary vertices needed by the transformer-based Hard Scatter selection algorithm, PVNN.
 * The decorations include links to electrons, muons, jets, and photons and additional
 * derived quantities needed by the InDetPVNNSelection::PVNNTool.
 * 
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */

namespace InDetPVNNSelection
{

  class VertexDecoratorAlg final : public AthHistogramAlgorithm{
public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) override;
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
        this, "jetsIn", "", "containerName to read"};
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonsInKey{
        this, "photonsIn", "Photons", "containerName to read"};

    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_ntrk {
      this, "decor_ntrkKey", m_vertexInKey, "ntrk"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_sumPt {
      this, "decor_sumPtKey", m_vertexInKey, "sumPt"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_chi2Over_ndf {
      this, "decor_chi2Over_ndfKey", m_vertexInKey, "chi2Over_ndf"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_z_asym {
      this, "decor_z_asymmetryKey", m_vertexInKey, "z_asymmetry"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_weighted_z_asym {
      this, "decor_weighted_z_asym", m_vertexInKey, "weighted_z_asymmetry"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_weighted_z_kurt {
      this, "decor_weighted_z_kurt", m_vertexInKey, "z_kurtosis"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_z_skew {
      this, "decor_z_skew", m_vertexInKey, "z_skewness"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_photon_deltaz {
      this, "decor_photon_deltaz", m_vertexInKey, "photon_deltaz"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_photon_deltaPhi {
      this, "decor_photon_deltaPhi", m_vertexInKey, "photon_deltaPhi"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_actualInterPerXing {
      this, "decor_actualInterPerXing", m_vertexInKey, "actualIntPerXing"};

    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_photonLinksKey{
      this, "photonLinks", m_vertexInKey, "photonLinks"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_jetLinksKey{
      this, "jetLinks", m_vertexInKey, "jetLinks"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_electronLinksKey{
      this, "electronLinks", m_vertexInKey, "electronLinks"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_muonLinksKey{
      this, "muonLinks", m_vertexInKey, "muonLinks"};
    
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_pvnnScore{
      this, "PVNNScoreKey", m_vertexInKey, "HSGN2_phsvertex", "Score from PVNN"};
      
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_nElectrons {
      this, "decor_nElectrons", m_vertexInKey, "nElectrons"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_nMuons {
      this, "decor_nMuons", m_vertexInKey, "nMuons"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_nJets {
      this, "decor_nJets", m_vertexInKey, "nJets"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_nPhotons {
      this, "decor_nPhotons", m_vertexInKey, "nPhotons"};

    SG::WriteHandleKey<xAOD::CompositeParticleContainer> m_multiPhotonsOutKey{
      this, "multiPhotonsOut", "MultiPhotons", "Output container for multi-photon node"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_multiPhotonLinksKey{
      this, "multiPhotonLinks", m_vertexInKey, "multiPhotonLinks",
	"Per-vertex link(s) to multi-photon node"};


    SG::ReadDecorHandleKey<xAOD::VertexContainer> m_deltaZKey{
      this, "deltaZKey", m_vertexInKey, "deltaZ"};
    SG::ReadDecorHandleKey<xAOD::VertexContainer> m_deltaPhiKey{
      this, "deltaPhiKey", m_vertexInKey, "deltaPhi"};

    // additional ReadHandles for the Photons to declare dependencies
    // to the scheduler
    SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_caloPointingZKey{
      this, "caloPointingZKey", m_photonsInKey, "caloPointingZ"};
    SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_zCommonKey{
      this, "zCommonKey",  m_photonsInKey, "zCommon"};
    SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_zCommonErrorKey{
      this, "zCommonErrorKey",  m_photonsInKey, "zCommonError"};

    // Internal members
    ToolHandle<CP::TrackVertexAssociationTool> m_trkVtxAssociationTool{
        this, "TrackVertexAssociationTool", 
        "CP::TrackVertexAssociationTool/TrackVertexAssociationTool", 
        "Track vertex association tool to use"};

    ToolHandle<PVNNTool> m_pvnnTool{
        this, "pvnnTool",
        "InDetPVNNSelection/PVNNTool",
        "PVNN tool to use"};

  };
} // namespace InDetPVNNSelection

#endif
