/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RPCSimulation.h"

#include "xAODTrigger/MuonRoIAuxContainer.h"
#include "TruthUtils/HepMCHelpers.h"


#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include <ranges>

namespace L0Muon
{

  StatusCode RPCSimulation::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_truthPartKey.initialize());
    ATH_CHECK(m_segmentLinkKey.initialize());

    ATH_CHECK(m_keyRpcRdo.initialize());
    ATH_CHECK(m_geoCtxKey.initialize()); 
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    ATH_CHECK(m_idHelperSvc.retrieve());
    /// container of output candidates
    ATH_CHECK(m_outputCandKey.initialize());

    /// retrieve the monitoring tool
    if (!m_monTool.empty())
      ATH_CHECK(m_monTool.retrieve());

    return StatusCode::SUCCESS;
  }

  StatusCode RPCSimulation::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    // output candidates container
    SG::WriteHandle outputCands(m_outputCandKey, ctx);
    ATH_CHECK(outputCands.record(std::make_unique<L0Muon::BarrelCandDataContainer>()));

    if (m_useTruth)
    {
      ATH_CHECK(buildFromTruth(*outputCands, ctx));
    }
    else if (m_usePatterns)
    {
      // ATH_CHECK(buildFromPatterns(outputCands, ctx));
    }
    else
    {
      ATH_MSG_ERROR("No valid option selected for building candidates.");
      return StatusCode::FAILURE;
    }

    SG::ReadHandle inputRDO(m_keyRpcRdo, ctx);
    ATH_CHECK(inputRDO.isPresent());
    ATH_MSG_DEBUG("Number of RPC RDO: " << inputRDO->size());

    /// monitor RDO quantities
    if (!m_monTool.empty())
    {
      auto n_of_RDO = Monitored::Scalar<unsigned int>("n_of_RDO", inputRDO->size());
    }

    return StatusCode::SUCCESS;
  }
  std::vector<const xAOD::MuonSimHit*> 
      RPCSimulation::collectHits(const xAOD::TruthParticle& truthPart,
                                 const EventContext& ctx) const {
      
      using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
      using SegLinkVec_t = std::vector<SegLink_t>;

      SG::ReadDecorHandle<xAOD::TruthParticleContainer, SegLinkVec_t> acc_link{m_segmentLinkKey,  ctx};
      std::vector<const xAOD::MuonSimHit*> rpcHits{};

      for (const SegLink_t& link : acc_link(truthPart)) {
          const xAOD::MuonSegment* segment{*link};
          if (!Muon::MuonStationIndex::isBarrel(segment->chamberIndex())){
              continue;
          }
          auto simHits = MuonR4::getMatchingSimHits(*segment);
          std::ranges::copy_if(simHits, std::back_inserter(rpcHits),
                               [this](const xAOD::MuonSimHit* hit){
                                  return m_idHelperSvc->isRpc(hit->identify());
                               });
      }
      std::ranges::sort(rpcHits, [](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){ 
                                      return a->identify() < b->identify(); 
                                 });
      return rpcHits;
  }

  StatusCode RPCSimulation::buildFromTruth(L0Muon::BarrelCandDataContainer& outputCands,
                                           const EventContext &ctx) const
  {
    const ActsGeometryContext* geoContextHandle{nullptr};
    ATH_CHECK(SG::get(geoContextHandle, m_geoCtxKey, ctx));
    const ActsGeometryContext& gctx{*geoContextHandle};
    /// retrieve the truth hits
    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
    /// retrieve the truth particles
    const xAOD::TruthParticleContainer *truthParticles = nullptr;
    ATH_CHECK(SG::get(truthParticles, m_truthPartKey, ctx));
    
    /// create the trigger candidates from the MC truth
    for (const xAOD::TruthParticle* truthMuon : *truthParticles) {
      std::vector<const xAOD::MuonSimHit*> rpcHits = collectHits(*truthMuon, ctx);
      if (rpcHits.empty()) {
        continue;
      }

      ATH_MSG_DEBUG("Found a muon with pdgId: " << truthMuon->pdgId());
      const auto muonP4 = truthMuon->p4();

      // Create a new candidate
      float eta = truthMuon->eta();
      float phi = truthMuon->phi();
      float pt = truthMuon->pt();
      /// Set the charge bit to zero for negative muons, 1 for positive muons
      uint8_t charge = truthMuon->charge() < 0 ? 0 : 1;

      /// create the candidate
      /// do not set the sectorId and bcTag for the moment
      uint16_t subdetectorId = eta > 0 ? 0x65 : 0x66;
      auto cand = std::make_unique<L0Muon::BarrelCandData>(subdetectorId, 0, 0);

      cand->setEta(eta);
      cand->setPhi(phi);
      cand->setPt(pt);
      cand->setThreshold(0);
      cand->setCharge(charge);
      cand->setMdtFlag(0);

      std::array<float, 4> zPos{};
      std::array<int, 4> nZPos{};
      /// loop on the truth hits and look for those associated with the muon
      for (const xAOD::MuonSimHit* hit : rpcHits) {
        /// using for the moment the local position of the hit
          
        // get the hit identifier and the strip position 
        const Identifier id = hit->identify();
              
        const MuonGMR4::RpcReadoutElement* detEl = m_detMgr->getRpcReadoutElement(id);
      
        float hitPosZ = detEl->stripPosition(gctx,id).z();
        switch (m_idHelperSvc->stationIndex(id)) {
           using enum Muon::MuonStationIndex::StIndex;
           case BI: {
              zPos[0] += hitPosZ;
              ++nZPos[0];
              break;
           } case BM: {
            const int dR = id_helper.doubletR(id);
            zPos[dR] += hitPosZ;
            ++nZPos[dR];
            break;
           } case BO: {
              zPos[3] += hitPosZ;
              ++nZPos[3];
              break;
           } default :{
              ATH_MSG_WARNING("Unknown RPC station: " << m_idHelperSvc->toString(hit->identify()));
              break;
           }
        }
      }
      // Set the Z positions in the candidate
      for (int i = 0; i < 4; ++i) {
          if (nZPos[i] > 0) {
            zPos[i] = std::abs(zPos[i]); // Use absolute value for Z position
            zPos[i] /= nZPos[i];
            // Normalize the Z position to the range of 12 bits
            // The range is from 0 to +12500, so we map it to 0-4095
            cand->setZPos(static_cast<uint16_t>(zPos[i]/
              L0Muon::BarrelCandData::s_zPosRange*L0Muon::BarrelCandData::s_zPosBitRange), i);
          }
      }
      cand->setQuality(L0Muon::BarrelCandData::Quality::Q_BEST);
      outputCands.push_back(std::move(cand));        
    }
    // Implementation of building candidates from truth
    return StatusCode::SUCCESS;
  }


} // end of namespace