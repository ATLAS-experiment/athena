/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitTosTGCMeasCnvAlg.h"

#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <xAODMuonPrepData/sTgcStripAuxContainer.h>
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
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_uncertCalibKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
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
    
    SG::WriteHandle<xAOD::sTgcStripContainer> prdContainer{m_writeKey, ctx};
    ATH_CHECK(prdContainer.record(std::make_unique<xAOD::sTgcStripContainer>(),
                                  std::make_unique<xAOD::sTgcStripAuxContainer>()));

    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

    for(const xAOD::MuonSimHit* simHit : *simHitContainer){
        const Identifier hitId = simHit->identify();
        //ignore radiation for now
        if(std::abs(simHit->pdgId())!=13) continue;


        const MuonGMR4::sTgcReadoutElement* readOutEle = m_DetMgr->getsTgcReadoutElement(hitId);
        bool isValid{false};
        Identifier simStripLayerIdentifier = id_helper.channelID(hitId, id_helper.multilayer(hitId), id_helper.gasGap(hitId),1, 1, isValid);
        if(!isValid) {
            ATH_MSG_WARNING("Invalid layer identifier");
            continue;
        }
        const Amg::Vector3D lHitPos{xAOD::toEigen(simHit->localPosition())};
        int channelNumber = readOutEle->stripDesign(simStripLayerIdentifier).stripNumber(lHitPos.block<2,1>(0,0));
        Identifier simStripChannelIdentifier = id_helper.channelID(hitId, id_helper.multilayer(hitId), id_helper.gasGap(hitId),1, channelNumber, isValid);
        if(channelNumber==-1){
            ATH_MSG_WARNING("hit is outside bounds, rejecting it");
            continue;
        }
        if(!isValid) ATH_MSG_ERROR("Invalid strip identifier for layer " << m_idHelperSvc->toString(simStripLayerIdentifier) << " channel " << channelNumber << " lHitPos " << Amg::toString(lHitPos));
        
        xAOD::sTgcStripCluster* prd = new xAOD::sTgcStripCluster();
        prdContainer->push_back(prd);
        prd->setIdentifier(simStripChannelIdentifier.get_compact());

        NswErrorCalibData::Input errorCalibInput{};
        errorCalibInput.stripId=simStripChannelIdentifier;
        errorCalibInput.locTheta = M_PI- simHit->localDirection().theta();
        errorCalibInput.clusterAuthor=3; // centroid
        double uncert = errorCalibDB->clusterUncertainty(errorCalibInput);
        ATH_MSG_VERBOSE ("stgc hit has theta " << errorCalibInput.locTheta  / Gaudi::Units::deg << " and uncertainty " << uncert);

        double newLocalX = CLHEP::RandGaussZiggurat::shoot(rndEngine, lHitPos.x(), uncert);
        xAOD::MeasVector<1> lClusterPos{newLocalX};
        xAOD::MeasMatrix<1> lCov{uncert*uncert}; 
        prd->setMeasurement(m_idHelperSvc->detElementHash(simStripChannelIdentifier) ,lClusterPos, lCov);

    }

    return StatusCode::SUCCESS;

}




CLHEP::HepRandomEngine* xAODSimHitTosTGCMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}