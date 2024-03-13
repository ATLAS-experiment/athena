/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#ifndef JPSIXPLUSDISPLACED_H
#define JPSIXPLUSDISPLACED_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkV0Fitter/TrkV0VertexFitter.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>

namespace Trk {
    class IVertexFitter;
    class TrkVKalVrtFitter;
    class IVertexCascadeFitter;
    class VxCascadeInfo;
    class V0Tools;
    class ParticleDataTable;
}
namespace InDet { class VertexPointEstimator; }
namespace DerivationFramework {
    class CascadeTools;
}

namespace DerivationFramework {

  static const InterfaceID IID_JpsiXPlusDisplaced("JpsiXPlusDisplaced", 1, 0);

  class JpsiXPlusDisplaced : virtual public AthAlgTool, public IAugmentationTool
  {
  enum V0Enum{ UNKNOWN=0, LAMBDA=1, LAMBDABAR=2, KS=3 };
  public:
    static const InterfaceID& interfaceID() { return IID_JpsiXPlusDisplaced;}
    JpsiXPlusDisplaced(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~JpsiXPlusDisplaced() = default;
    virtual StatusCode initialize() override;
    StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, xAOD::VertexContainer* V0OutputContainer, xAOD::VertexContainer* disVtxOutputContainer) const;
    virtual StatusCode addBranches() const override;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexJXContainerKey;
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexV0ContainerKey;
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexDisVContainerKey;
    std::vector<std::string> m_vertexJXHypoNames;
    std::vector<std::string> m_vertexV0HypoNames;
    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_cascadeOutputKeys;
    bool m_refitV0;
    SG::WriteHandleKey<xAOD::VertexContainer> m_v0VtxOutputKey;
    SG::WriteHandleKey<xAOD::VertexContainer> m_disVtxOutputKey;
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrkParticleCollection;
    SG::ReadHandleKey<xAOD::VertexContainer> m_VxPrimaryCandidateName;
    SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName;
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key;
    SG::ReadDecorHandleKeyArray<xAOD::EventInfo> m_beamSpotDecoKeys;
    std::string m_hypoName;

    double m_jxMassLower;
    double m_jxMassUpper;
    double m_jpsiMassLower;
    double m_jpsiMassUpper;
    double m_diTrackMassLower;
    double m_diTrackMassUpper;
    std::string m_V0Hypothesis;
    double m_V0MassLower;
    double m_V0MassUpper;
    double m_lxyV0_cut;
    bool   m_doV0Enum;
    bool   m_decorV0P;
    double m_minMass_gamma;
    double m_chi2cut_gamma;
    double m_DisplacedMassLower;
    double m_DisplacedMassUpper;
    double m_lxyDisV_cut;
    double m_MassLower;
    double m_MassUpper;
    int    m_jxDaug_num;
    double m_jxDaug1MassHypo; // mass hypothesis of 1st daughter from vertex JX
    double m_jxDaug2MassHypo; // mass hypothesis of 2nd daughter from vertex JX
    double m_jxDaug3MassHypo; // mass hypothesis of 3rd daughter from vertex JX
    double m_jxDaug4MassHypo; // mass hypothesis of 4th daughter from vertex JX
    int    m_disVDaug_num;
    double m_disVDaug3MassHypo; // mass hypothesis of 3rd daughter from displaced vertex
    double m_extraTrkMassHypo;
    double m_extraTrkMinPt;
    double m_massJX;
    double m_massJpsi;
    double m_massX;
    double m_massDisV;
    double m_massV0;
    double m_massMainV;
    bool   m_constrJX;
    bool   m_constrJpsi;
    bool   m_constrX;
    bool   m_constrDisV;
    bool   m_constrV0;
    bool   m_constrMainV;
    bool   m_JXSubVtx;
    double m_chi2cut_JX;
    double m_chi2cut_V0;
    double m_chi2cut_DisV;
    double m_chi2cut;
    bool   m_useTRT;
    double m_ptTRT;
    double m_d0_cut;
    unsigned int m_maxJXCandidates;
    unsigned int m_maxV0Candidates;
    unsigned int m_maxDisVCandidates;
    unsigned int m_maxMainVCandidates;

    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Trk::TrkV0VertexFitter >            m_iV0Fitter;
    ToolHandle < Trk::IVertexFitter >                m_iGammaFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < Reco::ITrackToVertex >              m_trackToVertexTool;
    ToolHandle < Trk::ITrackSelectorTool >           m_trkSelector;
    ToolHandle < Trk::ITrackSelectorTool >           m_v0TrkSelector;
    ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;

    bool        m_refitPV;
    int         m_PV_max;
    size_t      m_PV_minNTracks;
    int         m_DoVertexType;

    double m_mass_e;
    double m_mass_mu;
    double m_mass_pion;
    double m_mass_proton;
    double m_mass_Lambda;
    double m_mass_Ks;
    double m_mass_Xi;
    double m_mass_Bpm;

    bool d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV, const Amg::Vector3D& beamspot) const;
    xAOD::Vertex* fitDisVtx(const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticle* track3, const xAOD::TrackParticleContainer* trackContainer) const;
    Trk::VxCascadeInfo* fitMainVtx(const xAOD::Vertex* JXvtx, std::vector<double>& massesJX, const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticleContainer* trackContainer) const;
    Trk::VxCascadeInfo* fitMainVtx(const xAOD::Vertex* JXvtx, std::vector<double>& massesJX, const xAOD::Vertex* disVtx, const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticleContainer* trackContainer) const;
    template<size_t NTracks> const xAOD::Vertex* FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const;
  };
}

#endif
