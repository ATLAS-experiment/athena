/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitToRpcMeasCnvAlg.h"

#include <xAODMuonPrepData/RpcStripAuxContainer.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteHandle.h>
#include <CLHEP/Random/RandGaussZiggurat.h>
#include <GaudiKernel/PhysicalConstants.h>
// Random Numbers
#include <AthenaKernel/RNGWrapper.h>

namespace {
    constexpr double invC = 1./ Gaudi::Units::c_light;
}

xAODSimHitToRpcMeasCnvAlg::xAODSimHitToRpcMeasCnvAlg(const std::string& name, 
                                                     ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitToRpcMeasCnvAlg::initialize(){
    ATH_CHECK(m_surfaceProvTool.retrieve());
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
}
StatusCode xAODSimHitToRpcMeasCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::MuonSimHitContainer> simHitContainer{m_readKey, ctx};
    if (!simHitContainer.isPresent()){
        ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }

    const ActsGeometryContext gctx{};
    SG::WriteHandle<xAOD::RpcStripContainer> prdContainer{m_writeKey, ctx};
    ATH_CHECK(prdContainer.record(std::make_unique<xAOD::RpcStripContainer>(),
                                  std::make_unique<xAOD::RpcStripAuxContainer>()));
    
    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
    for (const xAOD::MuonSimHit* simHit : *simHitContainer) {
        const Identifier hitId = simHit->identify();
        // ignore radiation for now
        if (std::abs(simHit->pdgId()) != 13) continue;
        
       
        const MuonGMR4::RpcReadoutElement* readOutEle = m_DetMgr->getRpcReadoutElement(hitId);
        const MuonGMR4::StripDesign& etaDesign{*readOutEle->getParameters().etaDesign};
        
        const Amg::Vector3D locSimHitPos{xAOD::toEigen(simHit->localPosition())};

        const double etaUncert = etaDesign.stripPitch() / std::sqrt(12);
        const Amg::Vector2D smearedEtaPos{CLHEP::RandGaussZiggurat::shoot(rndEngine, locSimHitPos.x(), etaUncert), 0.};

        const double hitTime = simHit->globalTime() - 
                               invC *(readOutEle->localToGlobalTrans(gctx, hitId) * locSimHitPos).mag(); 

        const unsigned int etaStripNum = etaDesign.stripNumber(smearedEtaPos.block<2,1>(0,0));
        
        ATH_MSG_ALWAYS("Convert simulated hit "<<m_idHelperSvc->toStringGasGap(hitId)<<" located in gas gap at "
                        <<Amg::toString(locSimHitPos, 2)<<" eta strip number: "<<etaStripNum
                        <<" strip position "<<Amg::toString(etaDesign.center(etaStripNum).value_or(Amg::Vector2D::Zero()), 2));
        

        bool isValid{false};
        const Identifier etaHitId{id_helper.channelID(hitId, readOutEle->doubletZ(), 
                                                             id_helper.doubletPhi(hitId), 
                                                             id_helper.gasGap(hitId),
                                                             false, etaStripNum, isValid)};
        
        if (!isValid) {
            ATH_MSG_WARNING("Invalid hit identifier obtained for "<<m_idHelperSvc->toStringGasGap(hitId)
                            <<",  eta strip "<<etaStripNum<<" & hit "<<Amg::toString(locSimHitPos,2 ));
        } else {
            xAOD::RpcStrip* prd = new xAOD::RpcStrip();
            prdContainer->push_back(prd);
            prd->setIdentifier(etaHitId.get_compact());
            xAOD::MeasVector<1> lPos{smearedEtaPos.x()};
            xAOD::MeasMatrix<1> cov{etaUncert * etaUncert};
            prd->setMeasurement<1>(m_idHelperSvc->detElementHash(etaHitId), 
                                   std::move(lPos), std::move(cov));
            prd->setReadoutElement(readOutEle);
            prd->setStripNumber(etaStripNum);
            prd->setGasGap(id_helper.gasGap(etaHitId));
            prd->setDoubletPhi(id_helper.doubletPhi(etaHitId));
            prd->setMeasuresPhi(id_helper.measuresPhi(etaHitId));
            prd->setReadoutElement(readOutEle);
            prd->setTime(hitTime);
            prd->setAmbiguityFlag(0);
        }
        /// Check whether the read out element contains phi strips or not.
        if (!readOutEle->nPhiStrips()) {
            continue;
        }
        const MuonGMR4::StripDesign& phiDesign{*readOutEle->getParameters().phiDesign};
        const double phiUncert = phiDesign.stripPitch() / std::sqrt(12.);
        const Amg::Vector2D smearedPhiPos{CLHEP::RandGaussZiggurat::shoot(rndEngine, locSimHitPos.y(), phiUncert), 0.};
        
        const unsigned int phiStripNum = phiDesign.stripNumber(smearedPhiPos.block<2,1>(0,0));
        const Identifier phiHitId{id_helper.channelID(hitId, readOutEle->doubletZ(), 
                                                             id_helper.doubletPhi(hitId), 
                                                             id_helper.gasGap(hitId),
                                                             true, phiStripNum, isValid)};
        
        if (!isValid) {
            ATH_MSG_WARNING("Invalid hit identifier obtained for "<<m_idHelperSvc->toStringGasGap(hitId)
                            <<",  phi strip "<<phiStripNum<<" & hit "<<Amg::toString(locSimHitPos,2 ));
            continue;
        }

        xAOD::RpcStrip* prd = new xAOD::RpcStrip();
        prdContainer->push_back(prd);
        prd->setIdentifier(phiHitId.get_compact());
        xAOD::MeasVector<1> lPos{smearedPhiPos.x()};
        xAOD::MeasMatrix<1> cov{phiUncert * phiUncert};
        prd->setMeasurement<1>(m_idHelperSvc->detElementHash(phiHitId), 
                                std::move(lPos), std::move(cov));
        prd->setReadoutElement(readOutEle);
        prd->setStripNumber(phiStripNum);
        prd->setGasGap(id_helper.gasGap(phiHitId));
        prd->setDoubletPhi(id_helper.doubletPhi(phiHitId));
        prd->setMeasuresPhi(id_helper.measuresPhi(phiHitId));
        prd->setReadoutElement(readOutEle);
        prd->setTime(hitTime);
        prd->setAmbiguityFlag(0);
    }
    return StatusCode::SUCCESS;
}

CLHEP::HepRandomEngine* xAODSimHitToRpcMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}
