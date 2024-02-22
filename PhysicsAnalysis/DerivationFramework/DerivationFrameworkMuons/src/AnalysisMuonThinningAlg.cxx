/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <DerivationFrameworkMuons/AnalysisMuonThinningAlg.h>
#include <StoreGate/ThinningHandle.h>
namespace {
    using TrkThinKey = SG::ThinningHandleKey<xAOD::TrackParticleContainer>;
    using TrkThinning = SG::ThinningHandle<xAOD::TrackParticleContainer>;
    using TrkThinOptional = std::optional<TrkThinning>;
    using SegmentThinning = SG::ThinningHandle<xAOD::MuonSegmentContainer>;
    using KeepMap = std::map<const xAOD::TrackParticleContainer*, std::vector<bool>>;
    using KeepPair = std::pair<const xAOD::TrackParticleContainer*, std::vector<bool>>;

    using MuonPassDecor = SG::ReadDecorHandle<xAOD::MuonContainer, bool>;
    using TrackPassDecor = SG::ReadDecorHandle<xAOD::TrackParticleContainer, bool>;

    constexpr double MeVtoGeV = 1./ Gaudi::Units::GeV;
}  // namespace
namespace DerivationFramework {

    AnalysisMuonThinningAlg::AnalysisMuonThinningAlg(const std::string& n, ISvcLocator* p):
    AthReentrantAlgorithm(n, p) {}

    StatusCode AnalysisMuonThinningAlg::initialize() {
        const std::string& stream = m_streamName.value();
        if (stream.empty()) {
            ATH_MSG_FATAL("Please give a valid stream for thinning");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_muonSelTool.retrieve());
        ATH_CHECK(m_muonKey.initialize(stream));
        ATH_CHECK(m_IdTrkKey.initialize(stream, !m_IdTrkKey.empty()));
        ATH_CHECK(m_FwdIdTrkKey.initialize(stream, !m_FwdIdTrkKey.empty()));
        ATH_CHECK(m_MSTrkKey.initialize(stream, !m_MSTrkKey.empty()));
        ATH_CHECK(m_METrkKey.initialize(stream, !m_METrkKey.empty()));
        ATH_CHECK(m_MSOETrkKey.initialize(stream, !m_MSOETrkKey.empty()));
        ATH_CHECK(m_CmbTrkKey.initialize(stream, !m_CmbTrkKey.empty()));
        ATH_CHECK(m_SegmentKey.initialize(stream, !m_SegmentKey.empty()));

        ATH_CHECK(m_MuonPassKeys.initialize());
        ATH_CHECK(m_TrkPassKeys.initialize());
        if (!m_MuonPassKeys.empty()) {
            std::stringstream sstr{};
            for (const auto& key : m_MuonPassKeys) {
                sstr<<" *** "<<key.fullKey()<<std::endl;
            }
            ATH_MSG_INFO("Accept muons with upstream flag "<<std::endl<<sstr.str());
        }
        if (!m_TrkPassKeys.empty()) {
            std::stringstream sstr{};
            for (const auto& key : m_TrkPassKeys) {
                sstr<<" *** "<<key.fullKey()<<std::endl;
            }
            ATH_MSG_INFO("Accept tracks with upstream flag "<<std::endl<<sstr.str());
        }

        return StatusCode::SUCCESS;
    }

