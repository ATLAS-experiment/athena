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

        std::string m_vertexContainerKey;
        std::string m_vertexV0ContainerKey;
        std::vector<std::string> m_cascadeOutputsKeys;

        std::string   m_VxPrimaryCandidateName;   //!< Name of primary vertex container

        double m_jpsiMassLower;
        double m_jpsiMassUpper;
        double m_V0MassLower;
        double m_V0MassUpper;
        double m_MassLower;
        double m_MassUpper;

        double m_mass_electron;
        double m_mass_muon;
        double m_mass_pion;
        double m_mass_proton;
        double m_mass_lambda;
        double m_mass_ks;
        double m_mass_jpsi;
        double m_mass_b0;
        double m_mass_lambdaB;
        int m_v0_pid;
        bool m_constrV0;
        bool m_constrJpsi;

        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
        ToolHandle < Trk::TrkVKalVrtFitter > m_iVertexFitter;
        ToolHandle < Analysis::PrimaryVertexRefitter > m_pvRefitter;
        ToolHandle < Trk::V0Tools > m_V0Tools;
        ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;
        ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};

        int         m_jpsi_trk_pdg; // PDG ID for J/psi tracks, can be either 11 or 13
        bool        m_refitPV;
        Gaudi::Property<std::string>  m_refPVContainerName;
        Gaudi::Property<std::string> m_jpsiTrackContainerName;
        Gaudi::Property<std::string> m_v0TrackContainerName;
        std::string m_hypoName;               //!< name of the mass hypothesis. E.g. Jpis, Upsi, etc. Will be used as a prefix for decorations
        //This parameter will allow us to optimize the number of PVs under consideration as the probability
        //of a useful primary vertex drops significantly the higher you go
        int         m_PV_max;
        int         m_DoVertexType;
        size_t      m_PV_minNTracks;

    public:
        JpsiPlusV0Cascade(const std::string& t, const std::string& n, const IInterface*  p);
        ~JpsiPlusV0Cascade();
        StatusCode initialize() override;
        StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, const EventContext& ctx ) const;
        virtual StatusCode addBranches(const EventContext& ctx) const override;
        SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_RelinkContainers{this, "RelinkTracks", {}, "Track Containers if they need to be relinked through indirect use" };

    };
}


#endif

