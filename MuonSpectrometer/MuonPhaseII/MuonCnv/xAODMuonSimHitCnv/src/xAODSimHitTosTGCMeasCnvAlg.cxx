/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitTosTGCMeasCnvAlg.h"

#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>

#include <xAODMuonPrepData/sTgcStripAuxContainer.h>
#include <xAODMuonPrepData/sTgcWireAuxContainer.h>
#include <xAODMuonPrepData/sTgcPadAuxContainer.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteHandle.h>
#include <CLHEP/Random/RandGaussZiggurat.h>
// Random Numbers
#include <AthenaKernel/RNGWrapper.h>
#include <MuonCondData/Defs.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
xAODSimHitTosTGCMeasCnvAlg::xAODSimHitTosTGCMeasCnvAlg(const std::string& name, 
                                                     ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitTosTGCMeasCnvAlg::initialize(){
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_writeKeyStrip.initialize());
    ATH_CHECK(m_writeKeyPad.initialize());
    ATH_CHECK(m_writeKeyWire.initialize());
    ATH_CHECK(m_rndmSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_uncertCalibKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
}


void xAODSimHitTosTGCMeasCnvAlg::digitizeStrip(const EventContext& ctx,
                                               const xAOD::MuonSimHit& simHit,
                                               xAOD::sTgcStripContainer& prdContainer,
                                               CLHEP::HepRandomEngine* rndEngine) const {
    
    
    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    const Identifier hitId{simHit.identify()};
    const MuonGMR4::sTgcReadoutElement* readOutEle = m_DetMgr->getsTgcReadoutElement(hitId);
   
    const Amg::Vector3D lHitPos{xAOD::toEigen(simHit.localPosition())};
   
    int channelNumber = readOutEle->stripDesign(hitId).stripNumber(lHitPos.block<2,1>(0,0));
    if(channelNumber==-1){
        ATH_MSG_WARNING("hit "<<Amg::toString(lHitPos)<<" is outside bounds "<<
                        readOutEle->stripDesign(hitId)<<" rejecting it");
        return;
    }
    bool isValid{false};
    const Identifier stripId = id_helper.channelID(hitId, 
                                                  id_helper.multilayer(hitId), 
                                                  id_helper.gasGap(hitId),
                                                  sTgcIdHelper::sTgcChannelTypes::Strip, 
                                                  channelNumber, 
                                                  isValid);
    if(!isValid) {
        ATH_MSG_WARNING("Invalid strip identifier for layer " << m_idHelperSvc->toString(hitId) 
                        << " channel " << channelNumber << " lHitPos " << Amg::toString(lHitPos));
        return;
    }
    NswErrorCalibData::Input errorCalibInput{};
    errorCalibInput.stripId= stripId;
    errorCalibInput.locTheta = M_PI - simHit.localDirection().theta();
    errorCalibInput.clusterAuthor = 3; // centroid
    
    SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
    double uncert = errorCalibDB->clusterUncertainty(errorCalibInput);

    xAOD::sTgcStripCluster* prd = prdContainer.push_back(new xAOD::sTgcStripCluster());
    prd->setIdentifier(stripId.get_compact());

    ATH_MSG_VERBOSE ("stgc hit has theta " << errorCalibInput.locTheta  / Gaudi::Units::deg << " and uncertainty " << uncert);

    const double newLocalX = CLHEP::RandGaussZiggurat::shoot(rndEngine, lHitPos.x(), uncert);
    xAOD::MeasVector<1> lClusterPos{newLocalX};
    xAOD::MeasMatrix<1> lCov{uncert*uncert}; 
    prd->setMeasurement(readOutEle->identHash() ,lClusterPos, lCov);
    prd->setChannelNumber(channelNumber);
    prd->setGasGap(id_helper.gasGap(hitId));
    prd->setReadoutElement(readOutEle);
}

void xAODSimHitTosTGCMeasCnvAlg::digitizeWire(const EventContext& ctx,
                                              const xAOD::MuonSimHit& simHit,
                                              xAOD::sTgcWireContainer& prdContainer,
                                              CLHEP::HepRandomEngine* rndEngine) const {

    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    const Identifier hitId{simHit.identify()};
    const MuonGMR4::sTgcReadoutElement* readOutEle = m_DetMgr->getsTgcReadoutElement(hitId);

    const IdentifierHash stripHash{MuonGMR4::sTgcReadoutElement::createHash(id_helper.gasGap(hitId),
                                                                           sTgcIdHelper::sTgcChannelTypes::Strip, 0)};

    const IdentifierHash wireHash{MuonGMR4::sTgcReadoutElement::createHash(id_helper.gasGap(hitId),
                                                                           sTgcIdHelper::sTgcChannelTypes::Wire, 0)};

    SG::ReadHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
    const Amg::Transform3D layerRot{readOutEle->globalToLocalTrans(*gctx, wireHash) *
                                    readOutEle->localToGlobalTrans(*gctx, stripHash)};
    const Amg::Vector3D lHitPos{layerRot*xAOD::toEigen(simHit.localPosition())};

    const MuonGMR4::WireGroupDesign& design{readOutEle->wireDesign(hitId)};
    int channelNumber = design.stripNumber(lHitPos.block<2,1>(0,0));
    if(channelNumber==-1){
        ATH_MSG_WARNING("hit "<<Amg::toString(lHitPos)<<" is outside wire bounds "<<design<<" rejecting it");
        return;
    }
    bool isValid{false};
    const Identifier wireId = id_helper.channelID(hitId, 
                                                  id_helper.multilayer(hitId), 
                                                  id_helper.gasGap(hitId),
                                                  sTgcIdHelper::sTgcChannelTypes::Wire, 
                                                  channelNumber, 
                                                  isValid);
    if(!isValid) {
        ATH_MSG_WARNING("Invalid wire identifier for layer " << m_idHelperSvc->toString(hitId) 
                        << " channel " << channelNumber << " lHitPos " << Amg::toString(lHitPos));
        return;
    }
    const double wireUncert = design.numWiresInGroup(channelNumber) * design.stripPitch() / std::sqrt(12);
    const double newLocalX = CLHEP::RandGaussZiggurat::shoot(rndEngine, lHitPos.x(), wireUncert);

    xAOD::sTgcWireHit* prd = prdContainer.push_back(new xAOD::sTgcWireHit());
    prd->setIdentifier(wireId.get_compact());
    xAOD::MeasVector<1> lClusterPos{newLocalX};
    xAOD::MeasMatrix<1> lCov{wireUncert*wireUncert}; 
    prd->setMeasurement(readOutEle->identHash() ,lClusterPos, lCov);
    prd->setChannelNumber(channelNumber);
    prd->setGasGap(id_helper.gasGap(hitId));
    prd->setReadoutElement(readOutEle);
}


void xAODSimHitTosTGCMeasCnvAlg::digitizePad(const EventContext& ctx,
                                             const xAOD::MuonSimHit& simHit,
                                             xAOD::sTgcPadContainer& prdContainer) const {

    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    const Identifier hitId{simHit.identify()};
    const MuonGMR4::sTgcReadoutElement* readOutEle = m_DetMgr->getsTgcReadoutElement(hitId);

    const IdentifierHash stripHash{MuonGMR4::sTgcReadoutElement::createHash(id_helper.gasGap(hitId),
                                                                           sTgcIdHelper::sTgcChannelTypes::Strip, 0)};

    const IdentifierHash padHash{MuonGMR4::sTgcReadoutElement::createHash(id_helper.gasGap(hitId),
                                                                           sTgcIdHelper::sTgcChannelTypes::Pad, 0)};

    SG::ReadHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
    const Amg::Transform3D layerRot{readOutEle->globalToLocalTrans(*gctx, padHash) *
                                    readOutEle->localToGlobalTrans(*gctx, stripHash)};
    
    const Amg::Vector3D lHitPos{layerRot*xAOD::toEigen(simHit.localPosition())};

    /// Need to find out how the inverse function can be made
    // const MuonGMR4::PadDesign& design{readOutEle->padDesign(hitId)};

    xAOD::sTgcPadHit* prd = prdContainer.push_back(new xAOD::sTgcPadHit());
    prd->setIdentifier(hitId.get_compact());
    prd->setGasGap(id_helper.gasGap(hitId));
    prd->setReadoutElement(readOutEle);


}


StatusCode xAODSimHitTosTGCMeasCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::MuonSimHitContainer> simHitContainer{m_readKey, ctx};
    if (!simHitContainer.isPresent()){
        ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
    if (!errorCalibDB.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    SG::WriteHandle<xAOD::sTgcStripContainer> stripContainer{m_writeKeyStrip, ctx};
    ATH_CHECK(stripContainer.record(std::make_unique<xAOD::sTgcStripContainer>(),
                                    std::make_unique<xAOD::sTgcStripAuxContainer>()));

    SG::WriteHandle<xAOD::sTgcWireContainer> wireContainer{m_writeKeyWire, ctx};
    ATH_CHECK(wireContainer.record(std::make_unique<xAOD::sTgcWireContainer>(),
                                   std::make_unique<xAOD::sTgcWireAuxContainer>()));

    SG::WriteHandle<xAOD::sTgcPadContainer> padContainer{m_writeKeyPad, ctx};
    ATH_CHECK(padContainer.record(std::make_unique<xAOD::sTgcPadContainer>(),
                                  std::make_unique<xAOD::sTgcPadAuxContainer>()));

    

    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

    for(const xAOD::MuonSimHit* simHit : *simHitContainer){
        //ignore radiation for now
        if(std::abs(simHit->pdgId())!=13) continue;
        digitizeStrip(ctx, *simHit, *stripContainer, rndEngine);

        

    }

    return StatusCode::SUCCESS;

}
CLHEP::HepRandomEngine* xAODSimHitTosTGCMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}