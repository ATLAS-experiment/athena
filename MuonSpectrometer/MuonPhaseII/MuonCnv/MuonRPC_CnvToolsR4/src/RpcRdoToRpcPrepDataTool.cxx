/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcRdoToRpcPrepDataTool.h"

#include <StoreGate/WriteHandle.h>
#include <MuonCablingData/RpcCablingData.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <xAODMuonPrepData/RpcStrip2DAuxContainer.h>
#include <xAODMuonPrepData/RpcStripAuxContainer.h>
#include <xAODMuonPrepData/RpcMeasurement.h>
#include <MuonIdHelpers/IdentifierByDetElSorter.h>


namespace MuonR4{
    RpcRdoToRpcPrepDataTool::RpcRdoToRpcPrepDataTool(const std::string& type,
                                                     const std::string& name,
                                                     const IInterface* parent): 
        base_class(type, name, parent) {}


    StatusCode RpcRdoToRpcPrepDataTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_rdoKey.initialize());
        ATH_CHECK(m_cablingKey.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_writeKeyBI.initialize(m_decode2DStrips));
        return StatusCode::SUCCESS;
    }
    
    StatusCode RpcRdoToRpcPrepDataTool::decode(const EventContext& ctx, 
                                               const std::vector<IdentifierHash>& idVect) const {
        
        
        SG::ReadHandle rdoContainer{m_rdoKey, ctx};
        ATH_CHECK(rdoContainer.isPresent());
        
        SG::ReadCondHandle cablingMap{m_cablingKey, ctx};
        ATH_CHECK(cablingMap.isValid());
        
        const std::unordered_set<IdentifierHash> hashToSelect(idVect.begin(), idVect.end());        
        using RdoPairs = std::array<const xAOD::NRPCRDO*, 2>;
        std::map<Identifier, RdoPairs, Muon::IdentifierByDetElSorter> sortedRdos{Muon::IdentifierByDetElSorter{m_idHelperSvc.get()}};
        for (const xAOD::NRPCRDO* rdo : *rdoContainer){
            /* cabling data conversion */
            Muon::RpcCablingData cabling{};
            cabling.subDetector = rdo->subdetector();
            cabling.boardSector = rdo->boardsector();
            cabling.board = rdo->board();
            cabling.channelId = rdo->channel();


            if (!cablingMap->getOfflineId(cabling, msgStream())){
                return StatusCode::FAILURE;
            }
            Identifier offId{};
            if (!cablingMap->convert(cabling, offId)){
                ATH_MSG_FATAL("Cabling conversion failed "<<cabling);
                return StatusCode::FAILURE;
            }
            if (hashToSelect.size() && !hashToSelect.count(m_idHelperSvc->moduleHash(offId))) {
                ATH_MSG_VERBOSE("Skip "<<m_idHelperSvc->toString(offId)<<" due to ROI selection ");
                continue;
            }
            ///
            const bool stripSide = cabling.stripSide();
            sortedRdos[offId][stripSide] = rdo;
        }

        SG::WriteHandle<xAOD::RpcStripContainer> stripHandle{m_writeKey, ctx};
        ATH_CHECK(stripHandle.record(std::make_unique<xAOD::RpcStripContainer>(),
                                     std::make_unique<xAOD::RpcStripAuxContainer>()));

        SG::WriteHandle<xAOD::RpcStrip2DContainer> strip2DHandle{};
        if (!m_writeKeyBI.empty()) {
            strip2DHandle =  SG::WriteHandle<xAOD::RpcStrip2DContainer>{m_writeKeyBI, ctx};
            ATH_CHECK(strip2DHandle.record(std::make_unique<xAOD::RpcStrip2DContainer>(),
                                           std::make_unique<xAOD::RpcStrip2DAuxContainer>()));

        }

        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        
        auto setMeasValues = [&idHelper,this](xAOD::RpcMeasurement* outputMeas,
                                              const xAOD::NRPCRDO* rdo,
                                              const Identifier& offId){
            
            const MuonGMR4::RpcReadoutElement* reElement = m_detMgr->getRpcReadoutElement(offId);
            outputMeas->setIdentifierHash(m_idHelperSvc->detElementHash(offId));
            outputMeas->setReadoutElement(reElement);
            outputMeas->setIdentifier(offId.get_compact());
            outputMeas->setDoubletPhi(idHelper.doubletPhi(offId));
            outputMeas->setGasGap(idHelper.gasGap(offId));
            outputMeas->setStripNumber(idHelper.channel(offId));
            outputMeas->setTimeOverThreshold(rdo->timeoverthr());
            /** TODO: Do we need to apply a time of flight correction here? */
            outputMeas->setTime(rdo->time());
            outputMeas->setTimeCovariance(std::pow(m_stripTimeResolution,2));
        };
        
        /** Next step convert the RDOs into prepdata objects */
        using CheckVector2D = MuonGMR4::StripDesign::CheckVector2D;
        for (const auto& [offId, rdoPairs] : sortedRdos) {
            const MuonGMR4::RpcReadoutElement* reElement = m_detMgr->getRpcReadoutElement(offId);
            
            const MuonGMR4::StripDesign& design{reElement->sensorLayout(reElement->measurementHash(offId)).design()};
            
            CheckVector2D stripPos = design.center(idHelper.channel(offId));
            if (!stripPos) {
                ATH_MSG_WARNING("Failed to fetch a valid stripPosition for "<<m_idHelperSvc->toString(offId));
                continue;
            }
            const double stripLocX = (*stripPos).x();
            const double stripCovX = std::pow(design.stripPitch(), 2) / std::sqrt(12.);

            if (m_decode2DStrips && rdoPairs[0] && rdoPairs[1]) {
                xAOD::RpcStrip2D* measurement = strip2DHandle->push_back(std::make_unique<xAOD::RpcStrip2D>());
                
                xAOD::MeasVector<2> lPos{xAOD::MeasVector<2>::Zero()};
                xAOD::MeasMatrix<2> lCov{xAOD::MeasMatrix<2>::Identity()};

                lPos[0] = stripLocX;
                lPos[1] = -0.5*m_propagationVelocity *(rdoPairs[0]->time() - rdoPairs[1]->time());
                lCov(0,0) = stripCovX;
                lCov(1,1) = M_SQRT1_2 * m_propagationVelocity* m_stripTimeResolution;
                /// Hash is overwritten by the setMeasValues method
                measurement->setMeasurement<2>(0, lPos, lCov);
                // CheckVector2D stripPos
                setMeasValues(measurement, rdoPairs[0], offId);
                ATH_MSG_VERBOSE("Measurement "<<m_idHelperSvc->toString(offId)<<" "<<lPos.x()<<", "<<lPos.y());
                continue;
            }

            xAOD::RpcStrip* strip = stripHandle->push_back(std::make_unique<xAOD::RpcStrip>());
            xAOD::MeasVector<1> lPos{xAOD::MeasVector<1>::Zero()};
            xAOD::MeasMatrix<1> lCov{xAOD::MeasMatrix<1>::Identity()};
            lPos[0] = stripLocX;
            lCov(0,0) = stripCovX;

            strip->setMeasuresPhi(idHelper.measuresPhi(offId));
             /// Hash is overwritten by the setMeasValues method
            strip->setMeasurement<1>(0, lPos, lCov);
            setMeasValues(strip, rdoPairs[0] ? rdoPairs[0] : rdoPairs[1], offId);
        }
        return StatusCode::SUCCESS;
    
    }
    StatusCode RpcRdoToRpcPrepDataTool::decode(const EventContext& ctx,
                                               const std::vector<uint32_t>& robIds) const {
        SG::ReadCondHandle cablingMap{m_cablingKey, ctx};
        ATH_CHECK(cablingMap.isValid());
        return decode(ctx, cablingMap->getChamberHashVec(robIds, msgStream()));
    }
    StatusCode RpcRdoToRpcPrepDataTool::provideEmptyContainer(const EventContext& ctx) const {
        SG::WriteHandle<xAOD::RpcStripContainer> stripHandle{m_writeKey, ctx};
        ATH_CHECK(stripHandle.record(std::make_unique<xAOD::RpcStripContainer>(),
                                     std::make_unique<xAOD::RpcStripAuxContainer>()));

        if (!m_writeKeyBI.empty()) {
            SG::WriteHandle<xAOD::RpcStrip2DContainer> strip2DHandle{m_writeKeyBI, ctx};
            ATH_CHECK(strip2DHandle.record(std::make_unique<xAOD::RpcStrip2DContainer>(),
                                           std::make_unique<xAOD::RpcStrip2DAuxContainer>()));

        }
        return StatusCode::SUCCESS;
    }

}
