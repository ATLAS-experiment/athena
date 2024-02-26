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
    constexpr double percentage( unsigned int numerator, unsigned int denom) {
        return 100. * numerator / std::max(denom, 1u);
    }
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
StatusCode xAODSimHitToRpcMeasCnvAlg::finalize() {
    ATH_MSG_INFO("Tried to convert "<<m_allHits[0]<<"/"<<m_allHits[1]<<" hits. In, "
                <<percentage(m_acceptedHits[0], m_allHits[0]) <<"/"
                <<percentage(m_acceptedHits[1], m_allHits[1]) <<" cases, the conversion was successful");
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

    double hitTime{0.};
    const MuonGMR4::RpcReadoutElement* readOutEle{nullptr};

    using CheckVector2D = MuonGMR4::CheckVector2D;
    auto digitizeHit = [&] (const double locX,
                            const MuonGMR4::StripDesignPtr& designPtr,
                            const Identifier& hitId,                            
                            bool measPhi) {
        /// There're Rpc chambers without phi strips (BI)
        if (!designPtr){
            return;
        }
        const MuonGMR4::StripDesign& design{*designPtr};        
        const double uncert = design.stripPitch() / std::sqrt(12.);
        const double smearedX = CLHEP::RandGaussZiggurat::shoot(rndEngine, locX, uncert);
        const Amg::Vector2D locHitPos{locX * Amg::Vector2D::UnitX()};
        m_allHits[measPhi] = m_allHits[measPhi] + 1;
        if (!design.insideTrapezoid(locHitPos)) {
            ATH_MSG_VERBOSE("The hit "<<Amg::toString(locHitPos)<<" is outside of the trapezoid bounds for "
                            <<m_idHelperSvc->toStringGasGap(hitId)<<", measuresPhi: "<<(measPhi ? "yay" : "nay"));
            return;
        }
        int stripNumber = design.stripNumber(locHitPos);
        /// There're subtle cases where the hit is smeared outside the boundaries
        if (stripNumber < 0) {            
            const CheckVector2D firstStrip = design.center(1);
            const CheckVector2D lastStrip  = design.center(design.numStrips());
            if (!firstStrip || !lastStrip) {
                return;
            }
            if ( (*firstStrip).x() - 0.5 *design.stripPitch() < locHitPos.x()) {
                stripNumber = 1;
            } else if ( (*lastStrip).x() + 0.5 * design.stripPitch() > locHitPos.x()) {
                stripNumber = design.numStrips();
            } else {
                ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) << " cannot trigger any signal in a strip for "
                               << m_idHelperSvc->toStringGasGap(hitId) <<", measuresPhi: "<<(measPhi ? "yay" : "nay"));
                return;
            }
        }
        bool isValid{false};
        const Identifier prdId{id_helper.channelID(hitId, 
                                                   id_helper.doubletZ(hitId), 
                                                   id_helper.doubletPhi(hitId), 
                                                   id_helper.gasGap(hitId),
                                                   measPhi, stripNumber, isValid)};

        if (!isValid) {
            ATH_MSG_WARNING("Invalid hit identifier obtained for "<<m_idHelperSvc->toStringGasGap(hitId)
                            <<",  eta strip "<<stripNumber<<" & hit "<<Amg::toString(locHitPos,2 )
                            <<" /// "<<design);
            return;
        }
        m_acceptedHits[measPhi] =  m_acceptedHits[measPhi]  + 1;
        xAOD::RpcStrip* prd = new xAOD::RpcStrip();
        prdContainer->push_back(prd);
        prd->setIdentifier(prdId.get_compact());
        xAOD::MeasVector<1> lPos{smearedX};
        xAOD::MeasMatrix<1> cov{uncert * uncert};
        prd->setMeasurement<1>(m_idHelperSvc->detElementHash(prdId), 
                               std::move(lPos), std::move(cov));
        prd->setReadoutElement(readOutEle);
        prd->setStripNumber(stripNumber);
        prd->setGasGap(id_helper.gasGap(prdId));
        prd->setDoubletPhi(id_helper.doubletPhi(prdId));
        prd->setMeasuresPhi(id_helper.measuresPhi(prdId));
        prd->setReadoutElement(readOutEle);
        prd->setTime(hitTime);
        prd->setAmbiguityFlag(0);
        const Amg::Vector3D strip3D  = lPos.x() * Amg::Vector3D::UnitX();
        const Amg::Transform3D& globToCenter{m_surfaceProvTool->globalToChambCenter(gctx,prdId)};
        prd->setStripPosInStation(xAOD::toStorage(globToCenter * readOutEle->localToGlobalTrans(gctx,prd->layerHash()) * strip3D));
    };

    for (const xAOD::MuonSimHit* simHit : *simHitContainer) {
        const Identifier hitId = simHit->identify();
        // ignore radiation for now
        if (std::abs(simHit->pdgId()) != 13) continue;
        readOutEle = m_DetMgr->getRpcReadoutElement(hitId);
        const Amg::Vector3D locSimHitPos{xAOD::toEigen(simHit->localPosition())};
        hitTime = simHit->globalTime() - invC *(readOutEle->localToGlobalTrans(gctx, hitId) * locSimHitPos).mag();

        digitizeHit(locSimHitPos.x(), readOutEle->getParameters().etaDesign, hitId, false);
        digitizeHit(locSimHitPos.y(), readOutEle->getParameters().phiDesign, hitId, true);
        
    }
    return StatusCode::SUCCESS;
}

CLHEP::HepRandomEngine* xAODSimHitToRpcMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}
