/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORKMUONS_PFLOWISOLATIONDECORALG_H
#define DERIVATIONFRAMEWORKMUONS_PFLOWISOLATIONDECORALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteDecorHandleKey.h>
#include <xAODPrimitives/IsolationType.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <RecoToolInterfaces/INeutralEFlowIsolationTool.h>

/// Algorithm to decorate the calorimeter isolation variables to the track particles

namespace DerivationFramework {
    class PflowIsolationDecorAlg : public AthReentrantAlgorithm {
    public:
        /// Constructor with parameters:
        PflowIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator);

        /// Destructor:
        ~PflowIsolationDecorAlg() = default;

        /// Athena algorithm's Hooks
        StatusCode initialize() override;
        StatusCode execute(const EventContext& ctx) const override;

    private:
        /// Athena configured tools
        ToolHandle<xAOD::INeutralEFlowIsolationTool> m_isoTool{this, "IsolationTool", ""};
        xAOD::CaloCorrection m_calo_corr;

        /// track collection to decorate
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trk_key{this, "TrackCollection", "InDetTrackParticles"};

        /// pt threshold to apply
        Gaudi::Property<float> m_pt_min{this, "PtMin", 3. * Gaudi::Units::GeV};

        Gaudi::Property<std::string> m_customName{this, "customName", ""};

        /// Optional list of decorators to select only the good tracks for the isolation decoration. Only one decorator needs
        /// to pass to launch the isolation calculation
        Gaudi::Property<std::vector<std::string>> m_trkSel_Decors{this, "TrackSelections", {},
                                                  "List of decorator names of which one needs to be true to run the isolation" };
        SG::ReadDecorHandleKeyArray<xAOD::TrackParticleContainer> m_trkSel_keys{this, "SelectionKeys", {},
                                                                                "Will be overwritten in initialize"};

        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_neflowCone20_key{this, "NeflowCone20Key", m_trk_key, "",
                                                                               "Will be overwritten in initialize"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_neflowCone30_key{this, "NeflowCone30Key", m_trk_key, "",
                                                                               "Will be overwritten in initialize"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_neflowCone40_key{this, "NeflowCone40Key", m_trk_key, "",
                                                                               "Will be overwritten in initialize"};
       
        std::vector<xAOD::Iso::IsolationType> m_pflow_isos{xAOD::Iso::neflowisol40, xAOD::Iso::neflowisol30, xAOD::Iso::neflowisol20};
    };
}
#endif