/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUREC_TAUAODRUNNERALG_H
#define TAUREC_TAUAODRUNNERALG_H

#include "tauRecTools/ITauToolBase.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODPFlow/PFOAuxContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODParticleEvent/ParticleContainer.h"
#include "xAODParticleEvent/ParticleAuxContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODTau/TauDefs.h"
#include "GaudiKernel/ToolHandle.h"

/**
 *  @brief  The implementation of the TauAODRunnerAlg, which is meant to run at AOD level.
 *  This algorithm reads in the AOD TauJets and TauTracks containers, as well as other tau 
 *  related containers, and make deep copies. The tools scheduled for this algorithm were 
 *  divided into two categories, modification tools and standard tools after the 
 *  modification. The algorithm does not proceed to the standard tools if the tau object was 
 *  not modified by the modification tools, unless no modification tool has been provided.
 *  The example python scheduling scripts can be found in the DerivationFramework package.
 *
 *  Caution! The tool can be configured to only deep-copy the TauJet container (by setting
 *  the other input/output keys to "", but this will restrict the tools you can run, to
 *  only those that decorate/modify the TauJets, and none of the linked objects (e.g.
 *  vertices, tracks, PFOs, etc). Otherwise, errors and/or crashes will happen.
 */
class TauAODRunnerAlg: public AthReentrantAlgorithm {
    public:
        TauAODRunnerAlg(const std::string &name, ISvcLocator *);
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        // Tool handle arrays
        ToolHandleArray<ITauToolBase>  m_modificationTools{this, "modificationTools", {}, "Tools for modifying the taus"};
        ToolHandleArray<ITauToolBase>  m_officialTools    {this, "officialTools",     {}, "Official Reconstruction tools for taus after the modifications"};

        // Read and write keys
        SG::ReadHandleKey<xAOD::TauJetContainer>        m_tauContainer              {this, "Key_tauContainer",                  "TauJets",                      "input tau key"};
        SG::ReadHandleKey<xAOD::CaloClusterContainer>   m_pi0ClusterInputContainer  {this, "Key_pi0ClusterInputContainer",      "TauPi0Clusters",               "input pi0 cluster"};
        SG::WriteHandleKey<xAOD::TauJetContainer>       m_tauOutContainer           {this, "Key_tauOutputContainer",            "TauJets_AODReco",              "output tau key"};
        SG::WriteHandleKey<xAOD::ParticleContainer>     m_pi0Container              {this, "Key_pi0OutputContainer",            "TauFinalPi0s_AODReco",         "output tau final pi0 key"};
        SG::WriteHandleKey<xAOD::PFOContainer>          m_neutralPFOOutputContainer {this, "Key_neutralPFOOutputContainer",     "TauNeutralPFOs_AODReco",       "output tau neutral pfo key"};
        SG::WriteHandleKey<xAOD::PFOContainer>          m_chargedPFOOutputContainer {this, "Key_chargedPFOOutputContainer",     "TauChargedPFOs_AODReco",       "output tau charged pfo key"};
        SG::WriteHandleKey<xAOD::PFOContainer>          m_hadronicPFOOutputContainer{this, "Key_hadronicPFOOutputContainer",    "TauHadronicPFOs_AODReco",      "output tau hadronic pfo key"};
        SG::WriteHandleKey<xAOD::TauTrackContainer>     m_tauTrackOutputContainer   {this, "Key_tauTrackOutputContainer",       "TauTracks_AODReco",            "output tau track key"};
        SG::WriteHandleKey<xAOD::VertexContainer>       m_vertexOutputContainer     {this, "Key_vertexOutputContainer",         "TauSecondaryVertices_AODReco", "output vertex container key"};

        // Helper
        static bool isTauModified(const xAOD::TauJet* newtau) ;
};

#endif // TAUREC_TAUAODRUNNERALG_H
