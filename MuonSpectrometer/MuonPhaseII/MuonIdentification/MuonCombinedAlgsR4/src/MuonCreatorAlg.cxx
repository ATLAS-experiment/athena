/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCreatorAlg.h"


#include "GaudiKernel/SystemOfUnits.h"

namespace {
    constexpr double inGeV = 1./ Gaudi::Units::GeV;
    constexpr double inDeg = 1./ Gaudi::Units::deg;
}

namespace MuonCombinedR4 {

StatusCode MuonCreatorAlg::initialize() {
    ATH_CHECK(m_muonKey.initialize());
    ATH_CHECK(m_tagKeys.initialize());
    ATH_CHECK(m_selectionTool.retrieve());
    ATH_CHECK(m_summaryTool.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode MuonCreatorAlg::execute(const EventContext& ctx) const {
    DataShip ship{};
    ATH_CHECK(setupDataShip(ctx, ship));

    /** Create first the combined muon tags */
    for (const auto& [idTrk, muTags] : ship.combinedTags) {
        ATH_MSG_DEBUG("Store new combined muon having associated ID track with"
            <<idTrk->pt()*inGeV<<", eta: "<<idTrk->eta()<<", phi:" <<idTrk->phi()*inDeg);
        createMuon(ctx, muTags, ship);
    }
    /** Create then the other muon tags */
    for (const MuonR4::MuonTag* saTag: ship.standaloneTags) {
        ATH_MSG_DEBUG("Store new standalone muon with pt: "
                     <<saTag->msTrack()->pt()*inGeV
                     <<", eta: "<<saTag->msTrack()->eta()
                     <<", phi: "<<saTag->msTrack()->phi()*inDeg);
        createMuon(ctx, std::array{saTag}, ship);
    }
    return StatusCode::SUCCESS;
}


StatusCode MuonCreatorAlg::setupDataShip(const EventContext& ctx, DataShip& ship) const {
    ATH_CHECK(ship.muons.record(m_muonKey, ctx));

    std::unordered_set<const xAOD::TrackParticle*> cmbMsTrks{};
    std::vector<const MuonR4::MuonTagContainer*> tagContainers{};
    tagContainers.reserve(m_tagKeys.size());
    for (const SG::ReadHandleKey<MuonR4::MuonTagContainer>& key : m_tagKeys) {
        const MuonR4::MuonTagContainer* tagCont{nullptr};
        ATH_CHECK(SG::get(tagCont, key, ctx));
        if (tagCont->empty()) {
            continue;
        }
        tagContainers.push_back(tagCont);
    }

    /** @todo Sort the tag containers by authors to ensure that 
     *        the standalone tag is at last
    */
    /** Loop over the defined muon tag containers and sort them 
     *  according to their ID track 
     *  (Covers segmentTag, inside-out combined, combined) */
    for (const MuonR4::MuonTagContainer* tagCont: tagContainers) {
        for (const MuonR4::MuonTag* muTag: *tagCont) {
            if (muTag->idTrack() != nullptr) {
                ship.combinedTags[muTag->idTrack()].emplace_back(muTag);
                cmbMsTrks.insert(muTag->msTrack());
                continue;
            }
            // Check whether the tag has an MS tag
            if (!muTag->msTrack()) {
                continue;
            } 
            // If there is no combined tag with MS track the 
            // same MS track, then turn it into a standalone muon
            if (cmbMsTrks.insert(muTag->msTrack()).second) {
                ship.standaloneTags.emplace_back(muTag);
            } else {
                /** try to asociate the tag with the combined tag*/
                for (auto& [trk, tags]: ship.combinedTags) {
                    if (std::ranges::any_of(tags, [&muTag](const MuonR4::MuonTag* known){
                        return known->msTrack() == muTag->msTrack();
                    })) {
                        tags.push_back(muTag);
                        break;
                    }
                }
            }
        }        
    }
    return StatusCode::SUCCESS;
}

ElementLink<xAOD::TrackParticleContainer> 
    MuonCreatorAlg::linkParticle(const EventContext& ctx,
                                 const xAOD::TrackParticle* trk) const {
    if (!trk) {
        return TrackLink_t{};
    }
    return TrackLink_t{static_cast<const xAOD::TrackParticleContainer&>(*trk->container()),
                       trk->index(), ctx};
}


void MuonCreatorAlg::createMuon(const EventContext& ctx,
                                const std::span<const MuonR4::MuonTag* const>& muTags,
                                DataShip& ship) const {
    // Create a new muon
    xAOD::Muon* newMuon = ship.muons->push_back(std::make_unique<xAOD::Muon>());
    
    bool p4Set{false}, summarySet{false};
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    std::vector<SegLink_t> segLinks{};

    std::unordered_set<const xAOD::MuonSegment*> addedSegs{};
    for (const MuonR4::MuonTag* tag : muTags) {
        if (!p4Set) {
            const xAOD::TrackParticle* pTrk = tag->primaryTrack();
            newMuon->setP4(pTrk->pt(), pTrk->eta(), pTrk->phi());
            newMuon->setCharge(pTrk->charge());
            newMuon->setAuthor(tag->author());
        }
        if (!summarySet && tag->summary()) {
            m_summaryTool->copySummary(*tag->summary(), *newMuon);
            summarySet = true;
        }
        using Trk_t = xAOD::Muon::TrackParticleType;
        /** Link the available particles */
        if (!p4Set || !newMuon->trackParticle(Trk_t::InnerDetectorTrackParticle)){
            newMuon->setTrackParticleLink(Trk_t::InnerDetectorTrackParticle,
                                          linkParticle(ctx, tag->idTrack()));
        }
        if (!p4Set || !newMuon->trackParticle(Trk_t::MuonSpectrometerTrackParticle)){
            newMuon->setTrackParticleLink(Trk_t::MuonSpectrometerTrackParticle,
                                          linkParticle(ctx, tag->msTrack()));
        }
        if (!p4Set || !newMuon->trackParticle(Trk_t::CombinedTrackParticle)){
            newMuon->setTrackParticleLink(Trk_t::CombinedTrackParticle,
                                          linkParticle(ctx, tag->cbTrack()));
        }
        for (const xAOD::MuonSegment* seg : tag->segments()) {
            if (addedSegs.insert(seg).second) {
                segLinks.emplace_back(static_cast<const xAOD::MuonSegmentContainer&>(*seg->container()),
                                      seg->index(), ctx);
            }
        }
        newMuon->addAllAuthor(tag->author());
        tag->copyParameters(*newMuon);
        p4Set = true;
    }
    newMuon->setMuonSegmentLinks(segLinks);

    switch(newMuon->author()){
        using enum xAOD::Muon::Author;
        case MuidCo:
        case MuGirl:
        case STACO:
            newMuon->setMuonType(xAOD::Muon::MuonType::Combined);
            break;
        case MuidSA:
            newMuon->setMuonType(xAOD::Muon::MuonType::MuonStandAlone);
            break;
        case MuTagIMO:
            newMuon->setMuonType(xAOD::Muon::MuonType::SegmentTagged);
            break;
        default:
            ATH_MSG_WARNING("Invalid muon author "<<newMuon->author()<<". Cannot determine the muon type");
            ship.muons->pop_back();
    }

    return;
    m_selectionTool->setPassesIDCuts(*newMuon);
    m_selectionTool->setQuality(*newMuon);
}

}