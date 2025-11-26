/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuPlusDpstCascade_H
#define MuPlusDpstCascade_H
//*********************
// MuPlusDpstCascade header file
//
// Eva Bouhova <e.bouhova@cern.ch>
// Adam Barton <abarton@cern.ch>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include <vector>
#include "xAODEventInfo/EventInfo.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "GaudiKernel/IPartPropSvc.h"
// dummy EventContext for AnalysisBase
#include "AsgTools/CurrentContext.h"

namespace Trk {
    class IVertexFitter;
    class TrkVKalVrtFitter;
    class IVertexCascadeFitter;
    class VxCascadeInfo;
    class V0Tools;
    class VxTrackAtVertex;
}

namespace DerivationFramework {
    class CascadeTools;
}

namespace DerivationFramework {

    class MuPlusDpstCascade : public extends<AthAlgTool, IAugmentationTool>
    {
      public:
        MuPlusDpstCascade(const std::string& t, const std::string& n, const IInterface*  p);
        ~MuPlusDpstCascade();
        virtual StatusCode initialize() override;
        StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, const EventContext& ctx ) const;
        virtual StatusCode addBranches(const EventContext& ctx) const override;

      private:
        std::string m_vertexContainerKey; // FIXME Use Handles
        std::string m_vertexD0ContainerKey; // FIXME Use Handles
        std::vector<std::string> m_cascadeOutputsKeys; // FIXME Use Handles

        std::string m_VxPrimaryCandidateName;   //!< Name of primary vertex container // FIXME Use Handles

        double m_MuPiMassLower;
        double m_MuPiMassUpper;
        double m_D0MassLower;
        double m_D0MassUpper;
        double m_DstMassLower;
        double m_DstMassUpper;
        double m_DstMassUpperAft; //mass cut after cascade fit
        double m_MassLower;
        double m_MassUpper;
        double m_vtx0MassHypo;      // mass hypothesis of vertex 0
        double m_vtx1MassHypo;      // mass hypothesis of vertex 1
        double m_vtx0Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 0
        double m_vtx0Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 0
        double m_vtx1Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 1
        double m_vtx1Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 1


        int    m_Dx_pid;
        bool   m_constrD0;
        bool   m_constrMuPi;
        double m_chi2cut;

        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
        ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
        ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
        ToolHandle < Trk::V0Tools >                      m_V0Tools;
        ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;
        std::unique_ptr<InDet::InDetTrackSelectionTool> m_trackSelectionTools;
        ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};

        bool        m_refitPV;
        std::string m_refPVContainerName;
        std::string m_hypoName;               //!< name of the mass hypothesis. E.g. Jpsi, Upsi, etc. Will be used as a prefix for decorations
        //This parameter will allow us to optimize the number of PVs under consideration as the probability
        //of a useful primary vertex drops significantly the higher you go
        int         m_PV_max;
        int         m_DoVertexType;
        size_t      m_PV_minNTracks;

    };
}

#endif
