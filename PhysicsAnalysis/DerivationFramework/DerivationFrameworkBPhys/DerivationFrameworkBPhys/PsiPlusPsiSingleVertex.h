/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PSIPLUSPSISINGLEVERTEX_H
#define PSIPLUSPSISINGLEVERTEX_H
// Xin Chen <xin.chen@cern.ch>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>

namespace Trk {
    class TrkVKalVrtFitter;
    class V0Tools;
    class ParticleDataTable;
}

namespace DerivationFramework {

  static const InterfaceID IID_PsiPlusPsiSingleVertex("PsiPlusPsiSingleVertex", 1, 0);

  class PsiPlusPsiSingleVertex : virtual public AthAlgTool, public IAugmentationTool
  {
  public:
    static const InterfaceID& interfaceID() { return IID_PsiPlusPsiSingleVertex;}
    PsiPlusPsiSingleVertex(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~PsiPlusPsiSingleVertex() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode addBranches() const override;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexPsi1ContainerKey;
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexPsi2ContainerKey;
    std::vector<std::string> m_vertexPsi1HypoNames;
    std::vector<std::string> m_vertexPsi2HypoNames;
    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_outputsKeys; // StoreGate/WriteHandleKeyArray.h
    SG::ReadHandleKey<xAOD::VertexContainer> m_VxPrimaryCandidateName; //!< Name of primary vertex container
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackContainerName;
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key;

    double m_jpsi1MassLower;
    double m_jpsi1MassUpper;
    double m_diTrack1MassLower;
    double m_diTrack1MassUpper;
    double m_psi1MassLower;
    double m_psi1MassUpper;
    double m_jpsi2MassLower;
    double m_jpsi2MassUpper;
    double m_diTrack2MassLower;
    double m_diTrack2MassUpper;
    double m_psi2MassLower;
    double m_psi2MassUpper;
    double m_MassLower;
    double m_MassUpper;
    int    m_vtx1Daug_num;
    double m_vtx1Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 1
    double m_vtx1Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 1
    double m_vtx1Daug3MassHypo; // mass hypothesis of 3rd daughter from vertex 1
    double m_vtx1Daug4MassHypo; // mass hypothesis of 4th daughter from vertex 1
    int    m_vtx2Daug_num;
    double m_vtx2Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 2
    double m_vtx2Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 2
    double m_vtx2Daug3MassHypo; // mass hypothesis of 3rd daughter from vertex 2
    double m_vtx2Daug4MassHypo; // mass hypothesis of 4th daughter from vertex 2
    unsigned int    m_maxCandidates;

    double m_mass_jpsi1;
    double m_mass_diTrk1;
    double m_mass_psi1;
    double m_mass_jpsi2;
    double m_mass_diTrk2;
    double m_mass_psi2;
    bool   m_constrJpsi1;
    bool   m_constrDiTrk1;
    bool   m_constrPsi1;
    bool   m_constrJpsi2;
    bool   m_constrDiTrk2;
    bool   m_constrPsi2;
    double m_chi2cut;

    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;

    bool        m_refitPV;
    SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName;
    std::string m_hypoName;
    int         m_PV_max;
    int         m_DoVertexType;
    size_t      m_PV_minNTracks;
  };
}

#endif
