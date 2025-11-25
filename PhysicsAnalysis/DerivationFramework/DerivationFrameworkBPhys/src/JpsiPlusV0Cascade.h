/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef JPSIPLUSV0CASCADE_H
#define JPSIPLUSV0CASCADE_H
//*********************
// JpsiPlusV0Cascade header file
//
// Eva Bouhova <e.bouhova@cern.ch>
// Adam Barton <abarton@cern.ch>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IPartPropSvc.h"

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include <vector>
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
// dummy EventContext for AnalysisBase
#include "AsgTools/CurrentContext.h"

namespace Trk {
  class IVertexFitter;
  class TrkVKalVrtFitter;
  class IVertexCascadeFitter;
  class VxCascadeInfo;
  class V0Tools;
}

namespace DerivationFramework {
  class CascadeTools;
}


namespace DerivationFramework {

  class JpsiPlusV0Cascade : public extends<AthAlgTool, IAugmentationTool>
  {
  public:
    JpsiPlusV0Cascade(const std::string& t, const std::string& n, const IInterface*  p);
    ~JpsiPlusV0Cascade();
    virtual StatusCode initialize() override;
    virtual StatusCode addBranches(const EventContext & ctx) const override;
  private:
    StatusCode performSearch(std::vector<Trk::VxCascadeInfo*>& cascadeinfoContainer, const EventContext& ctx ) const;

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{this, "JpsiVertices", ""};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexV0ContainerKey{this, "V0Vertices", ""};

    SG::ReadHandleKey<xAOD::VertexContainer>  m_VxPrimaryCandidateName{this, "VxPrimaryCandidateName", "PrimaryVertices"};   //!< Name of primary vertex container
    SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_RelinkContainers{this, "RelinkTracks", {}, "Track Containers if they need to be relinked through indirect use" };

    SG::WriteHandleKeyArray<xAOD::VertexContainer> m_cascadeOutputsKeys{this, "CascadeVertexCollections", {"JpsiPlusV0CascadeVtx1", "JpsiPlusV0CascadeVtx2"} };

    Gaudi::Property<double> m_jpsiMassLower{this, "JpsiMassLowerCut", 0.0};
    Gaudi::Property<double> m_jpsiMassUpper{this, "JpsiMassUpperCut", 10000.0};
    Gaudi::Property<double> m_V0MassLower{this, "V0MassLowerCut", 0.0};
    Gaudi::Property<double> m_V0MassUpper{this, "V0MassUpperCut", 10000.0};
    Gaudi::Property<double> m_MassLower{this, "MassLowerCut", 0.0};
    Gaudi::Property<double> m_MassUpper{this, "MassUpperCut", 20000.0};
    Gaudi::Property<int> m_v0_pid{this, "V0Hypothesis", 310};
    Gaudi::Property<bool> m_constrV0{this, "ApplyV0MassConstraint", true};
    Gaudi::Property<bool> m_constrJpsi{this, "ApplyJpsiMassConstraint", true};

    PublicToolHandle < Trk::TrkVKalVrtFitter > m_iVertexFitter{this, "TrkVertexFitterTool", "Trk::TrkVKalVrtFitter"};
    ToolHandle < Analysis::PrimaryVertexRefitter > m_pvRefitter{this, "PVRefitter", "Analysis::PrimaryVertexRefitter"}; // private tool
    PublicToolHandle < Trk::V0Tools > m_V0Tools{this, "V0Tools", "Trk::V0Tools"};
    PublicToolHandle < DerivationFramework::CascadeTools > m_CascadeTools{this, "CascadeTools", "DerivationFramework::CascadeTools"};
    ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};

    Gaudi::Property<int> m_jpsi_trk_pdg{this, "JpsiTrackPDGID", 13}; // PDG ID for J/psi tracks, can be either 11 or 13
    Gaudi::Property<bool> m_refitPV{this, "RefitPV",  true};
    SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName{this, "RefPVContainerName", "RefittedPrimaryVertices"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_jpsiTrackContainerName{this, "JpsiTrackContainerName", "InDetTrackParticles"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_v0TrackContainerName{this, "V0TrackContainerName", "InDetTrackParticles"};
    Gaudi::Property<std::string> m_hypoName{this, "HypothesisName", "Bd"}; //!< name of the mass hypothesis. E.g. Jpis, Upsi, etc. Will be used as a prefix for decorations
    //This parameter will allow us to optimize the number of PVs under consideration as the probability
    //of a useful primary vertex drops significantly the higher you go
    Gaudi::Property<int> m_PV_max{this, "MaxnPV", 999};
    Gaudi::Property<int> m_DoVertexType{this, "DoVertexType", 7};
    Gaudi::Property<size_t> m_PV_minNTracks{this, "MinNTracksInPV", 0};

    // Locally cached particle mass constants
    double m_mass_electron{0.};
    double m_mass_muon{0.};
    double m_mass_pion{0.};
    double m_mass_proton{0.};
    double m_mass_lambda{0.};
    double m_mass_ks{0.};
    double m_mass_jpsi{0.};
    double m_mass_b0{0.};
    double m_mass_lambdaB{0.};
  };
}


#endif
