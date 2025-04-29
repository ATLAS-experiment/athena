/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   ElectronSiHitDecAlg
//   Author: RD Schaffer, R.D.Schaffer@cern.ch
//   Algorithm to decorate SiHit electrons with extra information
//   from tracks and clusters to avoid writing them out
///////////////////////////////////////////////////////////////////

#ifndef ASG_ANALYSIS_ALGORITHMS__ELECTRON_SIHIT_DEC_ALGORITHM__H
#define ASG_ANALYSIS_ALGORITHMS__ELECTRON_SIHIT_DEC_ALGORITHM__H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include <xAODEgamma/ElectronContainer.h>
#include "xAODMuon/MuonContainer.h"
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>


namespace CP
{
    /// \brief this decorates electrons with extra information from the track and cluster to avoid writing them out for SiHit electrons
    class ElectronSiHitDecAlg final : public EL::AnaAlgorithm
    {
        /// \brief the standard constructor
    public:
        ElectronSiHitDecAlg(const std::string &name,
                            ISvcLocator *pSvcLocator);

        StatusCode initialize() override;
        StatusCode execute() override;

    private:

        /// \brief the systematics list we run
        SysListHandle m_systematicsList {this};

        // EventInfo key
        SysReadHandle<xAOD::EventInfo> m_eventInfoKey{this, "EventInfo", "EventInfo", "Input event information"};

        // Primary vertex key
        SysReadHandle<xAOD::VertexContainer> m_vertexKey{this, "VertexContainer", "PrimaryVertices", "Primary vertex container"};

        // Electron container key
        SysReadHandle<xAOD::ElectronContainer> m_electronContainerKey{this, "ElectronContainer", "AnalysisSiHitElectrons", "Sihit electrons to decorate"};

        // Muon container key for event selection
        SysReadHandle<xAOD::MuonContainer> m_analMuonContKey{this, "AnalMuonContKey", "AnalysisMuons", "Muons use for event selection to decorate SiHit electrons"};

        // Electron container key for event selection
        SysReadHandle<xAOD::ElectronContainer> m_analElectronContKey{this, "AnalElectronContKey", "AnalysisElectrons", "Electrons use for event selection to decorate SiHit electrons"};

        // Apply event veto - require at least two muons or electrons to be present
        Gaudi::Property<bool> m_requireTwoLeptons {this, "RequireTwoLeptons", true, "boolean to select events with at least a pair of electrons or muons which pass the basic cuts, i.e. are in their corresponding analysis containers"};

        /// Decorators for the extra information from clusters and tracks for SiHit electrons
        CP::SysWriteDecorHandle<float>  m_z0stheta{this, "z0stheta", "z0stheta", "the decoration for z0 x sin(theta) of electron track"};
        CP::SysWriteDecorHandle<float>  m_d0Normalized{this, "d0Normalized", "d0Normalized", "the decoration for d0Normalized of the electron track"};
        CP::SysWriteDecorHandle<int>    m_nInnerExpPix{this, "nInnerExpPix", "nInnerExpPix", "the decoration for number of inner pixel hits of the electron track"};
        CP::SysWriteDecorHandle<float>  m_clEta{this, "clEta", "clEta", "the decoration for eta of the electron cluster"};
        CP::SysWriteDecorHandle<float>  m_clPhi{this, "clPhi", "clPhi", "the decoration for phi of the electron cluster"};

        /// Decorator for SiHit electron for event requirement on a pair of leptons
        CP::SysWriteDecorHandle<char> m_evtOKDec{this, "selectionName", "", "the decoration for the combined WP and FSR selection"};

    };
}
#endif
