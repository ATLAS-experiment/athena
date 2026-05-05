/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

// Local include(s):
#include <IsolationSelection/Defs.h>
#include <IsolationSelection/IsoVariableHelper.h>
#include <IsolationSelection/TestMacroHelpers.h>
#include <StoreGate/ReadHandle.h>

#include "TestIsolationCloseByCorrAlg.h"

namespace CP {

    TestIsolationCloseByCorrAlg::TestIsolationCloseByCorrAlg(const std::string& name, ISvcLocator* svcLoc) :
        AthHistogramAlgorithm(name, svcLoc) {}

    StatusCode TestIsolationCloseByCorrAlg::initialize() {
        ATH_CHECK(m_isoSelectorTool.retrieve());
        ATH_CHECK(m_isoCloseByCorrTool.retrieve());

        ATH_CHECK(m_muonKey.initialize(!m_muonKey.empty()));
        ATH_CHECK(m_elecKey.initialize(!m_elecKey.empty()));
        ATH_CHECK(m_photKey.initialize(!m_photKey.empty()));
        ATH_CHECK(m_polTrkKey.initialize(!m_polTrkKey.empty()));
        using wOpts = MuonVal::EventInfoBranch::WriteOpts;
        unsigned int writeOpts = wOpts::writePileUp | wOpts::writeBeamSpot;
        if (m_isMC) writeOpts |= wOpts::isMC;
        m_tree.addBranch(std::make_shared<MuonVal::EventInfoBranch>(m_tree, writeOpts));

        auto add_correctionHelper = [this](std::shared_ptr<IsoCorrectionTestHelper> helper) {
            if (!m_selDecoration.empty()) helper->SetSelectionDecorator(m_selDecoration.value());
            if (!m_isoDecoration.empty()) helper->SetIsolationDecorator(m_isoDecoration.value());
            if (!m_backup_prefix.empty()) helper->SetBackupPreFix(m_backup_prefix.value());
            if (!m_updatedIsoDeco.empty()) helper->SetUpdatedIsoDecorator(m_updatedIsoDeco.value());
            m_tree.addBranch(helper);
        };
        if (!m_elecKey.empty()) {
            m_ele_helper = std::make_shared<IsoCorrectionTestHelper>(m_tree, "Electrons", m_isoSelectorTool->getElectronWPs());
            add_correctionHelper(m_ele_helper);
        }
        if (!m_muonKey.empty()) {
            m_muo_helper = std::make_shared<IsoCorrectionTestHelper>(m_tree, "Muons", m_isoSelectorTool->getMuonWPs());
            add_correctionHelper(m_muo_helper);
        }
        if (!m_photKey.empty()) {
            m_pho_helper = std::make_shared<IsoCorrectionTestHelper>(m_tree, "Photons", m_isoSelectorTool->getPhotonWPs());
            add_correctionHelper(m_pho_helper);
        }
        if (!m_muonSelTool.empty()) ATH_CHECK(m_muonSelTool.retrieve());
        if (!m_elecSelTool.empty()) ATH_CHECK(m_elecSelTool.retrieve());
        if (!m_photSelTool.empty()) ATH_CHECK(m_photSelTool.retrieve());
        if (!m_selDecoration.empty()) m_selDecorator = std::make_unique<CharDecorator>(m_selDecoration);
        if (!m_isoDecoration.empty()) m_isoDecorator = std::make_unique<CharDecorator>(m_isoDecoration);
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode TestIsolationCloseByCorrAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    template <class TARGET_TYPE, class CONT_TYPE, class COPY_TYPE>
    StatusCode TestIsolationCloseByCorrAlg::loadContainer(const EventContext& ctx,
                                                          const SG::ReadHandleKey<CONT_TYPE>& key,
                                                          std::pair<std::unique_ptr<COPY_TYPE>,
                                                          std::unique_ptr<xAOD::ShallowAuxContainer>>& cont) const {
        if (key.empty()) {
            ATH_MSG_DEBUG("No key given. Assume it's no required to load the container");
            return StatusCode::SUCCESS;
        }
        SG::ReadHandle<CONT_TYPE> readHandle{key, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load container " << key.fullKey());
            return StatusCode::FAILURE;
        }

        cont = xAOD::shallowCopy(dynamic_cast<const TARGET_TYPE&> (*readHandle),ctx);
        if (!m_selDecorator && !m_isoDecorator) return StatusCode::SUCCESS;
        std::unique_ptr<COPY_TYPE>& elems = cont.first;
        for (auto part : *(elems.get()) ) {
            if (m_selDecorator) (*m_selDecorator)(*part) = passSelection(ctx, part);
            if (m_isoDecorator) (*m_isoDecorator)(*part) = true && m_isoSelectorTool->accept(*part);
        }

        return StatusCode::SUCCESS;
    }

    bool TestIsolationCloseByCorrAlg::passSelection(const EventContext&, const xAOD::Muon* muon) const {
        return muon->pt() >= m_mu_min_pt && (m_mu_max_eta < 0. || std::abs(muon->eta()) < m_mu_max_eta) &&
               (m_muonSelTool.empty() || m_muonSelTool->accept(*muon));
    }
    bool TestIsolationCloseByCorrAlg::passSelection(const EventContext& ctx, const xAOD::Egamma* egamm) const {
        if (egamm->type() == xAOD::Type::ObjectType::Electron) {
            return egamm->pt() >= m_el_min_pt && (m_el_max_eta < 0. || std::abs(egamm->eta()) < m_el_max_eta) &&
                   (m_elecSelTool.empty() || m_elecSelTool->accept(ctx, egamm));
        }
        /// Photon
        return egamm->pt() >= m_ph_min_pt && (m_ph_max_eta < 0. || std::abs(egamm->eta()) < m_ph_max_eta) &&
               (m_photSelTool.empty() || m_photSelTool->accept(ctx, egamm));
    }

    StatusCode TestIsolationCloseByCorrAlg::execute() {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        //
        xAOD::ElectronContainer* Electrons = nullptr;
        std::pair<std::unique_ptr<xAOD::ElectronContainer>, std::unique_ptr<xAOD::ShallowAuxContainer>> ElShallow;
        ATH_CHECK(loadContainer<xAOD::ElectronContainer>(ctx, m_elecKey, ElShallow));
        Electrons = ElShallow.first.get();
        //
        xAOD::PhotonContainer*  Photons = nullptr;
        std::pair<std::unique_ptr<xAOD::PhotonContainer>, std::unique_ptr<xAOD::ShallowAuxContainer>> PhShallow;
        ATH_CHECK(loadContainer<xAOD::PhotonContainer>(ctx, m_photKey, PhShallow));
        Photons = PhShallow.first.get();
        //
        xAOD::MuonContainer* Muons = nullptr;
        std::pair<std::unique_ptr<xAOD::MuonContainer>, std::unique_ptr<xAOD::ShallowAuxContainer>> MuonsShallow;
        ATH_CHECK(loadContainer<xAOD::MuonContainer>(ctx, m_muonKey, MuonsShallow));
        Muons = MuonsShallow.first.get();

        // Okay everything is defined for the preselection of the algorithm. lets  pass the things  towards the IsoCorrectionTool
        if (m_isoCloseByCorrTool->getCloseByIsoCorrection(ctx, Electrons, Muons, Photons).code() == CorrectionCode::Error) {
            ATH_MSG_ERROR("Something weird happened with the tool");
            return StatusCode::FAILURE;
        }
        // The isoCorrectionTool has now corrected everything using close-by objects satisfiyng the dec_PassQuality criteria
        // The name of the decorator is set via the 'SelectionDecorator' property of the tool
        // Optionally one can also define that the tool shall only objects surviving the overlap removal without  changing the initial
        // decorator Use therefore the 'PassOverlapDecorator' property to define the decorators name If you define  the 'BackupPrefix'
        // property then the original values are stored before correction <Prefix>_<IsolationCone> The final result  whether the object
        // passes the isolation criteria now can be stored in the 'IsolationSelectionDecorator' e.g. 'CorrectedIso'

        // parse the associated muon clusters to the tool
        ClusterSet muon_clusters;
        PflowSet pflows;


        /// If the track selection alg upstream is defined let's check whether the collection is complete
        TrackSet selected_trks{}, expected_trks{};
        if (!m_polTrkKey.empty()) {
            SG::ReadHandle<xAOD::TrackParticleContainer> closeTrkColl{m_polTrkKey, ctx};
            if (!closeTrkColl.isValid()) {
                ATH_MSG_FATAL("Failed to load " << m_polTrkKey.fullKey() << " from storegate");
                return StatusCode::FAILURE;
            }
            for (const xAOD::TrackParticle* trk : *closeTrkColl) { selected_trks.emplace(trk); }
            IsolationCloseByCorrectionTool::ObjectCache cache;
            correction_tool()->loadPrimaryParticles(Electrons, cache);
            correction_tool()->loadPrimaryParticles(Muons, cache);
            correction_tool()->loadPrimaryParticles(Photons, cache);
            correction_tool()->loadAssociatedObjects(ctx, cache);
            expected_trks = std::move(cache.tracks);
            muon_clusters = std::move(cache.clusters);
            pflows = std::move(cache.flows);
        }

        // Store everything in the final ntuples
        auto fill_helper = [&](std::shared_ptr<IsoCorrectionTestHelper> helper, const xAOD::IParticleContainer* parts) -> StatusCode{
            if (!helper) return StatusCode::SUCCESS;
            helper->SetClusters(muon_clusters);
            helper->SetFlowElements(pflows);
            return helper->Fill(parts);
        };
        ATH_CHECK(fill_helper(m_ele_helper, Electrons));
        ATH_CHECK(fill_helper(m_muo_helper, Muons));
        ATH_CHECK(fill_helper(m_pho_helper, Photons));
        ATH_CHECK(m_tree.fill(ctx));
        return StatusCode::SUCCESS;
    }
    const CP::IsolationCloseByCorrectionTool* TestIsolationCloseByCorrAlg::correction_tool() const {
        return dynamic_cast<const CP::IsolationCloseByCorrectionTool*>(m_isoCloseByCorrTool.get());
    }
}  // namespace CP
