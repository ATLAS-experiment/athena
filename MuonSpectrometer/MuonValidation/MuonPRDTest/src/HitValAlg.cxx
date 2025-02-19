/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// NSWValAlg inlcudes
#include "HitValAlg.h"

#include <MuonPRDTest/MuonPRDTestDict.h>


// Other NSW includes
#include "MuonReadoutGeometry/MuonDetectorManager.h"



#include <mutex>

using namespace MuonPRDTest;
namespace MuonVal{
StatusCode HitValAlg::initialize() {
    ATH_MSG_DEBUG("initialize()");
    unsigned int ev_infomask{EventInfoBranch::writePileUp};
    if (!m_isData) ev_infomask |= EventInfoBranch::isMC | EventInfoBranch::writeBeamSpot;    
    m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree,ev_infomask));

    ATH_CHECK(m_idHelperSvc.retrieve());
  
    if (m_doTruth) { m_tree.addBranch(std::make_unique<TruthVariables>(m_tree, m_Truth_ContainerName, msgLevel())); }

    if (m_doMuEntry) {
        m_tree.addBranch(std::make_unique<MuEntryVariables>(m_tree, m_MuEntry_ContainerName, msgLevel()));
    }

    if (m_doSTGCHit) { m_tree.addBranch(std::make_unique<sTGCSimHitVariables>(m_tree, m_NSWsTGC_ContainerName.value(), msgLevel())); }
    if (m_doSTGCDigit) {
        m_tree.addBranch(std::make_unique<sTgcDigitVariables>(m_tree, m_NSWsTGC_DigitContainerName.value(), msgLevel()));
    }
    if (m_doSTGCSDO) {
        m_tree.addBranch(std::make_unique<sTgcSDOVariables>(m_tree, m_NSWsTGC_SDOContainerName.value(), msgLevel()));
    }
    if (m_doSTGCFastDigit) { m_tree.addBranch(std::make_unique<sTgcSDOVariables>(m_tree, "sTGCfast_SDO", msgLevel())); }
    if (m_doSTGCRDO) { m_tree.addBranch(std::make_unique<sTGCRDOVariables>(m_tree, m_NSWsTGC_RDOContainerName.value(), msgLevel())); }
    if (m_doSTGCPRD) { m_tree.addBranch(std::make_unique<sTGCPRDVariables>(m_tree, m_NSWsTGC_PRDContainerName.value(), msgLevel())); }

    if (m_doMMHit) { m_tree.addBranch(std::make_unique<MMSimHitVariables>(m_tree, m_NSWMM_ContainerName.value(), msgLevel())); }
    if (m_doMMDigit) {
        m_tree.addBranch(std::make_unique<MMDigitVariables>(m_tree, m_NSWMM_DigitContainerName.value(), msgLevel()));
    }
    if (m_doMMSDO) {
        m_tree.addBranch(std::make_unique<MMSDOVariables>(m_tree, m_NSWMM_SDOContainerName.value(), msgLevel()));
    }
    if (m_doMMFastDigit) { m_tree.addBranch(std::make_unique<MMSDOVariables>(m_tree, "MMfast_SDO", msgLevel())); }
    if (m_doMMRDO) { m_tree.addBranch(std::make_unique<MMRDOVariables>(m_tree, m_NSWMM_RDOContainerName.value(), msgLevel())); }
    if (m_doMMPRD ) { m_tree.addBranch(std::make_unique<MMPRDVariables>(m_tree, m_NSWMM_PRDContainerName.value(), msgLevel())); }

    if (m_doCSCHit) { m_tree.addBranch(std::make_unique<CSCSimHitVariables>(m_tree, m_CSC_SimContainerName.value(), msgLevel())); }
    if (m_doCSCSDO) { m_tree.addBranch(std::make_unique<CscSDOVariables>(m_tree, m_CSC_SDOContainerName.value(), msgLevel())); }
    if (m_doCSCDigit) { m_tree.addBranch(std::make_unique<CscDigitVariables>(m_tree, m_CSC_DigitContainerName.value(), msgLevel())); }
    if (m_doCSCRDO) { 
        ATH_CHECK(m_csc_decoder.retrieve());
        m_tree.addBranch(std::make_unique<CSCRDOVariables>(m_tree, m_CSC_RDOContainerName.value(), msgLevel(), &m_idHelperSvc->cscIdHelper(), m_csc_decoder.get())); 
    }
    if (m_doCSCPRD) { m_tree.addBranch(std::make_unique<CSCPRDVariables>(m_tree, m_CSC_PRDContainerName.value(), msgLevel())); }

    if (m_doMDTHit) { m_tree.addBranch(std::make_unique<MDTSimHitVariables>(m_tree, m_MDT_SimContainerName.value(), msgLevel())); }
    if (m_doMDTSDO) { m_tree.addBranch(std::make_unique<MdtSDOVariables>(m_tree, m_MDT_SDOContainerName.value(), msgLevel())); }
    if (m_doMDTDigit) { m_tree.addBranch(std::make_unique<MdtDigitVariables>(m_tree, m_MDT_DigitContainerName.value(), msgLevel())); }

    if (m_doRPCHit) { m_tree.addBranch(std::make_unique<RPCSimHitVariables>(m_tree, m_RPC_SimContainerName, msgLevel())); }
    if (m_doRPCSDO) { m_tree.addBranch(std::make_unique<RpcSDOVariables>(m_tree, m_RPC_SDOContainerName, msgLevel())); }
    if (m_doRPCDigit) { m_tree.addBranch(std::make_unique<RpcDigitVariables>(m_tree, m_RPC_DigitContainerName, msgLevel())); }

    if (m_doTGCHit) { m_tree.addBranch(std::make_unique<TGCSimHitVariables>(m_tree, m_TGC_SimContainerName.value(), msgLevel())); }
    if (m_doTGCSDO) { m_tree.addBranch(std::make_unique<TgcSDOVariables>(m_tree, m_TGC_SDOContainerName.value(), msgLevel())); }
    if (m_doTGCDigit) { m_tree.addBranch(std::make_unique<TgcDigitVariables>(m_tree, m_TGC_DigitContainerName.value(), msgLevel())); }
    if (m_doTGCRDO) {
	ATH_CHECK(m_tgcCabling.retrieve());
        m_tree.addBranch(std::make_unique<TGCRDOVariables>(m_tree, m_TGC_RDOContainerName.value(), msgLevel(), m_tgcCabling));
    }
    if (m_doTGCPRD) { m_tree.addBranch(std::make_unique<TGCPRDVariables>(m_tree, m_TGC_PRDContainerName.value(), msgLevel())); }

    ATH_MSG_DEBUG("Init TTree");
    ATH_CHECK(m_tree.init(this));

    ATH_MSG_DEBUG("Finished with the initialization");
    return StatusCode::SUCCESS;
}

StatusCode HitValAlg::finalize() {
    ATH_MSG_DEBUG("PrdValAlg:: Finalize + Matching");
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}

StatusCode HitValAlg::execute() {
    ATH_MSG_DEBUG("execute()");
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATH_MSG_DEBUG("Fill TTree");
    if (!m_tree.fill(ctx)) return StatusCode::FAILURE;

    return StatusCode::SUCCESS;
}
}