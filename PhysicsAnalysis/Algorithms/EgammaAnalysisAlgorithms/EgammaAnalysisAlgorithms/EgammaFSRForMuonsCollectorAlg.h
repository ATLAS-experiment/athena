/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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


namespace CP
{
    /// \brief Algorithm to collect photons and electrons which close in dR to muons as FSR candidates
    class EgammaFSRForMuonsCollectorAlg final : public EL::AnaAlgorithm
    {
    public:
        /// \brief the standard constructor
        EgammaFSRForMuonsCollectorAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator);

        StatusCode initialize() override;
        StatusCode execute() override;

    private:
        ///////////////////////////////////////////////////////////////////
        /** @brief Protected data:                                       */
        ///////////////////////////////////////////////////////////////////

        Gaudi::Property<float> m_dRMax{this, "deltaR_Max", 0.2, "DeltaR max for accepting a particle when comparing to compareParticles"}; 

        Gaudi::Property<std::string> m_passWPorFSRName{this, "passWPorFSRName", "passWPorFSR", "Name for decoration for electron or photon which either passes their corresponding WP selection or the dR selection for a muon FSR."}; 

        /// \brief the systematics list we run
        SysListHandle m_systematicsList {this};

        SysReadHandle<xAOD::IParticleContainer> m_egammaContKey{this, "ElectronOrPhotonContKey", "", "Electrons or photons for dR comparison"}; 

        SysReadHandle<xAOD::MuonContainer> m_muonContKey{this, "MuonContKey", "AnalysisMuons", "Muons to compare with for selecting FSR"}; 

        Gaudi::Property<std::string> m_selectionName {this, "selectionDecoration", "", "the decoration for the combined WP and FSR selection"};

        Gaudi::Property<bool> m_vetoFSR {this, "vetoFSR", false, "boolean to revert FSR logic to rather veto FSR electrons or photons"};

        /// Decorator for electron or photon working point - used to add additional el/ph
        std::unique_ptr<SG::AuxElement::Decorator<uint32_t> > m_wpDec;

    };
}
#endif