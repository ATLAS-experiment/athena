/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonHitTesterAlg.h"

#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonPRDTestR4/SimHitTester.h"

#include "MuonPRDTest/MDTDigitVariables.h"
#include "MuonPRDTest/RPCDigitVariables.h"
#include "MuonPRDTest/TGCDigitVariables.h"
#include "MuonPRDTest/MMDigitVariables.h"
#include "MuonPRDTest/sTGCDigitVariables.h"

#include "MuonPRDTestR4/MdtDriftCircleVariables.h"
#include "MuonPRDTestR4/MdtTwinDriftCircleVariables.h"

#include "MuonPRDTestR4/RpcMeasurementVariables.h"
#include "MuonPRDTestR4/TgcStripVariables.h"
#include "MuonPRDTestR4/MmClusterVariables.h"

using namespace MuonVal;
using namespace MuonPRDTest;
namespace MuonValR4 {
    MuonHitTesterAlg::MuonHitTesterAlg(const std::string& alg_name,
                                       ISvcLocator* pSvcLocator):
        AthHistogramAlgorithm{alg_name, pSvcLocator}{} 
            
    StatusCode MuonHitTesterAlg::initialize(){
        int evOpts{0};
        if (m_isMC) evOpts |= EventInfoBranch::isMC;
        
        m_tree.addBranch(std::make_shared<EventInfoBranch>(m_tree, evOpts));
        ATH_CHECK(setupSimHits());
        ATH_CHECK(setupDigits());
        ATH_CHECK(setupPrds());
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
        StatusCode MuonHitTesterAlg::setupSimHits() {
        if (!m_writeSimHits || !m_isMC){
            return StatusCode::SUCCESS;
        }
        if (m_writeMdtSim) {
            m_tree.addBranch(std::make_shared<SimHitTester>(m_tree, m_mdtSimHitKey, 
                                                            ActsTrk::DetectorType::Mdt,
                                                            msgLevel()));
        }
        if (m_writeRpcSim) {
            m_tree.addBranch(std::make_shared<SimHitTester>(m_tree, m_rpcSimHitKey, 
                                                            ActsTrk::DetectorType::Rpc,
                                                            msgLevel()));
        }
        if (m_writeTgcSim) {
            m_tree.addBranch(std::make_shared<SimHitTester>(m_tree, 
                                                            m_tgcSimHitKey, 
                                                            ActsTrk::DetectorType::Tgc,                                  
                                                            msgLevel()));
        }
        if (m_writesTgcSim) {
            m_tree.addBranch(std::make_shared<SimHitTester>(m_tree, m_sTgcSimHitKey, 
                                                            ActsTrk::DetectorType::sTgc,
                                                            msgLevel()));
        }
        if (m_writeMmSim) {
            m_tree.addBranch(std::make_shared<SimHitTester>(m_tree, m_mmSimHitKey, 
                                                            ActsTrk::DetectorType::Mm, 
                                                            msgLevel()));
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHitTesterAlg::setupDigits() {
        if (!m_writeDigits) {
            return StatusCode::SUCCESS;
        }
        if (m_writeMdtDigits) {
            m_tree.addBranch(std::make_shared<MdtDigitVariables>(m_tree, m_mdtDigitKey, msgLevel()));
        }
        if (m_writeRpcDigits) {
            m_tree.addBranch(std::make_shared<RpcDigitVariables>(m_tree, m_rpcDigitKey, msgLevel()));
        }
        if (m_writeTgcDigits) {
            m_tree.addBranch(std::make_shared<TgcDigitVariables>(m_tree, m_tgcDigitKey, msgLevel()));
        }
        if (m_writeMmDigits) {
            m_tree.addBranch(std::make_shared<MMDigitVariables>(m_tree, m_mmDigitKey, msgLevel()));
        }
        if (m_writesTgcDigits) {
            m_tree.addBranch(std::make_shared<sTgcDigitVariables>(m_tree, m_sTgcDigitKey, msgLevel()));
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHitTesterAlg::setupPrds() {
        if (!m_writePrds) {
            return StatusCode::SUCCESS;
        }
        if (m_writeMdtPrds) {
            m_tree.addBranch(std::make_shared<MdtDriftCircleVariables>(m_tree, m_mdtPrdKey, msgLevel()));
            m_tree.addBranch(std::make_shared<MdtTwinDriftCircleVariables>(m_tree, m_mdtTwinPrdKey, msgLevel()));
        }
        if (m_writeRpcPrds) {
            m_tree.addBranch(std::make_shared<RpcMeasurementVariables>(m_tree, m_rpcPrdKey, msgLevel()));
        }
        if (m_writeTgcPrds) {
            m_tree.addBranch(std::make_shared<TgcStripVariables>(m_tree, m_tgcPrdKey, msgLevel()));
        }
        if (m_writeMmPrds) {
            m_tree.addBranch(std::make_shared<MmClusterVariables>(m_tree, m_mmPrdKey, msgLevel()));
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHitTesterAlg::finalize(){
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHitTesterAlg::execute(){
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }
}
