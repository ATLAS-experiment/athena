
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "IsoCloseByCorrectionTrkSelAlg.h"

#include <IsolationSelection/IsolationCloseByCorrectionTool.h>



#include "FourMomUtils/xAODP4Helpers.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ThinningHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include <algorithm>
namespace CP {

    IsoCloseByCorrectionTrkSelAlg::IsoCloseByCorrectionTrkSelAlg(const std::string& name, ISvcLocator* svcLoc) :
        AthReentrantAlgorithm(name, svcLoc) {}
    StatusCode IsoCloseByCorrectionTrkSelAlg::initialize() {
        ATH_CHECK(m_muonKey.initialize(!m_muonKey.empty()));
        ATH_CHECK(m_elecKey.initialize(!m_elecKey.empty()));
        ATH_CHECK(m_photKey.initialize(!m_photKey.empty()));
        if (!m_selDecoration.empty()) {
            m_elecSelKey = m_elecKey.empty() ? "" : m_elecKey.key() + "." + m_selDecoration.value();
            m_mounSelKey = m_muonKey.empty() ? "" : m_muonKey.key() + "." + m_selDecoration.value();
            m_photSelKey = m_photKey.empty() ? "" : m_photKey.key() + "." + m_selDecoration.value();
        }
        /// Input object selection
        ATH_CHECK(m_elecSelKey.initialize(!m_elecSelKey.empty()));
        ATH_CHECK(m_mounSelKey.initialize(!m_mounSelKey.empty()));
        ATH_CHECK(m_photSelKey.initialize(!m_photSelKey.empty()));
        if (!m_muonKey.empty() && !m_muonSelTool.empty())
            ATH_CHECK(m_muonSelTool.retrieve());
        else
            m_muonSelTool.disable();
        if (!m_elecKey.empty() && !m_elecSelTool.empty())
            ATH_CHECK(m_elecSelTool.retrieve());
        else
            m_elecSelTool.disable();
        if (!m_photSelTool.empty() && !m_photSelTool.empty())
            ATH_CHECK(m_photSelTool.retrieve());
        else
            m_photSelTool.disable();
        ATH_CHECK(m_closeByCorrTool.retrieve());
        /// Output keys
        ATH_CHECK(m_trkKey.initialize(!m_trkKey.empty()));
        ATH_CHECK(m_thinKey.initialize(m_stream, !m_stream.empty()));
        return StatusCode::SUCCESS;
    }
    StatusCode IsoCloseByCorrectionTrkSelAlg::finalize() {
        const Long64_t tot_acc = m_accepted_trks[0] + m_accepted_trks[1] + m_accepted_trks[2];
        ATH_MSG_INFO("Accepted " << tot_acc << " tracks associated with collimated lepton pairs."
                                 << " Track association split into particle type");
        ATH_MSG_INFO("      --- Muons: " << m_accepted_trks[0] << " out of " << m_selected_obj[0] << " ("
                                         << (m_selected_obj[0] ? 100. * m_accepted_trks[0] / m_selected_obj[0] : 0.) << "%).");
        ATH_MSG_INFO("      --- Electrons: " << m_accepted_trks[1] << " out of " << m_selected_obj[1] << " ("
                                             << (m_selected_obj[1] ? 100. * m_accepted_trks[1] / m_selected_obj[1] : 0.) << "%).");
        ATH_MSG_INFO("      --- Photons: " << m_accepted_trks[2] << " out of " << m_selected_obj[2] << " ("
                                           << (m_selected_obj[2] ? 100. * m_accepted_trks[2] / m_selected_obj[2] : 0.) << "%).");
        ATH_MSG_INFO("Rate w.r.t. all inner detector tracks (" << m_tot_trks << "): " << (m_tot_trks ? 100. * tot_acc / m_tot_trks : 0.)
                                                               << "%");
        return StatusCode::SUCCESS;
    }
    StatusCode IsoCloseByCorrectionTrkSelAlg::execute(const EventContext& ctx) const {

        TrackSet assoc_trks{};
        LepContainer prim_objs{};
        ATH_CHECK(loadTracks(ctx, m_muonKey, assoc_trks, prim_objs));
        const size_t n_muons = prim_objs.size();
        ATH_CHECK(loadTracks(ctx, m_elecKey, assoc_trks, prim_objs));
        const size_t n_elecs = prim_objs.size() - n_muons;
        ATH_CHECK(loadTracks(ctx, m_photKey, assoc_trks, prim_objs));
        const size_t n_phots = prim_objs.size() - n_muons - n_elecs;
        m_selected_obj[0] += n_muons;
        m_selected_obj[1] += n_elecs;
        m_selected_obj[2] += n_phots;

        /// Next kick all track that cannot polute the isolation variable cones
        {
            TrackSet pruned_trks{};
            std::copy_if(assoc_trks.begin(), assoc_trks.end(), std::inserter(pruned_trks, pruned_trks.begin()),
                         [this, &prim_objs, &ctx](const TrackPtr& track) -> bool {
                             int trk_idx{-1};
                             bool ret_code{false};
                             for (const xAOD::IParticle* prim : prim_objs) {
                                 TrackSet assoc_trk = m_closeByCorrTool->getTrackCandidates(ctx, prim);
                                 /// The track particle can never polute the isolation cone of its associated lepton / photon
                                 if (assoc_trk.count(track)) {
                                     /// Update the counter
                                     if (prim->type() == xAOD::Type::ObjectType::Muon)
                                         trk_idx = 0;
                                     else if (prim->type() == xAOD::Type::ObjectType::Electron)
                                         trk_idx = 1;
                                     else if (prim->type() == xAOD::Type::ObjectType::Photon)
                                         trk_idx = 2;
                                     if (ret_code) break;
                                     continue;
                                 }
                                 /// It's outside the cone
                                 if (xAOD::P4Helpers::deltaR(track, m_closeByCorrTool->isoRefParticle(prim)) > m_maxConeSize)
                                     continue;
                                 ret_code = true;
                                 if (trk_idx != -1) break;
                             }
                             if (ret_code) ++m_accepted_trks.at(trk_idx);
                             return ret_code;
                         });
            assoc_trks = std::move(pruned_trks);
        }

        /// Dump the containers
        if (!m_trkKey.empty()) {
            auto tracks = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(SG::VIEW_ELEMENTS);
            for (const TrackPtr& trk : assoc_trks) { 
                const xAOD::TrackParticle* trk_p = trk;
                tracks->push_back(trk_p);
            }
            auto writeHandle = SG::makeHandle(m_trkKey, ctx);
            ATH_CHECK(writeHandle.record(std::move(tracks)));
        }
        if (!m_thinKey.empty()) {
            SG::ThinningHandle<xAOD::TrackParticleContainer> thinner{m_thinKey, ctx};
            m_tot_trks += thinner->size();
            std::vector<bool> thin_dec(thinner->size(), false);
            for (const TrackPtr& trk : assoc_trks) { thin_dec[trk->index()] = true; }
            thinner.keep(thin_dec);
        }
        return StatusCode::SUCCESS;
    }

