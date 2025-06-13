/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RPCSimulation.h"

#include "xAODTrigger/MuonRoIAuxContainer.h"
#include "TruthUtils/HepMCHelpers.h"

namespace L0Muon
{

  StatusCode RPCSimulation::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_keyRpcRdo.initialize());
    ATH_CHECK(m_mcEventCollectionKey.initialize());
    ATH_CHECK(m_simHitContainerKey.initialize());
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
    SG::WriteHandle<L0Muon::BarrelCandDataContainer> outputCands(m_outputCandKey, ctx);
    ATH_CHECK(outputCands.record(std::make_unique<L0Muon::BarrelCandDataContainer>()));

    if (m_useTruth)
    {
      ATH_CHECK(buildFromTruth(outputCands, ctx));
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

  StatusCode RPCSimulation::buildFromTruth(SG::WriteHandle<L0Muon::BarrelCandDataContainer>
                                               outputCands,
                                           const EventContext &ctx) const
  {
    const ActsGeometryContext* geoContextHandle{nullptr};
    ATH_CHECK(SG::get(geoContextHandle, m_geoCtxKey, ctx));
    const ActsGeometryContext& gctx{*geoContextHandle};
    /// retrieve the truth hits
    const xAOD::MuonSimHitContainer *simHitContainer = nullptr;
    SG::ReadHandle<xAOD::MuonSimHitContainer> simHitContainerHandle{m_simHitContainerKey, ctx};
    if (!simHitContainerHandle.isValid())
    {
      ATH_MSG_FATAL("MuonSimHitContainer not found: " << m_simHitContainerKey.key());
      return StatusCode::FAILURE;
    }
    simHitContainer = simHitContainerHandle.cptr();
    const RpcIdHelper& id_helper{m_idHelperSvc->rpcIdHelper()};
    /// retrieve the truth particles
    const McEventCollection *mcCollptr = nullptr;
    SG::ReadHandle<McEventCollection> mcEventCollectionHandle{m_mcEventCollectionKey, ctx};
    if (!mcEventCollectionHandle.isValid())
    {
      ATH_MSG_FATAL(" McEventCollection not found: " << m_mcEventCollectionKey.key());
      return StatusCode::FAILURE;
    }
    mcCollptr = mcEventCollectionHandle.cptr();
    
    /// create the trigger candidates from the MC truth
    for (unsigned int cntr = 0; cntr < mcCollptr->size(); ++cntr)
    {
      const HepMC::GenEvent *genEvt = (mcCollptr->at(cntr));

      for (const auto &p : *genEvt)
      {
        if (MC::isMuon(p))
        {
          // Check if the particle is in the barrel region
          if (fabs(p->momentum().eta()) > 1.05)
            continue;

          ATH_MSG_DEBUG("Found a muon with pdgId: " << p->pdg_id());

          // Create a new candidate
          float eta = p->momentum().eta();
          float phi = p->momentum().phi();
          float pt = p->momentum().perp();
          /// Set the charge bit to zero for negative muons, 1 for positive muons
          uint8_t charge = p->pdg_id() < 0 ? 0 : 1;

          /// create the candidate
          /// do not set the sectorId and bcTag for the moment
          uint16_t subdetectorId = eta > 0 ? 0x65 : 0x66;
          auto cand = std::make_unique<L0Muon::BarrelCandData>(subdetectorId,
                                                               0, 0);

          cand->setEta(eta);
          cand->setPhi(phi);
          cand->setPt(pt);
          cand->setThreshold(0);
          cand->setCharge(charge);
          cand->setMdtFlag(0);

          float zPos[4] = {0.0, 0.0, 0.0, 0.0};
          int nZPos[4]  = {0, 0, 0, 0};
          /// loop on the truth hits and look for those associated with the muon
          for (auto hit : *simHitContainer)
          {

            if (hit->genParticleLink().isValid() &&
                hit->genParticleLink().barcode() == HepMC::barcode(*p))
            {
              /// using for the moment the local position of the hit
              
              // get the hit identifier and the strip position 
              const Identifier id = hit->identify();
              
              const MuonGMR4::RpcReadoutElement* detEl = m_detMgr->getRpcReadoutElement(id);
              if (!detEl) {
                ATH_MSG_WARNING("Could not find RPC readout element for identifier: " << id);
                continue;
              }
              float hitPosZ = detEl->stripPosition(gctx,id).z();
              std::string station = id_helper.stationNameString(id_helper.stationName(id));
              if (station.substr(0,2)=="BI") {
                zPos[0] += hitPosZ;
                nZPos[0]++;
              }
              else if (station.substr(0,2)=="BM") {
                if ( id_helper.doubletR(id)==1) {
                  zPos[1] += hitPosZ;
                  nZPos[1]++;
                }
                else if ( id_helper.doubletR(id)==2) {
                  zPos[2] += hitPosZ;
                  nZPos[2]++;
                }
              }
              else if (station.substr(0,2)=="BO") {
                zPos[3] += hitPosZ;
                nZPos[3]++;
              }
              else {
                ATH_MSG_WARNING("Unknown RPC station: " << station);
                continue;
              }
              
            }
          }
          // Set the Z positions in the candidate
          for (int i = 0; i < 4; ++i)
          {
            if (nZPos[i] > 0)
            {
              zPos[i] = fabs(zPos[i]); // Use absolute value for Z position
              zPos[i] /= nZPos[i];
              // Normalize the Z position to the range of 12 bits
              // The range is from 0 to +12500, so we map it to 0-4095
              cand->setZPos(static_cast<uint16_t>(zPos[i]/
                L0Muon::BarrelCandData::s_zPosRange*L0Muon::BarrelCandData::s_zPosBitRange), i);
            }
            else
            {
              cand->setZPos(0, i); // No hit found, set to zero
            }
          }

          cand->setQuality(L0Muon::BarrelCandData::Quality::Q_BEST);
          outputCands->push_back(std::move(cand));
        }
      }
    }

    // Implementation of building candidates from truth
    return StatusCode::SUCCESS;
  }


} // end of namespace