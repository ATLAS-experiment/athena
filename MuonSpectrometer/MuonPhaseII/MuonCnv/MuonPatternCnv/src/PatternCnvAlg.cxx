
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "PatternCnvAlg.h"

#include "MuonPatternEvent/MuonHoughDefs.h"

namespace{
    using DetIdx_t = Muon::MuonStationIndex::DetectorRegionIndex;
    using LayIdx_t = Muon::MuonStationIndex::LayerIndex;
    using LegacyMax = MuonHough::MuonLayerHough::Maximum;
}

namespace MuonR4{
    
    StatusCode PatternCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_keyTgc.initialize(!m_keyTgc.empty()));
        ATH_CHECK(m_keyRpc.initialize(!m_keyRpc.empty()));
        ATH_CHECK(m_keyMdt.initialize(!m_keyMdt.empty()));
        ATH_CHECK(m_keysTgc.initialize(!m_keysTgc.empty()));
        ATH_CHECK(m_keyMM.initialize(!m_keyMM.empty()));

        ATH_CHECK(m_combiKey.initialize());
        ATH_CHECK(m_dataPerSecKey.initialize());
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode PatternCnvAlg::execute(const EventContext& ctx) const {


        auto translatedPatterns = std::make_unique<MuonPatternCombinationCollection>();
        auto translatedHough = std::make_unique<Muon::HoughDataPerSectorVec>();

        for (const SG::ReadHandleKey<SegmentSeedContainer>& key : m_readKeys) {
            const SegmentSeedContainer* translateMe{nullptr};
            ATH_CHECK(SG::get(translateMe, key, ctx));
            ATH_CHECK(convertSeed(ctx, *translateMe, * translatedPatterns, *translatedHough));

        }
        
        SG::WriteHandle<MuonPatternCombinationCollection> patternHandle{m_combiKey, ctx};
        ATH_CHECK(patternHandle.record(std::move(translatedPatterns)));

        SG::WriteHandle<Muon::HoughDataPerSectorVec> houghDataHandle{m_dataPerSecKey, ctx};
        ATH_CHECK(houghDataHandle.record(std::move(translatedHough)));


        return StatusCode::SUCCESS;
    }
    StatusCode PatternCnvAlg::convertSeed(const EventContext& ctx,
                                          const SegmentSeedContainer& seedContainer,
                                          ::MuonPatternCombinationCollection& patternContainer,
                                          Muon::HoughDataPerSectorVec& houghDataSec) const {

        const Muon::RpcPrepDataContainer* rpcPrds{nullptr};
        const Muon::MdtPrepDataContainer* mdtPrds{nullptr};
        const Muon::TgcPrepDataContainer* tgcPrds{nullptr};
        const Muon::sTgcPrepDataContainer* stgcPrds{nullptr};
        const Muon::MMPrepDataContainer* mmPrds{nullptr};
        ATH_CHECK(SG::get(mdtPrds, m_keyMdt, ctx));
        ATH_CHECK(SG::get(rpcPrds, m_keyRpc, ctx));
        ATH_CHECK(SG::get(tgcPrds, m_keyTgc, ctx));
        ATH_CHECK(SG::get(stgcPrds, m_keysTgc, ctx));
        ATH_CHECK(SG::get(mmPrds, m_keyMM, ctx));

        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

       
    
        for (const SegmentSeed* seed: seedContainer) {
            const Amg::Transform3D& localToGlobal = seed->msSector()->localToGlobalTrans(*gctx);
                
            std::unordered_set<Identifier> channelsInMax{};
            for (const HoughHitType& hit : seed->getHitsInMax()) {
                channelsInMax.insert(hit->identify());
                if (hit->secondaryMeasurement()) {
                    channelsInMax.insert(xAOD::identify(hit->secondaryMeasurement()));
                }
            }
            std::vector<const Trk::PrepRawData*> trkHits{};
            trkHits.reserve(channelsInMax.size());
            using techIdx_t = Muon::MuonStationIndex::TechnologyIndex;
            for (const Identifier& chId : channelsInMax){
                switch (m_idHelperSvc->technologyIndex(chId)){
                    case techIdx_t::MDT:
                        trkHits.push_back(fetchPrd(chId, mdtPrds));
                        break;
                    case techIdx_t::RPC:
                        trkHits.push_back(fetchPrd(chId, rpcPrds));
                        break;
                    case techIdx_t::TGC:
                        trkHits.push_back(fetchPrd(chId, tgcPrds));
                        break;
                    case techIdx_t::MM:
                        trkHits.push_back(fetchPrd(chId, mmPrds));
                        break;
                    case techIdx_t::STGC:
                        trkHits.push_back(fetchPrd(chId, stgcPrds));
                        break;
                    default:
                        ATH_MSG_WARNING("Cscs are not part of the new paradigms. Where are they now coming from? "
                                        <<m_idHelperSvc->toString(chId));
                };
            }
            if (std::find(trkHits.begin(), trkHits.end(), nullptr) != trkHits.end()){
                ATH_MSG_ERROR("Errors during the Prd conversion occured");
                return StatusCode::FAILURE;
            }
            const Amg::Vector3D maxPos{seed->localPosition()};
            const Amg::Vector3D locDir{seed->localDirection()};

            Trk::TrackSurfaceIntersection isect{localToGlobal * maxPos, localToGlobal.linear()*locDir,0.};
            ATH_MSG_VERBOSE("Intersection at "<<m_idHelperSvc->toStringChamber(trkHits[0]->identify())<<" "<<Amg::toString(isect.position())<<" "<<Amg::toString(isect.direction())
                            <<Amg::angle(isect.position(), isect.direction()) / Gaudi::Units::deg );
            Muon::MuonPatternChamberIntersect chamberIsect{std::move(isect), std::move(trkHits)};
            
            convertMaximum(chamberIsect, houghDataSec);
            std::vector<Muon::MuonPatternChamberIntersect> chamberData{std::move(chamberIsect)};

            auto patternCombi = std::make_unique<Muon::MuonPatternCombination>(nullptr, std::move(chamberData));
            patternContainer.push_back(std::move(patternCombi));
        }
        
        return StatusCode::SUCCESS;
    }
    void PatternCnvAlg::convertMaximum(const Muon::MuonPatternChamberIntersect& intersect,
                                           Muon::HoughDataPerSectorVec& houghDataSec) const {
        
        if (houghDataSec.vec.empty()) {
            constexpr unsigned int nSectors = 16;
            houghDataSec.vec.resize(nSectors);
            for (unsigned int i = 0; i < nSectors; ++i) {
                houghDataSec.vec[i].sector = i + 1;
            }
        }

        const Identifier chId = intersect.prepRawDataVec()[0]->identify();
        const DetIdx_t regionIndex = m_idHelperSvc->regionIndex(chId);
        const LayIdx_t layerIndex = m_idHelperSvc->layerIndex(chId);
        const unsigned int sectorLayerHash = Muon::MuonStationIndex::sectorLayerHash(regionIndex, layerIndex);
        const int sector = m_idHelperSvc->sector(chId);
        const bool barrel = !m_idHelperSvc->isEndcap(chId);

        auto convertedMax = std::make_unique<LegacyMax>();
        convertedMax->hough = &houghDataSec.detectorHoughTransforms.hough(sector, regionIndex, layerIndex);
        convertedMax->theta =  intersect.intersectDirection().theta();
        for (const Trk::PrepRawData* prd : intersect.prepRawDataVec()) {
            std::unique_ptr<MuonHough::Hit> hit{};
            Amg::Vector3D gp{Amg::Vector3D::Zero()};
            if (m_idHelperSvc->isMdt(prd->identify())){
                gp = static_cast<const Muon::MdtPrepData*>(prd)->globalPosition();
            } else {
                gp = static_cast<const Muon::MuonCluster*>(prd)->globalPosition();
            }
            
            // TODO: Might need some fine-tuning if the hough hit information is needed downstream
            hit = std::make_unique<MuonHough::Hit>(0, barrel ? gp.perp() : gp.z(),  
                                                      barrel ? gp.z() : gp.perp(),  
                                                      barrel ? gp.z() : gp.perp(), 
                                                      1., nullptr, prd);
            convertedMax->hits.emplace_back(std::move(hit));
        }
        houghDataSec.vec[sector-1].maxVec[sectorLayerHash].push_back(std::move(convertedMax));
    }


    template <class PrdType> 
        const PrdType* PatternCnvAlg::fetchPrd(const Identifier& prdId,
                                                   const Muon::MuonPrepDataContainerT<PrdType>* prdContainer) const {
        if (!prdContainer) {
            ATH_MSG_ERROR("Cannot fetch a prep data object as the container given for "<<
                          m_idHelperSvc->toString(prdId)<<" is a nullptr");
            return nullptr;
        }
        const Muon::MuonPrepDataCollection<PrdType>* coll = prdContainer->indexFindPtr(m_idHelperSvc->moduleHash(prdId));
        if (!coll) {
            ATH_MSG_ERROR("No prep data collection where "<<m_idHelperSvc->toString(prdId)<<" can reside in.");
            return nullptr;
        }
        for (const PrdType* prd : *coll) {
            if (prd->identify() == prdId){
                return prd;
            }
        }
        ATH_MSG_ERROR("There is no measurement "<<m_idHelperSvc->toString(prdId));
        return nullptr;    
    }
}