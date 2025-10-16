/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#ifndef JPSIXPLUSDISPLACED_H
#define JPSIXPLUSDISPLACED_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkV0Fitter/TrkV0VertexFitter.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>
// dummy EventContext for AnalysisBase
#include "AsgTools/CurrentContext.h"

namespace Trk {
    class IVertexFitter;
    class TrkVKalVrtFitter;
    class IVertexCascadeFitter;
    class VxCascadeInfo;
    class V0Tools;
    class IExtrapolator;
}
namespace InDet { class VertexPointEstimator; }
namespace DerivationFramework {
    class CascadeTools;
}

namespace DerivationFramework {

  class JpsiXPlusDisplaced : public extends<AthAlgTool, IAugmentationTool>
  {
  public:
    enum V0Enum{ UNKNOWN=0, LAMBDA=1, LAMBDABAR=2, KS=3 };

    struct XiCandidate {
      V0Enum V0type = UNKNOWN;
      const xAOD::Vertex* V0vtx = nullptr;
      const xAOD::TrackParticle* track = nullptr;
      double chi2NDF = 999;
      TLorentzVector p4_V0track1;
      TLorentzVector p4_V0track2;
      TLorentzVector p4_disVtrack;
    };

    struct MesonCandidate {
      V0Enum V0type = UNKNOWN;
      const xAOD::Vertex* V0vtx = nullptr;
      const xAOD::TrackParticle* extraTrack1 = nullptr;
      const xAOD::TrackParticle* extraTrack2 = nullptr;
      const xAOD::TrackParticle* extraTrack3 = nullptr;
      double chi2NDF = 999;
      double pt = 0;
    };

    class MesonCandidateVector {
    public:
      MesonCandidateVector(size_t num, bool orderByPt);
      void push_back(const MesonCandidate& etac);
      const std::vector<MesonCandidate>& vector() const;

    private:
      size_t m_num{0};
      bool m_orderByPt{true};
      std::vector<MesonCandidate> m_vector;
    };

