/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODSimHitToMdtMeasCnvAlg.h"

#include <MuonCalibEvent/MdtCalibHit.h>
#include <xAODMuonPrepData/MdtDriftCircleAuxContainer.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <StoreGate/WriteHandle.h>
#include <CLHEP/Random/RandGaussZiggurat.h>
// Random Numbers
#include <AthenaKernel/RNGWrapper.h>
namespace{
    static constexpr double timeToTdcCnv = 1./ IMdtCalibrationTool::tdcBinSize;
}
xAODSimHitToMdtMeasCnvAlg::xAODSimHitToMdtMeasCnvAlg(const std::string& name, 
                                                     ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode xAODSimHitToMdtMeasCnvAlg::initialize(){
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_calibDbKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_DetMgr));
    return StatusCode::SUCCESS;
}
StatusCode xAODSimHitToMdtMeasCnvAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::MuonSimHitContainer> simHitContainer{m_readKey, ctx};
    if (!simHitContainer.isPresent()){
        ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
        return StatusCode::FAILURE;
    }
    
    SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> mdtCalibData{m_calibDbKey, ctx};
    if (!mdtCalibData.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve calibration data "<<m_calibDbKey.fullKey());
        return StatusCode::FAILURE;
    }

    const ActsGeometryContext gctx{};
    SG::WriteHandle<xAOD::MdtDriftCircleContainer> prdContainer{m_writeKey, ctx};
    ATH_CHECK(prdContainer.record(std::make_unique<xAOD::MdtDriftCircleContainer>(),
                                  std::make_unique<xAOD::MdtDriftCircleAuxContainer>()));
    
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
    for (const xAOD::MuonSimHit* simHit : *simHitContainer) {
        const Identifier hitId = simHit->identify();
        // ignore radiation for now
        if (std::abs(simHit->pdgId()) != 13) continue;
        
       
        xAOD::MdtDriftCircle* prd = new xAOD::MdtDriftCircle();
        prdContainer->push_back(prd);
        prd->setIdentifier(hitId.get_compact());
        prd->setIdentifierHash(m_idHelperSvc->detElementHash(hitId));
        prd->setLayer(id_helper.tubeLayer(hitId));
        prd->setTube(id_helper.tube(hitId));
        /// Define the identifier
        const MuonGMR4::MdtReadoutElement* readOutEle = m_DetMgr->getMdtReadoutElement(hitId);
        prd->setReadoutElement(readOutEle);

        const Amg::Vector3D globTubePos = readOutEle->center(gctx, prd->measurementHash());
        prd->setTubePosInStation(xAOD::toStorage(readOutEle->getChamber()->globalToLocalTrans(gctx) * globTubePos));
        /// extract the resolution from the Mdt calibration data
        bool bound{false};
        const MuonCalib::MdtFullCalibData* tubeContants = mdtCalibData->getCalibData(hitId, msgStream());
        const Amg::Vector3D lHitPos{xAOD::toEigen(simHit->localPosition())};
        const double driftTime = tubeContants->rtRelation->tr()->tFromR(lHitPos.perp(), bound);
        const double resol = tubeContants->rtRelation->rtRes()->resolution(driftTime);
        // Project and smear the hit
        double smearedDriftRadius = CLHEP::RandGaussZiggurat::shoot(rndEngine, lHitPos.perp(), resol);
        prd->setDriftRadius(std::min(readOutEle->innerTubeRadius(), std::max(0.,smearedDriftRadius)));
        prd->setDriftRadCov(resol*resol);
        /// The sMdts have HPTDC chips built in which run at a 4 times higher frequency than the chips currently
        /// in use for the Run 3 data taking. We have to reassess this once, I've read about the chip design in 
        /// use for Phase II
        const uint16_t tdcCounts = driftTime * timeToTdcCnv* (m_idHelperSvc->hasHPTDC(hitId) ? 4. : 1.);
        prd->setTdc(tdcCounts);

    }
    return StatusCode::SUCCESS;
}

CLHEP::HepRandomEngine* xAODSimHitToMdtMeasCnvAlg::getRandomEngine(const EventContext& ctx) const  {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
    std::string rngName = name() + m_streamName;
    rngWrapper->setSeed(rngName, ctx);
    return rngWrapper->getEngine(ctx);
}
