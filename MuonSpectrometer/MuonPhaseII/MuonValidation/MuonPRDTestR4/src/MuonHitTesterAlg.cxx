/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonHitTesterAlg.h"

#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonPRDTestR4/SimHitTester.h"

#include "MuonPRDTest/MDTDigitVariables.h"
#include "MuonPRDTest/RPCDigitVariables.h"
#include "MuonPRDTest/TGCDigitVariables.h"
#include "MuonPRDTest/MMDigitVariables.h"
#include "MuonPRDTest/sTGCDigitVariables.h"
#include "MuonPRDTest/ParticleVariables.h"
#include "MuonPRDTest/SegmentVariables.h"

#include "MuonPRDTestR4/MdtDriftCircleVariables.h"
#include "MuonPRDTestR4/MdtTwinDriftCircleVariables.h"

#include "MuonPRDTestR4/RpcMeasurementVariables.h"
#include "MuonPRDTestR4/TgcStripVariables.h"
#include "MuonPRDTestR4/MmClusterVariables.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
using namespace MuonVal;
using namespace MuonPRDTest;
namespace {
    using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
    using TruthLinkDecor_t = SG::ConstAccessor<TruthLink_t>;
}
namespace MuonValR4 {
    MuonHitTesterAlg::~MuonHitTesterAlg() = default;
    StatusCode MuonHitTesterAlg::initialize(){
        int evOpts{0};
        if (m_isMC) evOpts |= EventInfoBranch::isMC;
        ATH_CHECK(m_evtKey.initialize());
        m_tree.addBranch(std::make_shared<EventInfoBranch>(m_tree, evOpts,m_evtKey.key()));
        ATH_CHECK(setupTruth());
        ATH_CHECK(setupSimHits());
        ATH_CHECK(setupDigits());
        ATH_CHECK(setupPrds());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHitTesterAlg::setupTruth() {
        if (!m_isMC) {
            return StatusCode::SUCCESS;
        }
        if (m_writeTruthMuon) {
            m_truthParts = std::make_shared<ParticleVariables>(m_tree, m_truthMuonCont, "truthMuon", msgLevel());
            m_truthParts->addVariable<int>("truthOrigin");
            m_truthParts->addVariable<int>("truthType");
            m_tree.addBranch(m_truthParts);   
        }
        if (m_writeTruthSeg) {
            m_truthSegs = std::make_shared<SegmentVariables>(m_tree, m_truthSegCont, "truthSegment", msgLevel());
            if (m_writeTruthMuon) {
                m_truthSegs->addVariable(std::make_unique<GenericAuxEleBranch<xAOD::MuonSegment, unsigned short>>(
                                            m_tree, std::format("{}_truthLink", m_truthSegs->name()),
                                            [this](const xAOD::MuonSegment& seg) -> unsigned short{
                                                static const TruthLinkDecor_t acc{"truthParticleLink"};
                                                if (!acc(seg).isValid()){
                                                    return -1;
                                                }
                                                return m_truthParts->push_back(*acc(seg));
                                            }));
            }
            m_tree.addBranch(m_truthSegs);
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHitTesterAlg::setupSimHits() {
        if (!m_writeSimHits || !m_isMC){
            return StatusCode::SUCCESS;
        }
        auto addSimTester = [this](const std::string& key, const ActsTrk::DetectorType type){
            auto testerBr = std::make_shared<SimHitTester>(m_tree, key, type, msgLevel());
            if (m_truthSegs) {
                m_tree.addBranch(std::make_shared<MuonVal::GenericAuxEleBranch<xAOD::MuonSegment, std::vector<unsigned short>>>(m_tree,
                    std::format("{:}_matched{:}Hits", m_truthSegs->name(), ActsTrk::to_string(type)),
                    [this, testerBr, type](const xAOD::MuonSegment&seg) -> std::vector<unsigned short>{
                        const auto hitSet = MuonR4::getMatchingSimHits(seg);
                        std::vector<const xAOD::MuonSimHit*> sortedHits{};
                        std::ranges::copy_if(hitSet, std::back_inserter(sortedHits), [this, type] (const xAOD::MuonSimHit* hit){
                            return m_detMgr->getReadoutElement(hit->identify())->detectorType() == type;
                        });
                        std::ranges::sort(sortedHits, [](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                                return a->identify() < b->identify();
                        });
                        std::vector<unsigned short> matchIds{};
                        std::ranges::transform(sortedHits, std::back_inserter(matchIds), [testerBr](const xAOD::MuonSimHit* hit){
                            return testerBr->push_back(*hit);
                        });
                        return matchIds;
                    }));
            }
            m_tree.addBranch(std::move(testerBr));
        };
        if (m_writeMdtSim) {
            addSimTester(m_mdtSimHitKey, ActsTrk::DetectorType::Mdt);
        }
        if (m_writeRpcSim) {
            addSimTester(m_rpcSimHitKey, ActsTrk::DetectorType::Rpc);
        }
        if (m_writeTgcSim) {
            addSimTester(m_tgcSimHitKey, ActsTrk::DetectorType::Tgc);
        }
        if (m_writesTgcSim) {
            addSimTester(m_sTgcSimHitKey, ActsTrk::DetectorType::sTgc);
        }
        if (m_writeMmSim) {
            addSimTester(m_mmSimHitKey, ActsTrk::DetectorType::Mm);
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
