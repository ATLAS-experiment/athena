/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_TESTISOLATIONCLOSEBYCORRATHENAALG_H
#define ISOLATIONSELECTION_TESTISOLATIONCLOSEBYCORRATHENAALG_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

// Local include(s):
#include <IsolationSelection/IIsolationCloseByCorrectionTool.h>
#include <IsolationSelection/IIsolationSelectionTool.h>
#include <IsolationSelection/TestMacroHelpers.h>

#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODCore/ShallowCopy.h"

/**
 * @brief Simple algorithm to check the performance of the IsolationCloseByCorrectionTool. The algorithm writes TTrees that can be analyzed
 * by the python scripts in the package. The information written are
 *              -- particle four momentum
 *              -- pt of the assocaited ID track (if available)
 *              -- clusterEta/clusterphi of the associated cluster (if available)
 *              -- isolation variables before and after the correction
 */

namespace CP {
    class ATLAS_NOT_THREAD_SAFE TestIsolationCloseByCorrAlg : public AthHistogramAlgorithm {
    public:
        TestIsolationCloseByCorrAlg(const std::string& name, ISvcLocator* svcLoc);
        virtual ~TestIsolationCloseByCorrAlg() = default;
        StatusCode initialize() override;
        StatusCode execute(const EventContext& ctx) override;
        StatusCode finalize() override;

        unsigned int cardinality() const override { return 1; }

    private:
        template <class TARGET_TYPE, class CONT_TYPE, class COPY_TYPE>
        StatusCode loadContainer(const EventContext& ctx, const SG::ReadHandleKey<CONT_TYPE>& key, 
                                 std::pair<std::unique_ptr<COPY_TYPE>,
                                 std::unique_ptr<xAOD::ShallowAuxContainer>>& cont) const;

        bool passSelection(const EventContext& ctx, const xAOD::Muon* muon) const;
        bool passSelection(const EventContext& ctx, const xAOD::Egamma* egamm) const;

        /// Input containers
        SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonContainer", ""};
        SG::ReadHandleKey<xAOD::EgammaContainer> m_elecKey{this, "EleContainer", ""};
        SG::ReadHandleKey<xAOD::EgammaContainer> m_photKey{this, "PhotContainer", ""};
        /// Optionally the algorithm can test the behaviour of the tracks selected by the IsoCloseByCorrectionTrkSelAlg
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_polTrkKey{this, "TrackKey", ""};

        Gaudi::Property<std::string> m_selDecoration{this, "SelectionDecorator", "",
                                                     "Optional char decorator flag that the leptons have to pass in order to be selected"};

        Gaudi::Property<std::string> m_isoDecoration{this, "IsolationDecorator", "", "Decoration of the first isolation selection pass."};
        Gaudi::Property<std::string> m_updatedIsoDeco{this, "UpdatedIsoDecorator", "", "Decoration of the first isolation selection pass."};
        Gaudi::Property<std::string> m_backup_prefix{
            this, "BackupPrefix", "", "Prefix in front of the isolation variables, if the original cone values need  to  be backuped"};

        SelectionDecorator m_selDecorator{nullptr};
        SelectionDecorator m_isoDecorator{nullptr};

        /// Optionally the user can also parse the elec / muon / photon selection tools
        ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "MuonSelectionTool", ""};
        ToolHandle<IAsgElectronLikelihoodTool> m_elecSelTool{this, "ElectronSelectionTool", ""};
        ToolHandle<IAsgPhotonIsEMSelector> m_photSelTool{this, "PhotonSelectionTool", ""};

        const CP::IsolationCloseByCorrectionTool* correction_tool() const;

        ToolHandle<CP::IIsolationCloseByCorrectionTool> m_isoCloseByCorrTool{this, "IsoCloseByCorrTool", ""};
        ToolHandle<CP::IIsolationSelectionTool> m_isoSelectorTool{this, "IsolationSelectionTool", ""};

        MuonVal::MuonTesterTree m_tree{"IsoCorrTest", "/ISOCORRECTION"};
        std::shared_ptr<IsoCorrectionTestHelper> m_ele_helper{nullptr};
        std::shared_ptr<IsoCorrectionTestHelper> m_muo_helper{nullptr};
        std::shared_ptr<IsoCorrectionTestHelper> m_pho_helper{nullptr};

        Gaudi::Property<float> m_mu_min_pt{this, "MuonPt", 5 * Gaudi::Units::GeV};
        Gaudi::Property<float> m_mu_max_eta{this, "MuonEta", 2.7};
        Gaudi::Property<float> m_el_min_pt{this, "ElectronPt", 7. * Gaudi::Units::GeV};
        Gaudi::Property<float> m_el_max_eta{this, "ElectronEta", 2.47};
        Gaudi::Property<float> m_ph_min_pt{this, "PhotonPt", 25. * Gaudi::Units::GeV};
        Gaudi::Property<float> m_ph_max_eta{this, "PhotonEta", 2.35};

        Gaudi::Property<bool> m_isMC{this, "isMC", false};
    };

}  // namespace CP
#endif
