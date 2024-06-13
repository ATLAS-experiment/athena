/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcDigitToRpcMeasCnvAlg.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

#include <xAODMuonPrepData/RpcStripAuxContainer.h>
#include <xAODMuonPrepData/RpcStrip2DAuxContainer.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>

namespace MuonR4 {
    RpcDigitToRpcMeasCnvAlg::RpcDigitToRpcMeasCnvAlg(const std::string& name, ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator}{}
    
    StatusCode RpcDigitToRpcMeasCnvAlg::initialize() {
        ATH_CHECK(m_digitKey.initialize());
        ATH_CHECK(m_stripKey.initialize());
        ATH_CHECK(m_stripBIKey.initialize(m_writeBIClust));
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        m_stIdx_BIL = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL"); 
        return StatusCode::SUCCESS;
    }
    StatusCode RpcDigitToRpcMeasCnvAlg::execute(const EventContext& ctx) const {
        SG::ReadHandle<RpcDigitContainer> readHandle{m_digitKey, ctx};
        ATH_CHECK(readHandle.isPresent());

        SG::WriteHandle<xAOD::RpcStripContainer> prdContainer{m_stripKey, ctx};
        ATH_CHECK(prdContainer.record(std::make_unique<xAOD::RpcStripContainer>(), 
                                      std::make_unique<xAOD::RpcStripAuxContainer>()));
        
        SG::WriteHandle<xAOD::RpcStrip2DContainer> prdContainerBI{};
        if (m_writeBIClust) {
            prdContainerBI = SG::WriteHandle<xAOD::RpcStrip2DContainer>{m_stripBIKey, ctx};
            ATH_CHECK(prdContainerBI.record(std::make_unique<xAOD::RpcStrip2DContainer>(),
                                            std::make_unique<xAOD::RpcStrip2DAuxContainer>()));
        }
        for (const RpcDigitCollection* coll : *readHandle) {
            for (const RpcDigit* digit : *coll) {
                if (!m_writeBIClust || m_idHelperSvc->stationName(digit->identify()) != m_stIdx_BIL) {
                    convert(*digit, *prdContainer);
                } else {
                    convert(*digit, *prdContainerBI);
                }
            }
        }
        return StatusCode::SUCCESS;
    }

    void RpcDigitToRpcMeasCnvAlg::convert(const RpcDigit& digit,
                                          xAOD::RpcStripContainer& strips) const {
        
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        xAOD::RpcStrip* strip = strips.push_back(std::make_unique<xAOD::RpcStrip>());
    
        const MuonGMR4::RpcReadoutElement* readoutEle = m_detMgr->getRpcReadoutElement(digit.identify());
        strip->setReadoutElement(readoutEle);
        
        strip->setDoubletPhi(idHelper.doubletPhi(digit.identify()));
        strip->setGasGap(idHelper.gasGap(digit.identify()));
        strip->setMeasuresPhi(idHelper.measuresPhi(digit.identify()));
        strip->setStripNumber(idHelper.channel(digit.identify()));

        const MuonGMR4::StripDesign& design{readoutEle->sensorLayout(strip->measurementHash()).design()};
        xAOD::MeasVector<1> lPos{xAOD::MeasVector<1>::Zero()};
        lPos[0] = (*design.center(strip->stripNumber())).x();
        xAOD::MeasMatrix<1> lCov{xAOD::MeasMatrix<1>::Identity()};
        lCov[0] = design.stripPitch() / std::sqrt(12);
        strip->setMeasurement(readoutEle->identHash(), std::move(lPos), std::move(lCov));
        strip->setIdentifier(digit.identify().get_compact());
        strip->setTime(digit.time());
        /* TODO define time uncertainties */
    }

    void RpcDigitToRpcMeasCnvAlg::convert(const RpcDigit& digit,
                                          xAOD::RpcStrip2DContainer& strips) const {
        
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        xAOD::RpcStrip2D* strip = strips.push_back(std::make_unique<xAOD::RpcStrip2D>());
    
        const MuonGMR4::RpcReadoutElement* readoutEle = m_detMgr->getRpcReadoutElement(digit.identify());
        strip->setReadoutElement(readoutEle);
        
        strip->setDoubletPhi(idHelper.doubletPhi(digit.identify()));
        strip->setGasGap(idHelper.gasGap(digit.identify()));
        strip->setStripNumber(idHelper.channel(digit.identify()));
    }
}