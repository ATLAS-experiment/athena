/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NSWSimulation.h"
#include "NSWEncoding.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace L1Muon {

  StatusCode NSWSimulation::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_keyMmDigit.initialize());
    ATH_CHECK(m_keySTgcDigit.initialize());

    // Container of output NSW trigger candidates
    ATH_CHECK(m_outputKey.initialize());

    // MM trigger tool
    ATH_CHECK(m_mmTrigger.retrieve(EnableTool{m_doMM}));

    // Monitoring tool
    if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

    return StatusCode::SUCCESS;
  }

  StatusCode NSWSimulation::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    const MmDigitContainer* mmDigits{nullptr};
    ATH_CHECK(SG::get(mmDigits, m_keyMmDigit, ctx));
    const sTgcDigitContainer* stgcDigits{nullptr};
    ATH_CHECK(SG::get(stgcDigits, m_keySTgcDigit, ctx));

    ATH_MSG_DEBUG("Number of MM Digits: " << mmDigits->size());
    ATH_MSG_DEBUG("Number of sTGC Digits: " << stgcDigits->size());
    ATH_MSG_DEBUG("Number of NSW Digits: " << (mmDigits->size() + stgcDigits->size()));

    // MM trigger
    std::vector<L1MM::MMCandidate> mmCandidates;
    if (m_doMM) {
      ATH_CHECK(m_mmTrigger->runTrigger(ctx, mmCandidates, nullptr));
    }
    ATH_MSG_DEBUG("Number of MM trigger candidates: " << mmCandidates.size());

    // Monitoring
    auto nMMDigits = Monitored::Scalar<unsigned int>("nMMDigits", mmDigits->size());
    auto nsTGCDigits = Monitored::Scalar<unsigned int>("nsTGCDigits", stgcDigits->size());
    auto nMMCandidates = Monitored::Scalar<unsigned int>("nMMCandidates", mmCandidates.size());
    Monitored::Group(m_monTool, nMMDigits, nsTGCDigits, nMMCandidates);

    // Group the candidates per (boardID, BC), keeping the order of the trigger tool.
    // No duplicate removal, no cap: every candidate becomes one segment (as in Run 3).
    std::map<std::pair<uint16_t, int>, std::vector<const L1MM::MMCandidate*>> frames;
    for (const L1MM::MMCandidate& cand : mmCandidates) {
      frames[{NSWEncoding::boardID(cand.sectorPhi, cand.side == L1MM::SideType::C), cand.BC}].push_back(&cand);
    }

    SG::WriteHandle<xAOD::L1NSWCandDataContainer> output(m_outputKey, ctx);

    auto container = std::make_unique<xAOD::L1NSWCandDataContainer>();
    auto auxContainer = std::make_unique<xAOD::L1NSWCandDataAuxContainer>();
    container->setStore(auxContainer.get());

    unsigned int nDthOutOfRange{0}, nEtaOutOfRange{0}, nOverflowFrames{0};
    for (const auto& [frameKey, cands] : frames) {
      const auto& [board, bc] = frameKey;
      const unsigned int nSeg = cands.size();
      const bool overflow = nSeg > m_maxSegmentsPerBoard;
      if (overflow) ++nOverflowFrames;

      for (const L1MM::MMCandidate* cand : cands) {
        bool etaOk{true}, dthOk{true};
        const uint16_t etaIdx = NSWEncoding::etaIndex(cand->eta, etaOk);
        const uint16_t phiIdx = NSWEncoding::phiIndex(cand->phiGlobal);
        const uint8_t dthIdx = NSWEncoding::dthetaIndex(cand->dtheta, m_dThetaStep, m_dThetaRange, dthOk);
        if (!etaOk) ++nEtaOutOfRange;
        if (!dthOk) ++nDthOutOfRange;

        xAOD::L1NSWCandData* seg = container->push_back(std::make_unique<xAOD::L1NSWCandData>());
        // Frame header fields (identical for all segments of this (board, BC))
        seg->setL1Bcid(static_cast<uint16_t>(bc) & 0xFFF);
        seg->setBoardID(board);
        seg->setFiberID(0);
        seg->setL1NSegments(static_cast<uint8_t>(std::min(nSeg, 255u)));
        seg->setL1Overflow(overflow);
        // The segment
        seg->setSegment(etaIdx, phiIdx, dthIdx, NSWEncoding::kQualityMM);
      }
    }

    ATH_MSG_DEBUG("NSW segments: " << container->size() << " written in " << frames.size() << " (board, BC) frames"
                  << ", frames with > " << m_maxSegmentsPerBoard.value() << " segments: " << nOverflowFrames
                  << ", |dtheta| out of range (saturated): " << nDthOutOfRange
                  << ", eta out of range: " << nEtaOutOfRange);

    ATH_CHECK(output.record(std::move(container), std::move(auxContainer)));

    ATH_MSG_DEBUG("Recorded NSW candidate container with " << output->size() << " candidate(s)");

    return StatusCode::SUCCESS;
  }

}   // end of namespace L1Muon