    template <class CONT_TYPE>
    StatusCode IsoCloseByCorrectionTrkSelAlg::loadTracks(const EventContext& ctx, const SG::ReadHandleKey<CONT_TYPE>& key, TrackSet& tracks,
                                                         LepContainer& prim_objs) const {
        if (key.empty()) {
            ATH_MSG_DEBUG("No key has been defined. Assume that the container is disabled ");
            return StatusCode::SUCCESS;
        }
        SG::ReadHandle<CONT_TYPE> particles{key, ctx};
        if (!particles.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << key.fullKey());
            return StatusCode::FAILURE;
        }
        for (const auto* p : *particles) {
            if (!passSelection(ctx, p)) continue;
            prim_objs.insert(p);
            TrackSet part_trks = m_closeByCorrTool->getTrackCandidates(ctx, p);
            tracks.insert(part_trks.begin(), part_trks.end());
        }
        return StatusCode::SUCCESS;
    }

    bool IsoCloseByCorrectionTrkSelAlg::passSelection(const EventContext& ctx, const xAOD::Electron* elec) const {
        if (!m_elecSelKey.empty()) {
            SG::ReadDecorHandle<xAOD::ElectronContainer, char> decor{m_elecSelKey, ctx};
            if (!decor(*elec)) return false;
        }
        return elec->pt() >= m_minElecPt && (m_elecSelTool.empty() || m_elecSelTool->accept(ctx, elec));
    }
    bool IsoCloseByCorrectionTrkSelAlg::passSelection(const EventContext& ctx, const xAOD::Photon* phot) const {
        if (!m_photSelKey.empty()) {
            SG::ReadDecorHandle<xAOD::PhotonContainer, char> decor{m_photSelKey, ctx};
            if (!decor(*phot)) return false;
        }
        return phot->pt() >= m_minPhotPt && (m_photSelTool.empty() || m_photSelTool->accept(phot));
    }
    bool IsoCloseByCorrectionTrkSelAlg::passSelection(const EventContext& ctx, const xAOD::Muon* muon) const {
        if (!m_photSelKey.empty()) {
            SG::ReadDecorHandle<xAOD::MuonContainer, char> decor{m_mounSelKey, ctx};
            if (!decor(*muon)) return false;
        }
        return muon->pt() >= m_minMuonPt && (m_muonSelTool.empty() || m_muonSelTool->accept(*muon));
    }
}  // namespace CP
