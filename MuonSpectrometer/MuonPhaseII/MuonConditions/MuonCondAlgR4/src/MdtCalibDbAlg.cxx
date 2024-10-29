/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaKernel/IOVInfiniteRange.h"

#include "MdtCalibDbAlg.h"
#include "MdtCalibR4/TrSqrt.h"
#include "MdtCalibR4/RtResolutionSqrt.h"
#include "MdtCalibData/MdtRtRelation.h"
#include "MdtCalibData/RootRtRelation.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "GaudiKernel/PhysicalConstants.h"

using RegionGranularity = MuonCalib::MdtCalibDataContainer::RegionGranularity;
using namespace MuonCalibR4;

MdtCalibDbAlg::MdtCalibDbAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MdtCalibDbAlg::initialize() {
    ATH_MSG_VERBOSE("Initialize...");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_r4detMgr));
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("Executing MdtCalibDbAlgR4");
    SG::WriteCondHandle<MuonCalib::MdtCalibDataContainer> writeHandle{m_writeKey, ctx};
    if(writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
    RegionGranularity gran{RegionGranularity::OnePerMultiLayer};
    auto writeCdo = std::make_unique<MuonCalib::MdtCalibDataContainer>(m_idHelperSvc.get(), gran);

    ATH_CHECK(loadRt(*writeCdo));
    ATH_CHECK(loadTube(*writeCdo));

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    

    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::loadRt(MuonCalib::MdtCalibDataContainer& writeHandle) const {
    ATH_MSG_DEBUG("loadRt " << name());

    MuonCalib::CalibFunc::ParVec rtPars{-10000, 10000, 1.};
    MuonCalib::CalibFunc::ParVec resoPars{-10000, 10000.};

   
    //Loop over RT regions and store the RT in each
    auto rtRelRegion{std::make_unique<MuonCalib::RootRtRelation>(rtPars)};
    auto rtResRegion{std::make_unique<MuonCalibR4::RtResolutionSqrt>(resoPars)};
    auto trRelRegion{std::make_unique<MuonCalibR4::TrSqrt>(*rtRelRegion)};
    RtRelationPtr MdtRt = std::make_unique<MuonCalib::MdtRtRelation>(std::move(rtRelRegion), std::move(rtResRegion), std::move(trRelRegion), 0.);
    std::vector<const MuonGMR4::MdtReadoutElement*> detEls = m_r4detMgr->getAllMdtReadoutElements();
    for(const MuonGMR4::MuonReadoutElement* mdtDetEl : detEls) {
        const Identifier& detElId = mdtDetEl->identify();
        ATH_MSG_DEBUG("Adding Rt relation for " << m_idHelperSvc->toString(detElId));
        if(writeHandle.hasDataForChannel(detElId, msgStream())) {
            const MuonCalib::MdtFullCalibData* dataObj = writeHandle.getCalibData(detElId, msgStream());
            if(dataObj->rtRelation) {
                ATH_MSG_DEBUG("Rt relation constants for " << m_idHelperSvc->toString(detElId) << " already exist");
                continue;
            }
        }
        RtRelationPtr storeMe = MdtRt;
        if(!writeHandle.storeData(detElId, storeMe, msgStream())) {
            ATH_MSG_ERROR("Failed to store Rt for " << m_idHelperSvc->toString(detElId));
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::loadTube(MuonCalib::MdtCalibDataContainer& writeCdo) const {
    ATH_MSG_DEBUG("loadTube " << name());
    const double inversePropSpeed = 1. / (Gaudi::Units::c_light * m_prop_beta);
    std::vector<const MuonGMR4::MdtReadoutElement*> detEls = m_r4detMgr->getAllMdtReadoutElements();
    for(const MuonGMR4::MdtReadoutElement* mdtDetEl : detEls) {
        const Identifier& chamberId = mdtDetEl->identify();
        if(writeCdo.hasDataForChannel(chamberId, msgStream())) {
            const MuonCalib::MdtFullCalibData* dataObj = writeCdo.getCalibData(chamberId, msgStream());
            if(dataObj->tubeCalib) {
                ATH_MSG_DEBUG("Tube calibration constants for " << m_idHelperSvc->toString(chamberId) << " already exist");
                continue;
            }
        }
        TubeContainerPtr tubes = std::make_unique<MuonCalib::MdtTubeCalibContainer>(m_idHelperSvc.get(), chamberId);
        if (!writeCdo.storeData(chamberId, tubes, msgStream())) {
            ATH_MSG_FATAL("Failed to store chamber data");
            return StatusCode::FAILURE;
        }
        for(size_t layer = 1; layer <= mdtDetEl->numLayers(); ++layer){
            for(size_t tube = 1; tube <= mdtDetEl->numTubesInLay(); ++tube){
                MuonCalib::MdtTubeCalibContainer::SingleTubeCalib tubeCalib;
                const Identifier tubeId{mdtDetEl->measurementId(mdtDetEl->measurementHash(layer, tube))};
                tubeCalib.t0 = m_defaultT0;
                tubeCalib.adcCal = 1.;
                tubeCalib.inversePropSpeed = inversePropSpeed;
                if (!tubes->setCalib(std::move(tubeCalib), tubeId, msgStream())) {
                    ATH_MSG_FATAL("Failed to store tube constants...");                    
                    return StatusCode::FAILURE;

                }
            }
        }
    }
    return StatusCode::SUCCESS;
}

