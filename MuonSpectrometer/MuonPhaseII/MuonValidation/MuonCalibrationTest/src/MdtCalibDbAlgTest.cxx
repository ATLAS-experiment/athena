/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibDbAlgTest.h"

#include "GaudiKernel/PhysicalConstants.h"
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include "GaudiKernel/ITHistSvc.h"
#include "TH2D.h"
#include "TCanvas.h"

using namespace MuonValR4;

MdtCalibDbAlgTest::MdtCalibDbAlgTest(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthHistogramAlgorithm(name, pSvcLocator) {}

StatusCode MdtCalibDbAlgTest::initialize() {
    ATH_MSG_ALWAYS("Initializing MdtCalibDbAlgTest");
    ATH_CHECK(m_MdtKey.initialize());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_calibrationTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tree.init(this));
    ATH_CHECK(book(TH2D("DriftRadiusVsTdc", "DriftRadiusVsTdc", 100, 0., 250., 100, 0., 15.), "MdtCalibDbAlgTest", "MdtCalibDbAlgTest"));
    ATH_CHECK(book(TH2D("DriftdRdtVsTdc", "DriftdRdtVsTdc", 100, 0., 250., 100, 0., 0.6), "MdtCalibDbAlgTest", "MdtCalibDbAlgTest"));
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlgTest::finalize() {
    ATH_MSG_ALWAYS("Finalizing MdtCalibDbAlgTest");
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlgTest::execute() {
    // ATH_MSG_ALWAYS("Executing MdtCalibDbAlgTest");
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::MdtDriftCircleContainer> mdtHandle{m_MdtKey};
    SG::ReadHandle<ActsGeometryContext> geoCtxHandle{m_geoCtxKey};
    ATH_CHECK(mdtHandle.isPresent());
    ATH_CHECK(geoCtxHandle.isPresent());
    const xAOD::MdtDriftCircleContainer* mdtContainer = mdtHandle.get();
    const ActsGeometryContext* geoCtx = geoCtxHandle.get();
    constexpr double inversePropSpeed = 1. / Gaudi::Units::c_light;
    for(const xAOD::MdtDriftCircle* mdt : *mdtContainer) {
        const MuonGMR4::MdtReadoutElement* mdtRE = mdt->readoutElement();
        const Amg::Vector3D& mdtGlobalTubePos = mdtRE->globalTubePos(*geoCtx, mdt->measurementHash());
        const float tdcAdj = IMdtCalibrationTool::tdcBinSize * (mdt->tdc() - inversePropSpeed * (mdtGlobalTubePos.norm() - 0.5 * mdtRE->activeTubeLength(mdt->measurementHash())));
        m_out_tdcAdj = tdcAdj;
        m_out_tdc = mdt->tdc();
        m_out_driftRadius = mdt->driftRadius();
        m_out_driftdRdt = m_calibrationTool->getdRdtFromRt(ctx, mdtRE->identify(), tdcAdj);
        m_out_identifier = mdt->identify();
        m_out_globalPos = mdtGlobalTubePos.norm();
        m_out_globalPosX = mdtGlobalTubePos.x();
        m_out_globalPosY = mdtGlobalTubePos.y();
        m_out_globalPosZ = mdtGlobalTubePos.z();
        m_out_tubeLength = mdtRE->activeTubeLength(mdt->measurementHash());
        hist("DriftRadiusVsTdc")->Fill(tdcAdj, mdt->driftRadius());
        hist("DriftdRdtVsTdc")->Fill(tdcAdj, m_calibrationTool->getdRdtFromRt(ctx, mdtRE->identify(), tdcAdj));
        m_tree.fill(ctx);
    }

    return StatusCode::SUCCESS;
}