/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCreatorAlg.h"

#include "MuonTrackEvent/AuthorHierachy.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Utilities/Enumerate.hpp"
#include "AthContainers/Accessor.h"
#include "StoreGate/ReadCondHandle.h"

namespace {
    constexpr double inGeV = 1./ Gaudi::Units::GeV;
    constexpr double inDeg = 1./ Gaudi::Units::deg;

    /** @brief Translate the Acts calo extension into the entry / exit positions per calo layer.
     *         The state on the sensitive surface of a sampling is used as proxy sample point, so
     *         entry and exit coincide. The CaloCellCollector only uses their midpoint. */
    CaloExtensionHelpers::EntryExitLayerMap entryExitLayerMap(const Acts::GeometryContext& gctx,
                                                              const ActsTrk::CaloExtension& extension) {
        CaloExtensionHelpers::EntryExitLayerMap result{};
        for (std::size_t id = 0; id < static_cast<std::size_t>(CaloSampling::Unknown); ++id) {
            const auto sample = static_cast<CaloSampling::CaloSample>(id);
            std::optional<Amg::Vector3D> entry{std::nullopt}, exit{std::nullopt};
            for (const Acts::BoundTrackParameters& pars : extension.caloParameters(sample)) {
                // Reject portals: the sensitive surface is the sample point and portals carry layer 0 (PreSamplerB)
                if (pars.referenceSurface().geometryId().boundary() != 0) {
                    continue;
                }
                exit = pars.position(gctx);
                if (!entry) {
                    entry = exit;
                }
            }
            if (entry) {
                result.emplace(sample, std::make_pair(*entry, *exit));
            }
        }
        return result;
    }
}

