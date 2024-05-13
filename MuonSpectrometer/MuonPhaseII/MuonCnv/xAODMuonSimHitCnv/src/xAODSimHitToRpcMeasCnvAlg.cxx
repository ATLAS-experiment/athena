/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitToRpcMeasCnvAlg.h"

#include <xAODMuonPrepData/RpcStripAuxContainer.h>
#include <xAODMuonPrepData/RpcStrip2DAuxContainer.h>

#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>

#include <StoreGate/ReadHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteHandle.h>
#include <CLHEP/Random/RandGaussZiggurat.h>
#include <CLHEP/Random/RandFlat.h>
#include <GaudiKernel/PhysicalConstants.h>
// Random Numbers
#include <AthenaKernel/RNGWrapper.h>

namespace {
    constexpr double invC = 1./ Gaudi::Units::c_light;
    constexpr double percentage( unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }

    using CheckVector2D = MuonGMR4::CheckVector2D;

}

xAODSimHitToRpcMeasCnvAlg::xAODSimHitToRpcMeasCnvAlg(const std::string& name, 
                                                     ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitToRpcMeasCnvAlg::initialize(){
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_writeKeyBI.initialize(!m_writeKeyBI.empty()));
    
    ATH_CHECK(m_idHelperSvc.retrieve());
    m_stIdxBIL = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL");
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
}
StatusCode xAODSimHitToRpcMeasCnvAlg::finalize() {
    ATH_MSG_INFO("Tried to convert "<<m_allHits[0]<<"/"<<m_allHits[1]<<" hits. In, "
                <<percentage(m_acceptedHits[0], m_allHits[0]) <<"/"
                <<percentage(m_acceptedHits[1], m_allHits[1]) <<" cases, the conversion was successful");
    return StatusCode::SUCCESS;
}
void xAODSimHitToRpcMeasCnvAlg::digitizeHit(const double hitTime,
                                            const double locPosOnStrip,
                                            const MuonGMR4::StripDesignPtr& designPtr,
                                            const Identifier& gasGapId,
                                            const bool measuresPhi,
                                            xAOD::RpcStripContainer& prdContainer,
                                            CLHEP::HepRandomEngine* rndEngine) const {

    /// There're Rpc chambers without phi strips (BI)
    if (!designPtr){
        return;
    }
    const MuonGMR4::StripDesign& design{*designPtr};

    const double uncert = design.stripPitch() / std::sqrt(12.);
    const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locPosOnStrip, uncert);
    const Amg::Vector2D locHitPos{smearedX * Amg::Vector2D::UnitX()};
    ++(m_allHits[measuresPhi]);
    if (!design.insideTrapezoid(locHitPos)) {
        ATH_MSG_VERBOSE("The hit "<<Amg::toString(locHitPos)<<" is outside of the trapezoid bounds for "
                        <<m_idHelperSvc->toStringGasGap(gasGapId)<<", measuresPhi: "<<(measuresPhi ? "yay" : "nay"));
        return;
    }
    const int strip = stripNumber(design, locHitPos);
    if (strip < 0) {
        ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) << " cannot trigger any signal in a strip for "
                        << m_idHelperSvc->toStringGasGap(gasGapId) <<", measuresPhi: "<<(measuresPhi ? "yay" : "nay"));
        return;
    }

    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};

    bool isValid{false};
    const Identifier prdId{id_helper.channelID(gasGapId, 
                                              id_helper.doubletZ(gasGapId), 
                                              id_helper.doubletPhi(gasGapId), 
                                              id_helper.gasGap(gasGapId),
                                              measuresPhi, strip, isValid)};

    if (!isValid) {
        ATH_MSG_WARNING("Invalid hit identifier obtained for "<<m_idHelperSvc->toStringGasGap(gasGapId)
                        <<",  eta strip "<<strip<<" & hit "<<Amg::toString(locHitPos,2 )
                        <<" /// "<<design);
        return;
    }
    ++(m_acceptedHits[measuresPhi]);
    
    xAOD::RpcStrip* prd = new xAOD::RpcStrip();
    prdContainer.push_back(prd);
    
    prd->setIdentifier(prdId.get_compact());
    xAOD::MeasVector<1> lPos{smearedX};
    xAOD::MeasMatrix<1> cov{uncert * uncert};
    prd->setMeasurement<1>(m_idHelperSvc->detElementHash(prdId), 
                          std::move(lPos), std::move(cov));
    const MuonGMR4::RpcReadoutElement* readOutEle = m_DetMgr->getRpcReadoutElement(prdId);
    prd->setReadoutElement(readOutEle);
    prd->setStripNumber(strip);
    prd->setGasGap(id_helper.gasGap(prdId));
    prd->setDoubletPhi(id_helper.doubletPhi(prdId));
    prd->setMeasuresPhi(id_helper.measuresPhi(prdId));
    prd->setTime(hitTime);
    prd->setAmbiguityFlag(0);
}

