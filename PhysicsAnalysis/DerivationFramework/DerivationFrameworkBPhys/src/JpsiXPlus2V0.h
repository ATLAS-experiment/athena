/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#ifndef JPSIXPLUS2V0_H
#define JPSIXPLUS2V0_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
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

  class JpsiXPlus2V0 : public extends<AthAlgTool, IAugmentationTool>
  {
  enum V0Enum{ UNKNOWN=0, LAMBDA=1, LAMBDABAR=2, KS=3 };

  public:
    JpsiXPlus2V0(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~JpsiXPlus2V0() = default;
    virtual StatusCode initialize() override;
    StatusCode performSearch(std::vector<Trk::VxCascadeInfo*>& cascadeinfoContainer, const std::vector<std::pair<const xAOD::Vertex*,V0Enum> >& selectedV0Candidates) const;
    virtual StatusCode addBranches() const override;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexJXContainerKey;
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexV0ContainerKey;
    std::vector<std::string> m_vertexJXHypoNames;
    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_cascadeOutputKeys;
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
    std::string m_V01Hypothesis{};
    std::string m_V02Hypothesis{};
    double m_LambdaMassLower{};
    double m_LambdaMassUpper{};
    double m_KsMassLower{};
    double m_KsMassUpper{};
    double m_lxyV01_cut{};
    double m_lxyV02_cut{};
    double m_minMass_gamma{};
    double m_chi2cut_gamma{};
    double m_JXV02MassLower{};
    double m_JXV02MassUpper{};
    double m_MassLower{};
    double m_MassUpper{};
    int    m_jxDaug_num{};
    double m_jxDaug1MassHypo{}; // mass hypothesis of 1st daughter from vertex JX
    double m_jxDaug2MassHypo{}; // mass hypothesis of 2nd daughter from vertex JX
    double m_jxDaug3MassHypo{}; // mass hypothesis of 3rd daughter from vertex JX
    double m_jxDaug4MassHypo{}; // mass hypothesis of 4th daughter from vertex JX
    double m_massJX{};
    double m_massJpsi{};
    double m_massX{};
    double m_massLd{};
    double m_massKs{};
    double m_massJXV02{};
    double m_massMainV{};
    bool   m_constrJX{};
    bool   m_constrJpsi{};
    bool   m_constrX{};
    bool   m_constrV01{};
    bool   m_constrV02{};
    bool   m_constrJXV02{};
    bool   m_constrMainV{};
    bool   m_JXSubVtx{};
    bool   m_JXV02SubVtx{};
    double m_chi2cut_JX{};
    double m_chi2cut_V0{};
    double m_chi2cut{};
    bool   m_useTRT{};
    double m_ptTRT{};
    double m_d0_cut{};
    unsigned int m_maxJXCandidates{};
    unsigned int m_maxV0Candidates{};
    unsigned int m_maxMainVCandidates{};

    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Trk::TrkV0VertexFitter >            m_iV0Fitter;
    ToolHandle < Trk::IVertexFitter >                m_iGammaFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < Reco::ITrackToVertex >              m_trackToVertexTool;
    ToolHandle < Trk::ITrackSelectorTool >           m_v0TrkSelector;
    ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;
    ToolHandle < InDet::VertexPointEstimator >       m_vertexEstimator;
    ToolHandle < Trk::IExtrapolator >                m_extrapolator;

    ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};

    bool   m_refitPV{};
    int    m_PV_max{};
    size_t m_PV_minNTracks{};
    int    m_DoVertexType{};

    double m_mass_e{};
    double m_mass_mu{};
    double m_mass_pion{};
    double m_mass_proton{};
    double m_mass_Lambda{};
    double m_mass_Lambdab{};
    double m_mass_Ks{};
    double m_mass_Bpm{};
    double m_mass_phi{};

    std::vector<double> m_massesV0_ppi;
    std::vector<double> m_massesV0_pip;
    std::vector<double> m_massesV0_pipi;

    bool d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const;
    Trk::VxCascadeInfo* fitMainVtx(const xAOD::Vertex* JXvtx, std::vector<double>& massesJX, const xAOD::Vertex* V01vtx, const V0Enum V01, const xAOD::Vertex* V02vtx, const V0Enum V02, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const;
    void fitV0Container(xAOD::VertexContainer* V0ContainerNew, const std::vector<const xAOD::TrackParticle*>& selectedTracks, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const;
    template<size_t NTracks> const xAOD::Vertex* FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const;
  };
}

#endif