namespace MuonCombinedR4 {

StatusCode MuonCreatorAlg::initialize() {
    ATH_CHECK(m_muonKey.initialize());
    ATH_CHECK(m_tagKeys.initialize());
    ATH_CHECK(m_selectionTool.retrieve());
    ATH_CHECK(m_summaryTool.retrieve());
    m_useCells = m_useCaloCells && !m_clusterContainerName.key().empty();
    ATH_CHECK(m_clusterContainerName.initialize(m_useCells));
    ATH_CHECK(m_clusterContainerLinkName.initialize(m_useCells));
    ATH_CHECK(m_cellContainerName.initialize(m_useCells));
    ATH_CHECK(m_caloMgrKey.initialize(m_useCells));
    ATH_CHECK(m_caloNoiseKey.initialize(m_useCells && !m_caloNoiseKey.empty()));
    ATH_CHECK(m_ctxProvider.initialize(m_useCells));
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
    /** Attach the cell link container to the created clusters and record it */
    if (m_useCells) {
        SG::WriteHandle<CaloClusterCellLinkContainer> linkHandle{m_clusterContainerLinkName, ctx};
        auto* clusterStore = ship.caloClusters.getHandle().storeHandle().get();
        for (xAOD::CaloCluster* cluster : *ship.caloClusters) {
            cluster->setLink(ship.caloClusterLinks.get(), clusterStore);
        }
        ATH_CHECK(linkHandle.record(std::move(ship.caloClusterLinks)));
    }
    return StatusCode::SUCCESS;
}


StatusCode MuonCreatorAlg::setupDataShip(const EventContext& ctx, DataShip& ship) const {
    ATH_CHECK(ship.muons.record(m_muonKey, ctx));
    if (m_useCells) {
        ATH_CHECK(ship.caloClusters.record(m_clusterContainerName, ctx));
        ATH_CHECK(SG::get(ship.caloCells, m_cellContainerName, ctx));
        ATH_CHECK(SG::get(ship.caloMgr, m_caloMgrKey, ctx));
        if (!m_caloNoiseKey.empty()) {
            ATH_CHECK(SG::get(ship.caloNoise, m_caloNoiseKey, ctx));
        }
    }

    std::unordered_set<const xAOD::TrackParticle*> cmbMsTrks{};
    std::vector<const MuonR4::MuonTagContainer*> tagContainers{};
    tagContainers.resize(m_tagKeys.size(), nullptr);
    for (const auto [idx, key]  : Acts::enumerate(m_tagKeys)) {
        ATH_CHECK(SG::get(tagContainers[idx], key, ctx));
    }
    std::ranges::sort(tagContainers, MuonR4::AuthorHierachy{});
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


void MuonCreatorAlg::collectCells(const EventContext& ctx,
                                  xAOD::Muon& muon,
                                  DataShip& ship) const {
    static const SG::Accessor<float> acc_ET_Core{"ET_Core"};
    static const SG::Accessor<float> acc_ET_EMCore{"ET_EMCore"};
    static const SG::Accessor<float> acc_ET_TileCore{"ET_TileCore"};
    static const SG::Accessor<float> acc_ET_HECCore{"ET_HECCore"};
    acc_ET_Core(muon) = 0.f;
    acc_ET_EMCore(muon) = 0.f;
    acc_ET_TileCore(muon) = 0.f;
    acc_ET_HECCore(muon) = 0.f;

    const xAOD::TrackParticle* idTrack = muon.trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
    if (!idTrack) {
        return;
    }
    const ActsTrk::CaloExtension* extension = ActsTrk::getCaloExtension(*idTrack);
    if (!extension || extension->empty()) {
        ATH_MSG_DEBUG("No calo extension available for the ID track of the muon");
        return;
    }
    const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);
    xAOD::CaloCluster* cluster = m_cellCollector.collectCells(entryExitLayerMap(gctx, *extension),
                                                              ship.caloMgr, *ship.caloCells, *ship.caloClusters);
    if (!cluster) {
        ATH_MSG_DEBUG("Failed to create the cluster from the calo extension");
        return;
    }
    ATH_MSG_DEBUG("New cluster: eta "<<cluster->eta()<<" phi "<<cluster->phi()<<" cells "<<cluster->size());
    muon.setClusterLink(ElementLink<xAOD::CaloClusterContainer>(*ship.caloClusters, cluster->index(), ctx));

    std::vector<float> etcore(Rec::CaloCellCollector::N_ET_Core, 0.f);
    m_cellCollector.collectEtCore(*cluster, etcore, ship.caloNoise, m_sigmaCaloNoiseCut);
    acc_ET_Core(muon) = etcore[Rec::CaloCellCollector::ET_Core];
    acc_ET_EMCore(muon) = etcore[Rec::CaloCellCollector::ET_EMCore];
    acc_ET_TileCore(muon) = etcore[Rec::CaloCellCollector::ET_TileCore];
    acc_ET_HECCore(muon) = etcore[Rec::CaloCellCollector::ET_HECCore];
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
         if (!p4Set || !newMuon->trackParticle(Trk_t::ExtrapolatedMuonSpectrometerTrackParticle)){
            newMuon->setTrackParticleLink(Trk_t::ExtrapolatedMuonSpectrometerTrackParticle,
                                          linkParticle(ctx, tag->meTrack()));
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
        /// We know that every STACO or MuidCo muon needs to have a MS track
        /// Manually add MuidSA for these cases
        if (tag->author() == xAOD::Muon::Author::STACO ||
            tag->author() == xAOD::Muon::Author::MuidCo) {
            newMuon->addAllAuthor(xAOD::Muon::Author::MuidSA);
        }
        newMuon->addAllAuthor(tag->author());
        tag->copyParameters(*newMuon);
        p4Set = true;
    }
    newMuon->setMuonSegmentLinks(segLinks);

    if (ship.caloCells) {
        collectCells(ctx, *newMuon, ship);
    }

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
        case CaloTag:
        case CaloScore:
             newMuon->setMuonType(xAOD::Muon::MuonType::CaloTagged);
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
