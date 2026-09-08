/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCombinedMuonCandidateAlg.h"


namespace {
    // Temporary collection for extrapolated tracks and links with correspondent MS tracks
    struct track_link {
        std::unique_ptr<Trk::Track> track{};
        unsigned int container_index{0};
        bool extp_succeed{false};
        track_link(std::unique_ptr<Trk::Track> trk, unsigned int idx, bool succeed) :
            track{std::move(trk)}, container_index{idx}, extp_succeed{succeed} {}
    };
}  // namespace



StatusCode MuonCombinedMuonCandidateAlg::initialize() {
    ATH_CHECK(m_muonTrackParticleLocation.initialize());
    ATH_CHECK(m_candidateCollectionName.initialize());
    ATH_CHECK(m_msOnlyTracks.initialize());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_trackBuilder.retrieve(EnableTool{!m_trackBuilder.empty()}));
    ATH_CHECK(m_trackExtrapolationTool.retrieve(EnableTool{!m_trackExtrapolationTool.empty()}));
    ATH_CHECK(m_ambiguityProcessor.retrieve());
    ATH_CHECK(m_trackSummaryTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_beamSpotKey.initialize());

    ATH_CHECK(m_segmentKey.initialize(!m_segmentKey.empty()));
    ATH_CHECK(m_trackSegmentAssociationTool.retrieve(EnableTool{!m_segmentKey.empty()}));
  
    return StatusCode::SUCCESS;
}

