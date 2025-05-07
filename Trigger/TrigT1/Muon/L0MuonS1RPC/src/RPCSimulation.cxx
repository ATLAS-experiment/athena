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
    ATH_CHECK(m_cablingKey.initialize());
    ATH_CHECK(m_mcEventCollectionKey.initialize());

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

    SG::ReadHandle inputRDO(m_keyRpcRdo, ctx);
    ATH_CHECK(inputRDO.isPresent());
    ATH_MSG_DEBUG("Number of RPC RDO: " << inputRDO->size());

    SG::ReadCondHandle cablingMap{m_cablingKey, ctx};
    ATH_CHECK(cablingMap.isValid());

    /// monitor RDO quantities
    if (!m_monTool.empty())
    {
      auto n_of_RDO = Monitored::Scalar<unsigned int>("n_of_RDO", inputRDO->size());
    }

    /// output candidates container
    SG::WriteHandle<L0Muon::BarrelCandDataContainer> outputCands(m_outputCandKey, ctx);
    ATH_CHECK(outputCands.record(std::make_unique<L0Muon::BarrelCandDataContainer>()));

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

      for (const auto& p : *genEvt)
      {
        if (MC::isMuon(p))
        {
          // Check if the particle is in the barrel region
          if (fabs(p->momentum().eta())>1.05)
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

          cand->setQuality(L0Muon::BarrelCandData::Quality::Q_BEST);
          outputCands->push_back(std::move(cand));
        }
      }

    }
    return StatusCode::SUCCESS;
  }
} // end of namespace