void xAODSimHitToRpcMeasCnvAlg::digitizeHit(const double hitTime,
                                            const Amg::Vector2D& locPosOnStrip,
                                            const MuonGMR4::StripDesignPtr& designPtr,
                                            const Identifier& gasGapId,
                                            xAOD::RpcStrip2DContainer& prdContainer,
                                            CLHEP::HepRandomEngine* rndEngine) const{

    /// There're Rpc chambers without phi strips (BI)
    if (!designPtr){
        return;
    }
    const MuonGMR4::StripDesign& design{*designPtr};

    const double uncert = design.stripPitch() / std::sqrt(12.);
    const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locPosOnStrip.x(), uncert);
    const Amg::Vector2D locHitPos{smearedX, locPosOnStrip.y()};
    ++(m_allHits[false]);
    if (!design.insideTrapezoid(locHitPos)) {
        ATH_MSG_VERBOSE("The 2D hit "<<Amg::toString(locHitPos)<<" is outside of the trapezoid bounds for "
                        <<m_idHelperSvc->toStringGasGap(gasGapId));
        return;
    }
    const int strip = stripNumber(design, locHitPos);
    if (strip < 0) {
        ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) << " cannot trigger any signal in a strip for "
                        << m_idHelperSvc->toStringGasGap(gasGapId));
        return;
    }

    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};

    bool isValid{false};
    const Identifier prdId{id_helper.channelID(gasGapId, 
                                              id_helper.doubletZ(gasGapId), 
                                              id_helper.doubletPhi(gasGapId), 
                                              id_helper.gasGap(gasGapId),
                                              false, strip, isValid)};

    if (!isValid) {
        ATH_MSG_WARNING("Invalid hit identifier obtained for "<<m_idHelperSvc->toStringGasGap(gasGapId)
                        <<",  eta strip "<<strip<<" & hit "<<Amg::toString(locHitPos,2 )
                        <<" /// "<<design);
        return;
    }
    ++(m_acceptedHits[false]);
    
    xAOD::RpcStrip2D* prd = new xAOD::RpcStrip2D();
    prdContainer.push_back(prd);
    
    prd->setIdentifier(prdId.get_compact());
    xAOD::MeasVector<2> lPos{};
    xAOD::MeasMatrix<2> cov{xAOD::MeasMatrix<2>::Identity()};
    cov(0,0) = uncert * uncert;
    /// Dummy value for the moment
    cov(1,1) = 1.;
    prd->setMeasurement<2>(m_idHelperSvc->detElementHash(prdId), 
                           xAOD::toStorage(locHitPos), std::move(cov));
    const MuonGMR4::RpcReadoutElement* readOutEle = m_DetMgr->getRpcReadoutElement(prdId);
    prd->setReadoutElement(readOutEle);
    prd->setStripNumber(strip);
    prd->setGasGap(id_helper.gasGap(prdId));
    prd->setDoubletPhi(id_helper.doubletPhi(prdId));
    prd->setTime(hitTime);
    prd->setAmbiguityFlag(0);
}

int xAODSimHitToRpcMeasCnvAlg::stripNumber(const MuonGMR4::StripDesign& design,
                                           const Amg::Vector2D& locHitPos) const {

    int strip = design.stripNumber(locHitPos);
    if (strip > 0) return strip;

    const CheckVector2D firstStrip = design.center(1);
    const CheckVector2D lastStrip  = design.center(design.numStrips());
    if (!firstStrip || !lastStrip) {
        return -1;
    }
    if ( (*firstStrip).x() - 0.5 *design.stripPitch() < locHitPos.x()) {
        return 1;
    } else if ( (*lastStrip).x() + 0.5 * design.stripPitch() > locHitPos.x()) {
        return design.numStrips();
    } 
    return -1;
}

StatusCode xAODSimHitToRpcMeasCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::MuonSimHitContainer> simHitContainer{m_readKey, ctx};
    if (!simHitContainer.isPresent()){
        ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }

    SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
    ATH_CHECK(gctxHandle.isPresent());
    const ActsGeometryContext& gctx{*gctxHandle};

    SG::WriteHandle<xAOD::RpcStripContainer> prdContainer{m_writeKey, ctx};
    ATH_CHECK(prdContainer.record(std::make_unique<xAOD::RpcStripContainer>(),
                                  std::make_unique<xAOD::RpcStripAuxContainer>()));
    
    SG::WriteHandle<xAOD::RpcStrip2DContainer> prd2DContainer{};
    if (!m_writeKeyBI.empty()) {
        prd2DContainer = SG::WriteHandle<xAOD::RpcStrip2DContainer>{m_writeKeyBI, ctx};
        ATH_CHECK(prd2DContainer.record(std::make_unique<xAOD::RpcStrip2DContainer>(),
                                        std::make_unique<xAOD::RpcStrip2DAuxContainer>()));

    }
    
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

    for (const xAOD::MuonSimHit* simHit : *simHitContainer) {
        const Identifier hitId = simHit->identify();
        // ignore radiation for now
        if (std::abs(simHit->pdgId()) != 13) continue;
        const MuonGMR4::RpcReadoutElement* readOutEle = m_DetMgr->getRpcReadoutElement(hitId);
        const Amg::Vector3D locSimHitPos{xAOD::toEigen(simHit->localPosition())};
        const double hitTime = simHit->globalTime() - invC *(readOutEle->localToGlobalTrans(gctx, hitId) * locSimHitPos).mag();

        if (m_idHelperSvc->stationName(hitId) != m_stIdxBIL) {
            digitizeHit(hitTime, locSimHitPos.x(), readOutEle->getParameters().etaDesign, hitId, false, *prdContainer, rndEngine);
            digitizeHit(hitTime, locSimHitPos.y(), readOutEle->getParameters().phiDesign, hitId, true, *prdContainer, rndEngine);
        } else if (!m_writeKeyBI.empty()) {
            digitizeHit(hitTime, locSimHitPos.block<2,1>(0,0), readOutEle->getParameters().etaDesign, hitId, 
                        *prd2DContainer, rndEngine);
        }
    }
    return StatusCode::SUCCESS;
}

CLHEP::HepRandomEngine* xAODSimHitToRpcMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}