    JpsiXPlusDisplaced(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~JpsiXPlusDisplaced() = default;
    virtual StatusCode initialize() override;
    StatusCode performSearch(std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> >& cascadeinfoContainer, const std::vector<std::pair<const xAOD::Vertex*,V0Enum> >& selectedV0Candidates, const std::vector<const xAOD::TrackParticle*>& tracksDisplaced, const EventContext& ctx) const;
    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexJXContainerKey;
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexV0ContainerKey;
    std::vector<std::string> m_vertexJXHypoNames;
    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_cascadeOutputKeys;
    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_cascadeOutputKeys_mvc;
    SG::WriteHandleKey<xAOD::VertexContainer> m_v0VtxOutputKey;
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrkParticleCollection;
    SG::ReadHandleKey<xAOD::VertexContainer> m_VxPrimaryCandidateName;
    SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName;
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key;
    SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_RelinkContainers;
    std::string m_hypoName;

    bool   m_useImprovedMass{};
    double m_jxMassLower{};
    double m_jxMassUpper{};
    double m_jpsiMassLower{};
    double m_jpsiMassUpper{};
    double m_diTrackMassLower{};
    double m_diTrackMassUpper{};
    std::string m_V0Hypothesis{};
    double m_LambdaMassLower{};
    double m_LambdaMassUpper{};
    double m_KsMassLower{};
    double m_KsMassUpper{};
    double m_lxyV0_cut{};
    double m_minMass_gamma{};
    double m_chi2cut_gamma{};
    double m_DisplacedMassLower{};
    double m_DisplacedMassUpper{};
    double m_lxyDisV_cut{};
    double m_lxyDpm_cut{};
    double m_lxyD0_cut{};
    double m_MassLower{};
    double m_MassUpper{};
    double m_PostMassLower{};
    double m_PostMassUpper{};
    int    m_jxDaug_num{};
    double m_jxDaug1MassHypo{}; // mass hypothesis of 1st daughter from vertex JX
    double m_jxDaug2MassHypo{}; // mass hypothesis of 2nd daughter from vertex JX
    double m_jxDaug3MassHypo{}; // mass hypothesis of 3rd daughter from vertex JX
    double m_jxDaug4MassHypo{}; // mass hypothesis of 4th daughter from vertex JX
    bool   m_jxPtOrdering{};
    int    m_disVDaug_num{};
    double m_disVDaug3MassHypo{}; // mass hypothesis of 3rd daughter from displaced vertex
    double m_disVDaug3MinPt{};
    double m_extraTrk1MassHypo{};
    double m_extraTrk1MinPt{};
    double m_extraTrk2MassHypo{};
    double m_extraTrk2MinPt{};
    double m_extraTrk3MassHypo{};
    double m_extraTrk3MinPt{};
    double m_DpmMassLower{};
    double m_DpmMassUpper{};
    double m_D0MassLower{};
    double m_D0MassUpper{};
    size_t m_maxMesonCandidates{};
    bool   m_MesonPtOrdering{};
    double m_massJX{};
    double m_massJpsi{};
    double m_massX{};
    double m_massDisV{};
    double m_massLd{};
    double m_massKs{};
    double m_massDpm{};
    double m_massD0{};
    double m_massJXV0{};
    double m_massMainV{};
    bool   m_constrJX{};
    bool   m_constrJpsi{};
    bool   m_constrX{};
    bool   m_constrDisV{};
    bool   m_constrV0{};
    bool   m_constrDpm{};
    bool   m_constrD0{};
    bool   m_constrJXV0{};
    bool   m_constrMainV{};
    bool   m_doPostMainVContrFit{};
    bool   m_JXSubVtx{};
    bool   m_JXV0SubVtx{};
    double m_chi2cut_JX{};
    double m_chi2cut_V0{};
    double m_chi2cut_DisV{};
    double m_chi2cut_Dpm{};
    double m_chi2cut_D0{};
    double m_chi2cut{};
    bool   m_useTRT{};
    double m_ptTRT{};
    double m_d0_cut{};
    unsigned int m_maxJXCandidates{};
    unsigned int m_maxV0Candidates{};
    unsigned int m_maxDisVCandidates{};
    unsigned int m_maxMainVCandidates{};

    ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};

    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Trk::TrkV0VertexFitter >            m_iV0Fitter;
    ToolHandle < Trk::IVertexFitter >                m_iGammaFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < Reco::ITrackToVertex >              m_trackToVertexTool;
    ToolHandle < Trk::ITrackSelectorTool >           m_trkSelector;
    ToolHandle < Trk::ITrackSelectorTool >           m_v0TrkSelector;
    ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;
    ToolHandle < InDet::VertexPointEstimator >       m_vertexEstimator;
    ToolHandle < Trk::IExtrapolator >                m_extrapolator;

    bool   m_refitPV{};
    int    m_PV_max{};
    size_t m_PV_minNTracks{};
    int    m_DoVertexType{};

    double m_mass_e{};
    double m_mass_mu{};
    double m_mass_pion{};
    double m_mass_proton{};
    double m_mass_Lambda{};
    double m_mass_Ks{};
    double m_mass_Xi{};
    double m_mass_phi{};
    double m_mass_B0{};
    double m_mass_Dpm{};
    double m_mass_D0{};
    double m_mass_BCPLUS{};
    double m_mass_Lambdab{};

    std::vector<double> m_massesV0_ppi;
    std::vector<double> m_massesV0_pip;
    std::vector<double> m_massesV0_pipi;

    bool d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const;
    XiCandidate getXiCandidate(const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticle* track3) const;
    std::unique_ptr<xAOD::Vertex> fitTracks(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2, const xAOD::TrackParticle* track3 = nullptr) const;
    MesonCandidate getDpmCandidate(const xAOD::Vertex* JXvtx, const xAOD::TrackParticle* extraTrk1, const xAOD::TrackParticle* extraTrk2, const xAOD::TrackParticle* extraTrk3) const;
    MesonCandidate getD0Candidate(const xAOD::Vertex* JXvtx, const xAOD::TrackParticle* extraTrk1, const xAOD::TrackParticle* extraTrk2) const;
    std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> > fitMainVtx(const xAOD::Vertex* JXvtx, const std::vector<double>& massesJX, const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticleContainer* trackContainer, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const;
    std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> > fitMainVtx(const xAOD::Vertex* JXvtx, const std::vector<double>& massesJX, const XiCandidate& disVtx, const xAOD::TrackParticleContainer* trackContainer, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const;
    void fitV0Container(xAOD::VertexContainer* V0ContainerNew, const std::vector<const xAOD::TrackParticle*>& selectedTracks, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const;
    template<size_t NTracks> const xAOD::Vertex* FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const;
  };
}

#endif
