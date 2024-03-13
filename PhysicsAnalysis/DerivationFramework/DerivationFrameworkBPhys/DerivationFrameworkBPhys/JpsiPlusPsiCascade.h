/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#ifndef JPSIPLUSPSICASCADE_H
#define JPSIPLUSPSICASCADE_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
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

namespace DerivationFramework {
    class CascadeTools;
}

namespace DerivationFramework {

  static const InterfaceID IID_JpsiPlusPsiCascade("JpsiPlusPsiCascade", 1, 0);

  class JpsiPlusPsiCascade : virtual public AthAlgTool, public IAugmentationTool
  {
  public:
    static const InterfaceID& interfaceID() { return IID_JpsiPlusPsiCascade;}
    JpsiPlusPsiCascade(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~JpsiPlusPsiCascade() = default;
    virtual StatusCode initialize() override;
    StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer_noConstr) const;
    virtual StatusCode addBranches() const override;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey;
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexPsiContainerKey;
    std::vector<std::string> m_vertexJpsiHypoNames;
    std::vector<std::string> m_vertexPsiHypoNames;
    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_cascadeOutputsKeys;
    SG::ReadHandleKey<xAOD::VertexContainer> m_VxPrimaryCandidateName; //!< Name of primary vertex container
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackContainerName;
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key;

    double m_jpsiMassLower;
    double m_jpsiMassUpper;
    double m_diTrackMassLower;
    double m_diTrackMassUpper;
    double m_psiMassLower;
    double m_psiMassUpper;
    double m_jpsi2MassLower;
    double m_jpsi2MassUpper;
    double m_MassLower;
    double m_MassUpper;
    int    m_vtx1Daug_num;
    double m_vtx1Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 1
    double m_vtx1Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 1
    double m_vtx1Daug3MassHypo; // mass hypothesis of 3rd daughter from vertex 1
    double m_vtx1Daug4MassHypo; // mass hypothesis of 4th daughter from vertex 1
    double m_vtx2Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 2
    double m_vtx2Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 2

    double m_mass_jpsi;
    double m_mass_diTrk;
    double m_mass_psi;
    double m_mass_jpsi2;
    bool   m_constrPsi;
    bool   m_constrJpsi;
    bool   m_constrDiTrk;
    bool   m_constrJpsi2;
    double m_chi2cut_Psi;
    double m_chi2cut_Jpsi;
    double m_chi2cut;
    unsigned int m_maxCandidates;

    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;

    bool        m_refitPV;
    SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName;
    std::string m_hypoName;
    int         m_PV_max;
    int         m_DoVertexType;
    size_t      m_PV_minNTracks;
  };
}

#endif