StatusCode MuonCombinedMuonCandidateAlg::execute(const EventContext& ctx) const {
    // retrieve MuonSpectrometer tracks
    const xAOD::TrackParticleContainer* muonTrackParticles{};
    ATH_CHECK(SG::get(muonTrackParticles, m_muonTrackParticleLocation, ctx));

    SG::WriteHandle muonCandidates{m_candidateCollectionName, ctx};
    SG::WriteHandle msOnlyTracks{m_msOnlyTracks, ctx};

    ATH_CHECK(muonCandidates.record(std::make_unique<MuonCandidateCollection>()));
    ATH_CHECK(msOnlyTracks.record(std::make_unique<TrackCollection>()));
   
    const InDet::BeamSpotData* beamSpot{nullptr};
    ATH_CHECK(SG::get(beamSpot, m_beamSpotKey, ctx));
   
    ATH_MSG_DEBUG("Producing MuonCandidates for " << muonTrackParticles->size());
    unsigned int ntracks = 0;

    
    ATH_MSG_DEBUG("Beamspot position bs_x=" << Amg::toString(beamSpot->beamPos()));

    std::vector<track_link> trackLinks;

    unsigned int index = -1;
    // Loop over MS tracks
    for (const xAOD::TrackParticle* track : *muonTrackParticles) {
        ++index;

        if (!track->track()) {
            ATH_MSG_WARNING("MuonStandalone track particle without Trk::Track");
            continue;
        }
        const Trk::Track& msTrack = *track->track();

        ATH_MSG_VERBOSE("Re-Fitting track " << std::endl
                << m_printer->print(msTrack) << std::endl
                << m_printer->printStations(msTrack));

        std::unique_ptr<Trk::Track> standaloneTrack;
        if (m_extrapolationStrategy == 0u) {
            standaloneTrack = m_trackBuilder->standaloneFit(ctx, msTrack, beamSpot->beamPos(), nullptr);
        } else {
            standaloneTrack = m_trackExtrapolationTool->extrapolate(msTrack, ctx);
        }
        
        if (standaloneTrack) {
            // Reject the track if its fit quality is much (much much) worse than that of the non-extrapolated track
            if (standaloneTrack->fitQuality()->doubleNumberDoF() == 0) {
                standaloneTrack.reset();
                ATH_MSG_DEBUG("extrapolated track has no DOF, don't use it");
            } else {
                double mschi2 = 2.5;  // a default we should hopefully never have to use (taken from CombinedMuonTrackBuilder)
                if (msTrack.fitQuality()->doubleNumberDoF() > 0)
                    mschi2 = msTrack.fitQuality()->chiSquared() / msTrack.fitQuality()->doubleNumberDoF();
                // choice of 1000 is slightly arbitrary, the point is that the fit should be really be terrible
                if (standaloneTrack->fitQuality()->chiSquared() / standaloneTrack->fitQuality()->doubleNumberDoF() > 1000 * mschi2) {
                    standaloneTrack.reset();
                    ATH_MSG_DEBUG("extrapolated track has a degraded fit, don't use it");
                }
            }
        }
        if (standaloneTrack) {
            standaloneTrack->info().setParticleHypothesis(Trk::muon);
            standaloneTrack->info().setPatternRecognitionInfo(Trk::TrackInfo::MuidStandAlone);
            ATH_MSG_VERBOSE("Extrapolated track " << std::endl
                        << m_printer->print(*standaloneTrack) << std::endl
                        << m_printer->printStations(*standaloneTrack));
            ++ntracks;
            if (!standaloneTrack->perigeeParameters()) {
                ATH_MSG_WARNING(" Track without perigee " << (*standaloneTrack));
            } else if (!standaloneTrack->perigeeParameters()->covariance()) {
                ATH_MSG_WARNING(" Track with perigee without covariance " << (*standaloneTrack));
            }
            trackLinks.emplace_back(std::move(standaloneTrack), index, true);
        } else {
            // We can create tracks from EM segments+TGC hits
            // If these are not successfully extrapolated, they are too low quality to be useful
            // So only make candidates from un-extrapolated tracks if they are not EM-only
            bool skipTrack = true;
            const Trk::MuonTrackSummary* msMuonTrackSummary = nullptr;
            std::unique_ptr<Trk::TrackSummary> msTrackSummary;
            // If reading from an ESD, the track will not have a track summary yet
            if (!msTrack.trackSummary()) {
                msTrackSummary = m_trackSummaryTool->summary(ctx, msTrack);
                msMuonTrackSummary = msTrackSummary->muonTrackSummary();
            } else{
                msMuonTrackSummary = msTrack.trackSummary()->muonTrackSummary();
            }
            for (const auto& chs : msMuonTrackSummary->chamberHitSummary()) {
                using namespace Muon::MuonStationIndex;
                if ((chs.isMdt() && m_idHelperSvc->stationIndex(chs.chamberId()) != StIndex::EM) ||
                    m_idHelperSvc->isCsc(chs.chamberId())) {
                    skipTrack = false;
                    break;
                }
            }
            if (!skipTrack) { 
                trackLinks.emplace_back(std::make_unique<Trk::Track>(msTrack), index, false); 
            }
        }
    }
    ///
    auto extrapTracks = std::make_unique<TrackCollection>(SG::VIEW_ELEMENTS);
    extrapTracks->reserve(trackLinks.size());
    for (const track_link& link : trackLinks) {
        extrapTracks->push_back(link.track.get());
    }    
    ATH_MSG_DEBUG("Finished back-tracking, total number of successfull fits " << ntracks);

    // Resolve ambiguity between extrapolated tracks (where available)
    auto resolvedTracks(m_ambiguityProcessor->process(ctx, extrapTracks.get()));

    ATH_MSG_DEBUG("Finished ambiguity solving: " << extrapTracks->size() << " track(s) in -> " << resolvedTracks->size()
                                                     << " track(s) out");

    const Trk::SegmentCollection* segments{nullptr};
    ATH_CHECK(SG::get(segments, m_segmentKey, ctx));

    // Loop over resolved tracks and build MuonCondidate collection
    for (const Trk::Track* track : *resolvedTracks) {
        std::vector<track_link>::iterator tLink = std::ranges::find_if(trackLinks, 
                                                    [&track](const track_link& link) { 
                                                        return link.track.get() == track; 
                                                    });

        if (tLink == trackLinks.end()) {
            ATH_MSG_WARNING("Unable to find internal link between MS and SA tracks!");
            continue;
        }

        std::unique_ptr<MuonCombined::MuonCandidate> muon_candidate{};
        ElementLink<xAOD::TrackParticleContainer> MS_TrkLink{*muonTrackParticles, tLink->container_index, ctx};
        if (tLink->extp_succeed) {
            msOnlyTracks->push_back(std::move(tLink->track));
            ElementLink<TrackCollection> saLink(*msOnlyTracks, msOnlyTracks->size() - 1, ctx);
            muon_candidate = std::make_unique<MuonCombined::MuonCandidate>(MS_TrkLink, saLink, 
                                                                           msOnlyTracks->size() - 1);
                // remove track from set so it is not deleted
        } else {
            // in this case the extrapolation failed
            muon_candidate = std::make_unique<MuonCombined::MuonCandidate>(MS_TrkLink);
        }
        /// Last but not least set the segments
        if (segments) {
            std::vector<const Muon::MuonSegment*> assoc_segs;
            m_trackSegmentAssociationTool->associatedSegments(*muon_candidate->primaryTrack(), segments, assoc_segs);
            muon_candidate->setSegments(std::move(assoc_segs));
        }
        muonCandidates->push_back(std::move(muon_candidate));
    }
    return StatusCode::SUCCESS;
}
