/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitToMmMeasCnvAlg.h"

#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <xAODMuonPrepData/MMClusterAuxContainer.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteHandle.h>
#include <CLHEP/Random/RandGaussZiggurat.h>
// Random Numbers
#include <AthenaKernel/RNGWrapper.h>
#include <MuonCondData/Defs.h>
#include <GaudiKernel/SystemOfUnits.h>

xAODSimHitToMmMeasCnvAlg::xAODSimHitToMmMeasCnvAlg(const std::string& name, 
                                                     ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitToMmMeasCnvAlg::initialize(){
    ATH_CHECK(m_surfaceProvTool.retrieve());
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_uncertCalibKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
}


StatusCode xAODSimHitToMmMeasCnvAlg::execute(const EventContext& ctx) const {
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
    
    const ActsGeometryContext gctx{};
    SG::WriteHandle<xAOD::MMClusterContainer> prdContainer{m_writeKey, ctx};
    ATH_CHECK(prdContainer.record(std::make_unique<xAOD::MMClusterContainer>(),
                                  std::make_unique<xAOD::MMClusterAuxContainer>()));

    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

    for(const xAOD::MuonSimHit* simHit : *simHitContainer){
        const Identifier hitId = simHit->identify();
        //ignore radiation for now
        if(std::abs(simHit->pdgId())!=13) continue;


        const MuonGMR4::MmReadoutElement* readOutEle = m_DetMgr->getMmReadoutElement(hitId);
        bool isValid{false};

        const Amg::Vector3D lHitPos{xAOD::toEigen(simHit->localPosition())};
        const Amg::Vector2D lHitPos2D{lHitPos.x(), lHitPos.y()};
        
        int channelNumber = readOutEle->stripLayer(hitId).design().stripNumber(lHitPos2D);
        if(channelNumber==-1){
            ATH_MSG_WARNING("hit is outside bounds, rejecting it");
            continue;
        }
        Identifier clusId = id_helper.channelID(hitId, id_helper.multilayer(hitId), 
                                                       id_helper.gasGap(hitId), 
                                                       channelNumber, isValid);
        if(!isValid) {
            ATH_MSG_WARNING("Invalid strip identifier for layer " << m_idHelperSvc->toString(hitId) << " channel " << channelNumber << " lHitPos " << lHitPos.x() << " " << lHitPos.y());
            continue;
        }
        xAOD::MMCluster* prd = new xAOD::MMCluster();
        prdContainer->push_back(prd);
        
        prd->setChannelNumber(channelNumber);
        prd->setGasGap(id_helper.gasGap(clusId));
        prd->setIdentifier(clusId.get_compact());

        NswErrorCalibData::Input errorCalibInput{};
        errorCalibInput.stripId = clusId;
        errorCalibInput.locTheta = M_PI- simHit->localDirection().theta();
        errorCalibInput.clusterAuthor=66; // cluster time projection method
        double uncert = errorCalibDB->clusterUncertainty(errorCalibInput);
        ATH_MSG_VERBOSE("mm hit has theta " << errorCalibInput.locTheta / Gaudi::Units::deg << " and uncertainty " << uncert);

        double newLocalX = CLHEP::RandGaussZiggurat::shoot(rndEngine, lHitPos.x(), uncert);
        xAOD::MeasVector<1> lClusterPos{newLocalX};
        xAOD::MeasMatrix<1> lCov{uncert*uncert}; 
        prd->setMeasurement(m_idHelperSvc->detElementHash(clusId) ,lClusterPos, lCov);
        const Amg::Vector3D strip3D = lClusterPos.x() * Amg::Vector3D::UnitX();
        const Amg::Transform3D& globToCenter{m_surfaceProvTool->globalToChambCenter(gctx, clusId)};
        prd->setStripPosInStation(xAOD::toStorage(globToCenter * readOutEle->localToGlobalTrans(gctx, prd->layerHash()) * strip3D)); 

    }

    return StatusCode::SUCCESS;

}




CLHEP::HepRandomEngine* xAODSimHitToMmMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}