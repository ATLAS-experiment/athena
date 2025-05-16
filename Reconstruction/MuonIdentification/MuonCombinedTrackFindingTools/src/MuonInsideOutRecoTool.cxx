/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonInsideOutRecoTool.h"

#include "MuonCombinedEvent/MuGirlTag.h"
#include "MuonLayerEvent/MuonCandidate.h"
#include "MuonLayerEvent/MuonLayerPrepRawData.h"
#include "MuonLayerEvent/MuonSystemExtension.h"
#include "MuonSegment/MuonSegment.h"
#include "xAODTracking/Vertex.h"

using namespace Muon::MuonStationIndex;
namespace MuonCombined {

    MuonInsideOutRecoTool::MuonInsideOutRecoTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonCombinedInDetExtensionTool>(this);
        declareInterface<MuonInsideOutRecoTool>(this);
    }

    StatusCode MuonInsideOutRecoTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_segmentFinder.retrieve());
        ATH_CHECK(m_segmentMatchingTool.retrieve());
        ATH_CHECK(m_ambiguityResolver.retrieve());
        ATH_CHECK(m_candidateTrackBuilder.retrieve());
        ATH_CHECK(m_trackFitter.retrieve());
        ATH_CHECK(m_trackAmbiguityResolver.retrieve());
        // trigger does not use primary vertex
        ATH_CHECK(m_vertexKey.initialize(!m_vertexKey.empty()));
        ATH_CHECK(m_trackSummaryTool.retrieve());
        ATH_CHECK(m_recoValidationTool.retrieve(DisableTool{m_recoValidationTool.empty()}));
        return StatusCode::SUCCESS;
    }

    void MuonInsideOutRecoTool::extend(const InDetCandidateCollection& inDetCandidates, InDetCandidateToTagMap* tagMap,
                                       TrackCollection* combTracks, TrackCollection* meTracks, Trk::SegmentCollection* segments,
                                       const EventContext& ctx) const {
        ATH_MSG_WARNING("This version is deprecated, please use extendWithPRDs for MuGirl");
        IMuonCombinedInDetExtensionTool::MuonPrdData prdData;
        extendWithPRDs(inDetCandidates, tagMap, prdData, combTracks, meTracks, segments, ctx);
    }

    void MuonInsideOutRecoTool::extendWithPRDs(const InDetCandidateCollection& inDetCandidates, InDetCandidateToTagMap* tagMap,
                                               IMuonCombinedInDetExtensionTool::MuonPrdData prdData, TrackCollection* combTracks,
                                               TrackCollection* meTracks, Trk::SegmentCollection* segments, const EventContext& ctx) const {
        ATH_MSG_DEBUG(" extending " << inDetCandidates.size());
        for (const InDetCandidate* it : inDetCandidates) { handleCandidate(*it, tagMap, prdData, combTracks, meTracks, segments, ctx); }
    }

    void MuonInsideOutRecoTool::handleCandidate(const InDetCandidate& indetCandidate, InDetCandidateToTagMap* tagMap,
                                                const IMuonCombinedInDetExtensionTool::MuonPrdData& prdData, TrackCollection* combTracks,
                                                TrackCollection* meTracks, Trk::SegmentCollection* segColl, const EventContext& ctx) const {
        if (m_ignoreSiAssocated && indetCandidate.isSiliconAssociated()) {
            ATH_MSG_DEBUG(" skip silicon associated track for extension ");
            return;
        }

        const xAOD::TrackParticle& indetTrackParticle = indetCandidate.indetTrackParticle();
        if (!indetTrackParticle.track() || indetTrackParticle.pt() < m_idTrackMinPt) return;

        // get intersections which precision layers in the muon system
        const Muon::MuonSystemExtension* muonSystemExtension = indetCandidate.getExtension();
        if (!muonSystemExtension) {
            ATH_MSG_VERBOSE("No MuonSystemExtension, aborting ");
            return;
        }
        // fill validation content
        if (!m_recoValidationTool.empty()) m_recoValidationTool->addTrackParticle(indetTrackParticle, *muonSystemExtension);

        // loop over intersections, get segments
        std::vector<Muon::MuonLayerRecoData> allLayers;

        const std::vector<Muon::MuonSystemExtension::Intersection>& layerIntersections = muonSystemExtension->layerIntersections();
        ATH_MSG_DEBUG(" ID track: pt " << indetTrackParticle.pt() << " eta " << indetTrackParticle.eta() << " phi "
                                       << indetTrackParticle.phi() << " layers " << layerIntersections.size());

        for (const Muon::MuonSystemExtension::Intersection& layer_intersect : layerIntersections) {
            // vector to store segments
            std::vector<std::shared_ptr<const Muon::MuonSegment>> segments;

            // find segments for intersection
            Muon::MuonLayerPrepRawData layerPrepRawData;
            if (!getLayerData(layer_intersect.layerSurface, layerPrepRawData, prdData)) {
                ATH_MSG_VERBOSE("Failed to get layer data");
                continue;
            }
            m_segmentFinder->find(ctx, layer_intersect, layerPrepRawData, segments);
            if (segments.empty()) continue;

            // fill validation content
            if (!m_recoValidationTool.empty()) {
                for (const std::shared_ptr<const Muon::MuonSegment>& seg : segments) m_recoValidationTool->add(layer_intersect, *seg, 0);
            }

            // match segments to intersection
            std::vector<std::shared_ptr<const Muon::MuonSegment>> selectedSegments;
            m_segmentMatchingTool->select(ctx, layer_intersect, segments, selectedSegments);
            if (selectedSegments.empty()) continue;

            // fill validation content
            if (!m_recoValidationTool.empty()) {
                for (const auto& seg : selectedSegments) m_recoValidationTool->add(layer_intersect, *seg, 1);
            }

            // add to total list
            allLayers.emplace_back(layer_intersect, std::move(selectedSegments));
        }

        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << " Layers with segments " << allLayers.size();
            for (auto& layer : allLayers) {
                for (auto& seg : layer.segments) { msg(MSG::DEBUG) << std::endl << m_printer->print(*seg); }
            }
            msg(MSG::DEBUG) << endmsg;
        }

        // find best candidate and exit if none found
        std::pair<std::unique_ptr<const Muon::MuonCandidate>, std::unique_ptr<Trk::Track>> bestCandidate =
            findBestCandidate(ctx, indetTrackParticle, allLayers);
        if (!bestCandidate.first) { return; }

        // add candidate to indet candidate
        addTag(ctx, indetCandidate, tagMap, *bestCandidate.first, bestCandidate.second, combTracks, meTracks, segColl);
    }

    std::pair<std::unique_ptr<const Muon::MuonCandidate>, std::unique_ptr<Trk::Track>> MuonInsideOutRecoTool::findBestCandidate(
        const EventContext& ctx, const xAOD::TrackParticle& indetTrackParticle,
        const std::vector<Muon::MuonLayerRecoData>& allLayers) const {
        // resolve ambiguities
        std::vector<Muon::MuonCandidate> resolvedCandidates;
        m_ambiguityResolver->resolveOverlaps(ctx, allLayers, resolvedCandidates);

        // fit candidates
        TrackCollection tracks(SG::VIEW_ELEMENTS);
        typedef std::pair<std::unique_ptr<Trk::Track>, std::unique_ptr<const Muon::MuonCandidate>> candidatePair;
        std::vector<candidatePair> trackCandidateLookup;
        for (Muon::MuonCandidate& candidate : resolvedCandidates) {
            std::unique_ptr<Trk::Track> track = m_candidateTrackBuilder->buildCombinedTrack(ctx, *indetTrackParticle.track(), candidate);
            /// Check if the fit succeeded and whether there are TSOS & a fitQuality
            if (!track || !track->isValid() || !track->fitQuality()->numberDoF()) continue;

            tracks.push_back(track.get());
            trackCandidateLookup.emplace_back(std::move(track), std::make_unique<Muon::MuonCandidate>(std::move(candidate)));
        }

        ATH_MSG_DEBUG("found " << tracks.size() << " combined tracks");

        // first handle easy cases of zero or one track
        if (tracks.empty()) return {nullptr, nullptr};

        Trk::Track* selectedTrack = nullptr;
        if (tracks.size() == 1) {
            selectedTrack = tracks.front();
        } else {
            // more than 1 track call ambiguity solver and select first track
            std::unique_ptr<const TrackCollection> resolvedTracks(m_trackAmbiguityResolver->process(&tracks));
            if (!resolvedTracks || resolvedTracks->empty()) {
                ATH_MSG_WARNING("Ambiguity resolver returned no tracks. Arbitrarily using the first track of initial collection.");
                selectedTrack = tracks.front();
            } else {
                TrackCollection::iterator it = std::find(tracks.begin(), tracks.end(), resolvedTracks->front());
                if (it != tracks.end()) {
                    selectedTrack = *it;
                } else {
                    ATH_MSG_ERROR("Ambiguity resolver returned an unknown track. Arbitrarily using the first track of initial collection.");
                    selectedTrack = tracks.front();
                }
            }
        }
        // get candidate
        std::vector<candidatePair>::iterator look_itr =
            std::find_if(trackCandidateLookup.begin(), trackCandidateLookup.end(),
                         [selectedTrack](candidatePair& ele) { return ele.first.get() == selectedTrack; });

        if (look_itr == trackCandidateLookup.end() || !look_itr->second) {
            ATH_MSG_WARNING("candidate lookup failed, this should not happen");
            return {nullptr, nullptr};
        }
        // generate a track summary for this candidate
        if (m_trackSummaryTool.isEnabled()) { m_trackSummaryTool->computeAndReplaceTrackSummary(*selectedTrack, false); }

        return std::make_pair(std::move(look_itr->second), std::move(look_itr->first));
    }

    void MuonInsideOutRecoTool::addTag(const EventContext& ctx, const InDetCandidate& indetCandidate, InDetCandidateToTagMap* tagMap,
                                       const Muon::MuonCandidate& candidate, std::unique_ptr<Trk::Track>& selectedTrack,
                                       TrackCollection* combTracks, TrackCollection* meTracks, Trk::SegmentCollection* segments) const {
        const xAOD::TrackParticle& idTrackParticle = indetCandidate.indetTrackParticle();
        Amg::Vector3D origin{0., 0., 0.};

        const xAOD::Vertex* matchedVertex = nullptr;
        if (!m_vertexKey.empty()) {
            SG::ReadHandle<xAOD::VertexContainer> vertices{m_vertexKey, ctx};
            if (!vertices.isValid()) {
                ATH_MSG_WARNING("No vertex container with key = " << m_vertexKey.key() << " found");
            } else {
                for (const auto* const vx : *vertices) {
                    for (const auto& tpLink : vx->trackParticleLinks()) {
                        if (*tpLink == &idTrackParticle) {
                            matchedVertex = vx;
                            break;
                        }
                        if (matchedVertex) break;
                    }
                }
            }
        }
        if (matchedVertex) {
            origin = Amg::Vector3D{matchedVertex->x(), matchedVertex->y(), matchedVertex->z()};
            ATH_MSG_DEBUG(" found matched vertex " << origin);
        } else {
            origin = Amg::Vector3D{-idTrackParticle.d0() * std::sin(idTrackParticle.phi()) + idTrackParticle.vx(),
                                   idTrackParticle.d0() * std::cos(idTrackParticle.phi()) + idTrackParticle.vy(),
                                   idTrackParticle.z0() + idTrackParticle.vz()};
            ATH_MSG_DEBUG(" NO matched vertex  take track perigee " << origin);
        }

        ATH_MSG_VERBOSE("selectedTrack:");
        if (msgLevel(MSG::VERBOSE)) {
            int tsos = 0;
            for (const Trk::TrackStateOnSurface* it : *selectedTrack->trackStateOnSurfaces()) {
                ++tsos;
                if (it->trackParameters()) {
                    ATH_MSG_VERBOSE("check tsos " << tsos << " r " << it->trackParameters()->position().perp() << " z "
                                                  << it->trackParameters()->position().z() << " p "
                                                  << it->trackParameters()->momentum().mag());
                }
            }
        }

        std::vector<const Muon::MuonSegment*> segLinks;
        for (const Muon::MuonLayerIntersection& layer : candidate.layerIntersections) {
            std::unique_ptr<Muon::MuonSegment> copy = std::make_unique<Muon::MuonSegment>(*layer.segment);
            segLinks.push_back(copy.get());
            segments->push_back(std::move(copy));
        }
        /// Sort the segments here; note the lifetime of 'copy' is still valid here as it exists inside 'segments'
        //cppcheck-suppress invalidLifetime
        std::sort(segLinks.begin(), segLinks.end(), [this](const Muon::MuonSegment* seg_a, const Muon::MuonSegment* seg_b) -> bool {
            ChIndex ch_a = m_idHelperSvc->chamberIndex(m_edmHelperSvc->chamberId(*seg_a));
            ChIndex ch_b = m_idHelperSvc->chamberIndex(m_edmHelperSvc->chamberId(*seg_b));
            StIndex st_a = toStationIndex(ch_a);
            StIndex st_b = toStationIndex(ch_b);
            if (st_a != st_b) return st_a < st_b;
            /// Sort the CSC segments at the first giving priority to the small sectors
            if (ch_a == ChIndex::CSL || ch_a == ChIndex::CSS || ch_b == ChIndex::CSS || ch_b == ChIndex::CSL)
                return (ch_a == ChIndex::CSL) + 2 * (ch_a == ChIndex::CSS) > (ch_b == ChIndex::CSL) + 2 * (ch_b == ChIndex::CSS);
            return ch_a < ch_b;
        });

        if (msgLevel(MSG::DEBUG)) {
            std::stringstream sstr;
            for (const Muon::MuonSegment* muo_seg : segLinks) {
                auto chIdx = m_idHelperSvc->chamberIndex(m_edmHelperSvc->chamberId(*muo_seg));
                auto thIdx = m_idHelperSvc->technologyIndex(m_edmHelperSvc->chamberId(*muo_seg));
                sstr << chName(chIdx) << "  (" << technologyName(thIdx) << "), ";
            }
            ATH_MSG_DEBUG("Selected segments " << segLinks.size() << " " << sstr.str());
        }

        // perform standalone refit
        std::unique_ptr<Trk::Track> standaloneRefit{m_trackFitter->standaloneRefit(ctx, *selectedTrack, origin)};

        combTracks->push_back(std::move(selectedTrack));
        ElementLink<TrackCollection> comblink(*combTracks, combTracks->size() - 1);

        // create tag and set SA refit
        MuGirlTag* tag = new MuGirlTag(comblink, segLinks);
        if (standaloneRefit) {
            meTracks->push_back(std::move(standaloneRefit));
            ElementLink<TrackCollection> melink(*meTracks, meTracks->size() - 1);
            tag->setUpdatedExtrapolatedTrack(melink);
        }

        // add tag to IndetCandidateToTagMap
        tagMap->addEntry(&indetCandidate, tag);
    }

    bool MuonInsideOutRecoTool::getLayerData(const Muon::MuonLayerSurface& surf, Muon::MuonLayerPrepRawData& layerPrepRawData,
                                             IMuonCombinedInDetExtensionTool::MuonPrdData prdData) const {
        // get technologies in the given layer
        StIndex stIndex = toStationIndex(surf.regionIndex, surf.layerIndex);
        const std::set<TechnologyIndex>& technologiesInStation = m_idHelperSvc->technologiesInStation(stIndex);
        if (msgLevel(MSG::DEBUG)) {
            std::string techString;
            for (const TechnologyIndex& tech : technologiesInStation)
                techString += " " + technologyName(tech);
            ATH_MSG_DEBUG("getLayerData: sector " << surf.sector << " " << regionName(surf.regionIndex) << " "
                                                  << layerName(surf.layerIndex) << " technologies " << techString);
        }

        bool isok{false};
        // loop over technologies and get data
        for (const TechnologyIndex& it : technologiesInStation) {
            // get collections, keep track of failures
            switch(it) {
                using enum TechnologyIndex;
                case TechnologyIndexMax:
                case TechnologyUnknown:
                    break;
                case MDT:
                    isok |= getLayerDataTech<Muon::MdtPrepData>(it, surf, prdData.mdtPrds, layerPrepRawData.mdts);
                    break;
                case RPC:
                    isok |= getLayerDataTech<Muon::RpcPrepData>(it, surf, prdData.rpcPrds, layerPrepRawData.rpcs);
                    break;
                case TGC:
                    isok |= getLayerDataTech<Muon::TgcPrepData>(it, surf, prdData.tgcPrds, layerPrepRawData.tgcs);
                    break;
                case CSC:
                    isok |= getLayerDataTech<Muon::CscPrepData>(it, surf, prdData.cscPrds, layerPrepRawData.cscs);
                    break;
                case STGC:
                    isok |= getLayerDataTech<Muon::sTgcPrepData>(it, surf, prdData.stgcPrds, layerPrepRawData.stgcs);
                    break;
                case MM:
                    isok |= getLayerDataTech<Muon::MMPrepData>(it, surf, prdData.mmPrds, layerPrepRawData.mms);
                    break;
            }
        }

        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << " Got data: sector " << surf.sector << " " << regionName(surf.regionIndex) << " "
                            << layerName(surf.layerIndex);
            if (!layerPrepRawData.mdts.empty()) msg(MSG::DEBUG) << " MDTs " << layerPrepRawData.mdts.size();
            if (!layerPrepRawData.rpcs.empty()) msg(MSG::DEBUG) << " RPCs " << layerPrepRawData.rpcs.size();
            if (!layerPrepRawData.tgcs.empty()) msg(MSG::DEBUG) << " TGCs " << layerPrepRawData.tgcs.size();
            if (!layerPrepRawData.cscs.empty()) msg(MSG::DEBUG) << " CSCs " << layerPrepRawData.cscs.size();
            if (!layerPrepRawData.stgcs.empty()) msg(MSG::DEBUG) << " STGCs " << layerPrepRawData.stgcs.size();
            if (!layerPrepRawData.mms.empty()) msg(MSG::DEBUG) << " MMs " << layerPrepRawData.mms.size();
            msg(MSG::DEBUG) << endmsg;
        }
        return isok;
    }

    template <class COL>
    bool MuonInsideOutRecoTool::getLayerDataTech(TechnologyIndex technology, const Muon::MuonLayerSurface& surf,
                                                 const Muon::MuonPrepDataContainerT<COL>* input, 
                                                 std::vector<const Muon::MuonPrepDataCollection<COL>*>& output) const {
        if (!input || input->size() == 0) return false;
        // get technologies in the given layer
        unsigned int layHash = sectorLayerHash(surf.regionIndex, surf.layerIndex);

        // get hashes
        const Muon::MuonLayerHashProviderTool::HashVec hashes = m_layerHashProvider->getHashes(surf.sector, technology, layHash);

        // skip empty inputs
        if (hashes.empty()) return false;

        // loop over hashes
        for (Muon::MuonLayerHashProviderTool::HashVec::const_iterator it = hashes.begin(); it != hashes.end(); ++it) {
            // skip if not found
            const auto* colIt = input->indexFindPtr(*it);
            if (!colIt) { continue; }
            ATH_MSG_VERBOSE("  adding " << m_idHelperSvc->toStringChamber(colIt->identify()) << " size " << colIt->size());
            // else add
            output.push_back(colIt);
        }
        return output.size();
    }
}  // namespace MuonCombined
