/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSPCalibrationTest.h"
#include "GaudiKernel/PhysicalConstants.h"


using namespace MuonValR4;
using CalibSpacePointPtr = MuonR4::ISpacePointCalibrator::CalibSpacePointPtr;

StatusCode MuonSPCalibrationTest::initialize() {
    ATH_MSG_VERBOSE("Initializing MdtCalibDbAlgTest");
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_spKeyArray.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MuonSPCalibrationTest::execute() {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle geoCtx{m_geoCtxKey, ctx};
    ATH_CHECK(geoCtx.isPresent());

    for (const auto& spContainer : m_spKeyArray) {
        SG::ReadHandle<MuonR4::SpacePointContainer> spHandle{spContainer, ctx};
        ATH_CHECK(spHandle.isValid());

        for (const MuonR4::SpacePointBucket* spBucket : *spHandle) {
            if (!spBucket) continue;
            for(const auto& sp : *spBucket) {
                if (!sp) continue;
                ATH_MSG_ALWAYS("Processing SpacePoint " << m_idHelperSvc->toString(sp->identify()) << " with dimension " << sp->dimension() 
                                << " and position " << Amg::toString(sp->localPosition())
                                << " and direction " << Amg::toString(sp->sensorDirection()));

                // Get the seed position and direction in the chamber
                Amg::Vector3D seedPosInChamb = sp->localPosition();
                Amg::Vector3D seedDirInChamb = sp->sensorDirection();

                CalibSpacePointPtr calibSP =  m_calibTool->calibrate(ctx, sp.get(), seedPosInChamb, seedDirInChamb, 0.0);
                ATH_MSG_ALWAYS("Calibrated SpacePoint: with position " << Amg::toString(calibSP->localPosition())
                                << " and direction " << Amg::toString(calibSP->sensorDirection()));
            }

        }
    }
    return StatusCode::SUCCESS;
}