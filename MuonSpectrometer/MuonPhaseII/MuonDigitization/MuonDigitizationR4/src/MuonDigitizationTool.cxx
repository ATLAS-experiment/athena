/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonDigitizationR4/MuonDigitizationTool.h"

#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <AthenaKernel/RNGWrapper.h>
#include <GeoModelKernel/throwExcept.h>

namespace MuonR4{

    MuonDigitizationTool::MuonDigitizationTool(const std::string& type, 
                                               const std::string& name, 
                                               const IInterface* pIID):
            PileUpToolBase{type,name, pIID} {}

    StatusCode MuonDigitizationTool::initialize(){
        ATH_MSG_INFO("SimHitKey "<<m_simHitKey.key()<<", "<<m_streamName<<", SDO container: "<<m_sdoKey.key());
        if (m_simHitKey.empty() && m_inputObjectName.empty()) {
            ATH_MSG_FATAL("Property <SimHitKey> not set !");
            return StatusCode::FAILURE;
        }
        if (m_streamName.empty()) {
            ATH_MSG_FATAL("Property "<<m_streamName<<" not set !");
            return StatusCode::FAILURE;
        }
        if (m_onlyUseContainerName) {
            m_inputObjectName = m_simHitKey.key();
            ATH_CHECK(m_mergeSvc.retrieve());
        }
        /// Initialize ReadHandleKey
        ATH_CHECK(m_simHitKey.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_sdoKey.initialize(!m_sdoKey.empty()));
        ATH_CHECK(m_rndmSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode MuonDigitizationTool::prepareEvent(const EventContext& /*ctx*/, 
                                                  unsigned int nInputEvents) {

        ATH_MSG_DEBUG("prepareEvent() called for " << nInputEvents << " input events");
        m_timedHits.clear();
        m_simHits.clear();
        return StatusCode::SUCCESS;
    }
 
    StatusCode MuonDigitizationTool::fillTimedHits(PileUpHits&& hitColl, TimedHits& timedHits) const {
        for (const auto& [timeIndex, simHitColl] : hitColl) {
            timedHits.reserve(timedHits.capacity() + simHitColl->size());
            for (const xAOD::MuonSimHit* simHit : *simHitColl) {
                timedHits.emplace_back(timeIndex.time(), timeIndex.index(), simHit, timeIndex.type());
            }           
        }
        std::stable_sort(timedHits.begin(), timedHits.end(), 
                 [](const TimedHit& a, const TimedHit& b){
                    if (a->identify() != b->identify()){
                        return a->identify() < b->identify();
                    }
                    if (a.eventId() != b.eventId()) {
                       return a.eventId() < b.eventId();
                    } 
                    return a.eventTime() < b.eventTime(); 
                }); 
        return StatusCode::SUCCESS;
    }
    double MuonDigitizationTool::hitTime(const TimedHit& hit) {
        return hit.eventTime() + hit->globalTime();
    }
    StatusCode MuonDigitizationTool::processAllSubEvents(const EventContext& ctx) {
        const MuonDigitizationTool* digiTool = this;
        return digiTool->processAllSubEvents(ctx);
    }

    StatusCode MuonDigitizationTool::processAllSubEvents(const EventContext& ctx) const {
        PileUpHits hitCollList{};
        TimedHits timedHits{};
        /// In case of single hits container just load the collection using read handles    
        if (!m_onlyUseContainerName) {            
            const xAOD::MuonSimHitContainer* hitCollection{nullptr};
            ATH_CHECK(retrieveContainer(ctx, m_simHitKey, hitCollection));
            hitCollList.emplace_back(PileUpTimeEventIndex(0), hitCollection);
         } else {
            ATH_CHECK(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList));
        }
        ATH_CHECK(fillTimedHits(std::move(hitCollList), timedHits));
        SG::WriteHandle<xAOD::MuonSimHitContainer> sdoContainer{};
        if (!m_sdoKey.empty()) {
             sdoContainer = SG::WriteHandle<xAOD::MuonSimHitContainer>{m_sdoKey, ctx};
             ATH_CHECK(sdoContainer.record(std::make_unique<xAOD::MuonSimHitContainer>(),
                                           std::make_unique<xAOD::MuonSimHitAuxContainer>()));
        }
        ATH_CHECK(digitize(ctx, timedHits, !m_sdoKey.empty() ? sdoContainer.ptr() : nullptr));
        return StatusCode::SUCCESS;
    }
    StatusCode MuonDigitizationTool::mergeEvent(const EventContext& ctx) {
        ATH_MSG_DEBUG("mergeEvent()");
        
        SG::WriteHandle<xAOD::MuonSimHitContainer> sdoContainer{};
        if (!m_sdoKey.empty()) {
             sdoContainer = SG::WriteHandle<xAOD::MuonSimHitContainer>{m_sdoKey, ctx};
             ATH_CHECK(sdoContainer.record(std::make_unique<xAOD::MuonSimHitContainer>(),
                                           std::make_unique<xAOD::MuonSimHitAuxContainer>()));
        }
        ATH_CHECK(digitize(ctx, m_timedHits, !m_sdoKey.empty() ? sdoContainer.ptr() : nullptr));
        m_timedHits.clear();
        m_simHits.clear();
        return StatusCode::SUCCESS;
    }


    StatusCode MuonDigitizationTool::processBunchXing(int bunchXing, 
                                                      SubEventIterator bSubEvents, 
                                                      SubEventIterator eSubEvents) {        
        ATH_MSG_DEBUG("processBunchXing()" << bunchXing);
        PileUpHits hitList{}, hitListPermanent{};
        ATH_CHECK(m_mergeSvc->retrieveSubSetEvtData(m_inputObjectName, hitList, bunchXing, bSubEvents, eSubEvents));
        ATH_MSG_VERBOSE(hitList.size() << " hits in  xAODMuonSimHitContainer " << m_inputObjectName << " found");
        for (auto& [hitPtr, hitContainer] : hitList) {
            auto copyContainer = std::make_unique<xAOD::MuonSimHitContainer>();
            auto copyAuxContainer = std::make_unique<xAOD::MuonSimHitAuxContainer>();
            copyContainer->setStore(copyAuxContainer.get());
            for (const xAOD::MuonSimHit* copyMe : *hitContainer) {
               (*copyContainer->push_back(std::make_unique<xAOD::MuonSimHit>())) = (*copyMe);
            }
            hitListPermanent.emplace_back(hitPtr, copyContainer.get());
            m_simHits.emplace_back(std::move(copyContainer), std::move(copyAuxContainer));
        } 
        ATH_CHECK(fillTimedHits(std::move(hitListPermanent), m_timedHits));
        return StatusCode::SUCCESS;
    }

    CLHEP::HepRandomEngine* MuonDigitizationTool::getRandomEngine(const EventContext&ctx) const {
        ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
        std::string rngName = m_streamName;
        rngWrapper->setSeed(rngName, ctx);
        return rngWrapper->getEngine(ctx);
    }
    const ActsGeometryContext& MuonDigitizationTool::getGeoCtx(const EventContext& ctx) const {
        const ActsGeometryContext* gctx{};
        if (!retrieveContainer(ctx, m_geoCtxKey, gctx).isSuccess()) {
            THROW_EXCEPTION("Failed to retrieve the geometry context "<<m_geoCtxKey.fullKey());
        }
        return *gctx;
    }
    xAOD::MuonSimHit* MuonDigitizationTool::addSDO(const TimedHit& hit, 
                                                   xAOD::MuonSimHitContainer* sdoContainer) const {
        if(!sdoContainer) {
            ATH_MSG_VERBOSE("No SDO container setup of writing");
            return nullptr;
        }
        if (!m_includePileUpTruth && HepMC::ignoreTruthLink(hit->genParticleLink(), m_vetoPileUpTruthLinks)) { 
            ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hit->identify())<<" is a pile-up truth link");
            return nullptr; 
        }
        
        xAOD::MuonSimHit* sdoHit = sdoContainer->push_back(std::make_unique<xAOD::MuonSimHit>());
        (*sdoHit) = (*hit);
        static const SG::Accessor<float> acc_eventTime{"MuSim_evtTime"};
        static const SG::Accessor<unsigned short> acc_eventID{"MuSim_evtID"};
        static const SG::Accessor<unsigned short> acc_puType{"MuSim_puType"};
        acc_eventTime(*sdoHit) = hit.eventTime();
        acc_eventID(*sdoHit) = hit.eventId();
        acc_puType(*sdoHit) = hit.pileupType();
        return sdoHit;
    }
    bool MuonDigitizationTool::passDeadTime(const Identifier& channelId,
                                            const double hitTime, 
                                            const double deadTimeWindow,
                                            DeadTimeMap& deadTimeMap) {
        auto insertItr = deadTimeMap.insert(std::make_pair(channelId,hitTime));
        /// Channel not seen before
        if (insertItr.second) {
            return true;
        }
        if (hitTime - insertItr.first->second < deadTimeWindow) {
            return false;
        }
        /// Update dead time map & accept hit
        insertItr.first->second = hitTime;
        return true;
    }
}