/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   EgammaFSRForMuonsCollectorAlg
//   Author: RD Schaffer, R.D.Schaffer@cern.ch
//   Algorithm to collect photons and electrons which close in dR
//   to muons as FSR candidates
///////////////////////////////////////////////////////////////////

#ifndef ASG_ANALYSIS_ALGORITHMS_EGAMMA_FSR_FOR_MUONS_COLLECTOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS_EGAMMA_FSR_FOR_MUONS_COLLECTOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <xAODBase/IParticleContainer.h>
#include "xAODMuon/MuonContainer.h"
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>


namespace CP
{
    /// @brief Algorithm to collect photons and electrons which close in dR to muons as FSR candidates
    class EgammaFSRForMuonsCollectorAlg final : public EL::AnaAlgorithm
    {
    public:
        /// @brief the standard constructor
        using EL::AnaAlgorithm::AnaAlgorithm;

        StatusCode initialize() override;
        StatusCode execute(const EventContext& ctx) override;

    private:
        ///////////////////////////////////////////////////////////////////
        /** @brief Protected data:                                       */
        ///////////////////////////////////////////////////////////////////

        Gaudi::Property<float> m_dRMax{this, "deltaR_Max", 0.2, "DeltaR max for accepting a particle when comparing to compareParticles"};

        /// @brief the systematics list we run
        SysListHandle m_systematicsList {this};

        SysReadHandle<xAOD::IParticleContainer> m_egammaContKey{this, "ElectronOrPhotonContKey", "", "Electrons or photons for dR comparison"};

        SysReadHandle<xAOD::MuonContainer> m_muonContKey{this, "MuonContKey", "AnalysisMuons", "Muons to compare with for selecting FSR"};

        /// @brief the input WP selection to combine with FSR
        SysReadSelectionHandle m_wpSelection{this, "wpSelection", "", "the input WP selection to OR with FSR"};

        /// @brief the output combined WP||FSR selection
        SysWriteDecorHandle<char> m_outputDec{this, "selectionDecoration", "", "the output combined WP||FSR selection"};

        Gaudi::Property<bool> m_vetoFSR {this, "vetoFSR", false, "boolean to revert FSR logic to rather veto FSR electrons or photons"};

    };
}
#endif