    StatusCode AnalysisMuonThinningAlg::execute(const EventContext& ctx) const {
        /// The Muon container thinning
        SG::ThinningHandle<xAOD::MuonContainer> MuonContainer{m_muonKey, ctx};
        if (!MuonContainer.isValid()) {
            ATH_MSG_FATAL("Failed to establish thinning handle for " << m_muonKey.fullKey());
            return StatusCode::FAILURE;
        }
        std::vector<bool> keep_muo(MuonContainer->size(), false);

        /// Initialize the decorators whitelisting the muon to be written to the output
        std::vector<MuonPassDecor> mu_passFlags;
        for (const SG::ReadDecorHandleKey<xAOD::MuonContainer>& pass_key : m_MuonPassKeys) { mu_passFlags.emplace_back(pass_key, ctx); }

        /// Cache the thinning decisions for each associated track particle
        KeepMap thin_decisions;
        std::vector<std::unique_ptr<TrkThinning>> TrkContainers;

        /// Lambda function to fill the thinning helpers above
        auto make_handle = [&, this](const TrkThinKey& key) {
            if (key.empty()) {
                ATH_MSG_DEBUG("No key was given for thinning");
                return StatusCode::SUCCESS;
            }
            ATH_MSG_DEBUG("Create new thinning handle " << key.fullKey());
            TrkContainers.emplace_back(std::make_unique<TrkThinning>(key, ctx));

            std::unique_ptr<TrkThinning>& last = TrkContainers.back();
            if (!last->isValid()) {
                ATH_MSG_FATAL("Failed to create a track thinning handle " << key.fullKey());
                return StatusCode::FAILURE;
            }
            const xAOD::TrackParticleContainer* cont = (*last).cptr();
            thin_decisions[cont].resize(cont->size(), false);
            return StatusCode::SUCCESS;
        };

        ATH_CHECK(make_handle(m_IdTrkKey));
        ATH_CHECK(make_handle(m_FwdIdTrkKey));
        ATH_CHECK(make_handle(m_MSTrkKey));
        ATH_CHECK(make_handle(m_METrkKey));
        ATH_CHECK(make_handle(m_MSOETrkKey));
        ATH_CHECK(make_handle(m_CmbTrkKey));

        /// Do not forget to apply the thinning to the SegmentContainers as well
        std::unique_ptr<SegmentThinning> segmentContainer =
            !m_SegmentKey.empty() ? std::make_unique<SegmentThinning>(m_SegmentKey, ctx) : nullptr;
        std::vector<bool> keep_seg(segmentContainer ? segmentContainer->cptr()->size() : 0, false);

        /// Loop over the muon container
        for (const xAOD::Muon* muon : *MuonContainer) {
            /// The muon is rejected by the selection tool
            /// & not marked as pass by the augmentation tools upstream
            ATH_MSG_DEBUG("Check muon with pt " << muon->pt() * MeVtoGeV << " [GeV], eta: " << muon->eta() << ", phi: " << muon->phi() << " q: " << muon->charge()
                                          << " quality: " << muon->quality() << ", author: " << muon->author());
            
            if (!m_muonSelTool->accept(*muon) &&
                std::find_if(mu_passFlags.begin(), mu_passFlags.end(), 
                            [this, muon](const MuonPassDecor& decor) ->bool { 
                                ATH_MSG_VERBOSE("Check decorator "<<decor.decorKey()<<" "
                                            <<SG::AuxTypeRegistry::instance().getName(decor.auxid())
                                            <<" "<< decor(*muon));
                                return decor(*muon); 
                            }) == mu_passFlags.end())
                continue;
            keep_muo[muon->index()] = true;
            ATH_MSG_DEBUG("Muon will be dumped");

            /// Ensure that all associated track particles are written to the output file
            for (auto tp :
                 {xAOD::Muon::InnerDetectorTrackParticle, xAOD::Muon::MuonSpectrometerTrackParticle, xAOD::Muon::CombinedTrackParticle,
                  xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle, xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle}) {
                const xAOD::TrackParticle* track = muon->trackParticle(tp);
                if (!track) {
                    ATH_MSG_DEBUG("No track particle given");
                    continue;
                }
                KeepMap::iterator itr = std::find_if(thin_decisions.begin(), thin_decisions.end(),
                                                     [track](const KeepPair& pair) { return pair.first == track->container(); });
                if (itr == thin_decisions.end()) {
                    ATH_MSG_WARNING("Could not find for track pT " << track->pt() << " eta: " << track->eta() << ", phi: " << track->phi()
                                                                  << " a valid associated container");
                    continue;
                }
                itr->second[track->index()] = true;
            }
            /// Proceed with the Segment containers
            if (segmentContainer) {
                for (size_t s = 0; s < muon->nMuonSegments(); ++s) {
                    const xAOD::MuonSegment* seg = muon->muonSegment(s);
                    if (seg) keep_seg[seg->index()] = true;
                }
            }
        }
        /// The next step is to find all track particles that are accepted by the upstream
        /// augmentation tools as well
        for (const SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>& key : m_TrkPassKeys) {
            TrackPassDecor handle{key, ctx};
            const xAOD::TrackParticleContainer* trks = handle.cptr();

            std::vector<bool>& trk_thin = thin_decisions[trks];
            /// The vector has been freshly created -> the container is unknown to the
            /// current tool
            if (!trks->empty() && trk_thin.empty()) {
                ATH_MSG_WARNING("The container " << key.fullKey() << " is not part of the current thinning scheme. Please check");
                continue;
            }
            /// Put the tracks on the white list
            for (const xAOD::TrackParticle* trk : *trks) {
                if (handle(*trk)) trk_thin[trk->index()] = true;
            }
        }

        /// Propagate the decisions
        MuonContainer.keep(keep_muo);
        for (std::unique_ptr<TrkThinning>& thin : TrkContainers) {
            std::vector<bool>& dec = thin_decisions[thin->cptr()];
            thin->keep(dec);
        }
        if (segmentContainer) { segmentContainer->keep(keep_seg); }

        return StatusCode::SUCCESS;
    }

}  // namespace DerivationFramework
