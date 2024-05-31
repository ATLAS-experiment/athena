/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonDigitizationR4/MuonDigitizationTool.h"

#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <AthenaKernel/RNGWrapper.h>
#include <GeoModelHelpers/throwExcept.h>

namespace MuonR4{

    MuonDigitizationTool::MuonDigitizationTool(const std::string& type, 
                                               const std::string& name, 
                                               const IInterface* pIID):
            PileUpToolBase{type,name, pIID} {}

    StatusCode MuonDigitizationTool::initialize(){
        if (m_simHitKey.empty()) {
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
        ATH_CHECK(m_simHitKey.initialize(!m_onlyUseContainerName));
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
        return StatusCode::SUCCESS;
    }
 
    StatusCode MuonDigitizationTool::fillTimedHits(const PileUpHits& hitColl) {
        for (const auto& [timeIndex, simHitColl] : hitColl) {
            m_timedHits.reserve(m_timedHits.capacity() + simHitColl->size());
            for (const xAOD::MuonSimHit* simHit : *simHitColl) {
                m_timedHits.emplace_back(timeIndex.time(), timeIndex.index(), simHit, timeIndex.type());
            }           
        }
        std::sort(m_timedHits.begin(), m_timedHits.end(), 
                 [](const TimedHitPtr<xAOD::MuonSimHit>& a, const TimedHitPtr<xAOD::MuonSimHit>& b){
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
    StatusCode MuonDigitizationTool::processAllSubEvents(const EventContext& ctx) {
        PileUpHits hitCollList{};
        /// In case of single hits container just load the collection using read handles    
        if (!m_onlyUseContainerName) {            
            const xAOD::MuonSimHitContainer* hitCollection{nullptr};
            ATH_CHECK(retrieveContainer(ctx, m_simHitKey, hitCollection));
            hitCollList.emplace_back(PileUpTimeEventIndex(0), hitCollection);
         } else {
            ATH_CHECK(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList));
        }
        ATH_CHECK(fillTimedHits(hitCollList));
        ATH_CHECK(mergeEvent(ctx));
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
        return StatusCode::SUCCESS;
    }


    StatusCode MuonDigitizationTool::processBunchXing(int bunchXing, 
                                                      SubEventIterator bSubEvents, 
                                                      SubEventIterator eSubEvents) {        
        ATH_MSG_DEBUG("processBunchXing()" << bunchXing);
        PileUpHits hitList{};
        ATH_CHECK(m_mergeSvc->retrieveSubSetEvtData(m_inputObjectName, hitList, bunchXing, bSubEvents, eSubEvents));
        ATH_MSG_VERBOSE(hitList.size() << " hits in  xAODMuonSimHitContainer " << m_inputObjectName << " found");
        ATH_CHECK(fillTimedHits(hitList));
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
    void MuonDigitizationTool::addSDO(const TimedHit& hit, xAOD::MuonSimHitContainer* sdoContainer) const{
        if(!sdoContainer) {
            ATH_MSG_VERBOSE("No SDO container setup of writing");
            return;
        }
        if (!m_includePileUpTruth && HepMC::ignoreTruthLink(hit->genParticleLink(), m_vetoPileUpTruthLinks)) { 
            ATH_MSG_VERBOSE("Hit "<<m_idHelperSvc->toString(hit->identify())<<" is a pile-up truth link");
            return; 
        }
        
        xAOD::MuonSimHit* sdoHit = sdoContainer->push_back(std::make_unique<xAOD::MuonSimHit>());
        (*sdoHit) = (*hit);
        static const SG::Accessor<float> acc_eventTime{"SDO_evtTime"};
        static const SG::Accessor<unsigned short> acc_eventID{"SDO_evtID"};
        static const SG::Accessor<unsigned short> acc_puType{"SDO_puType"};
        acc_eventTime(*sdoHit) = hit.eventTime();
        acc_eventID(*sdoHit) = hit.eventId();
        acc_puType(*sdoHit) = hit.pileupType();
    }
}