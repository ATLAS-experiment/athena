/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSCvsRawChannelMonAlg.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "LArIdentifier/LArOnlineID.h"

StatusCode LArSCvsRawChannelMonAlg::initialize() {

  ATH_CHECK(m_SCKey.initialize());
  ATH_CHECK(m_RCKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_cablingSCKey.initialize());
  ATH_CHECK(m_badChanKey.initialize());
  ATH_CHECK(m_badSCKey.initialize());
  ATH_CHECK(m_caloSuperCellMgrKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));
  ATH_CHECK(m_scidtool.retrieve());
  ATH_CHECK(m_bcMask.buildBitMask(m_problemsToMask, msg()));
  ATH_CHECK(m_caloBCIDAvg.initialize(SG::AllowEmpty));
  return AthMonitorAlgorithm::initialize();
}

StatusCode LArSCvsRawChannelMonAlg::fillHistograms(const EventContext& ctx) const {

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey, ctx);
  if (!cablingHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map !!!!");
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> cablingSCHdl(m_cablingSCKey, ctx);
  if (!cablingSCHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map for SuperCells !!!!");
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<CaloSuperCellDetDescrManager> scDetMgr(m_caloSuperCellMgrKey, ctx);
  if (!scDetMgr.isValid()) {
    ATH_MSG_ERROR("Do not have CaloSuperCellDetDescrManager !!!!");
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArBadChannelCont> bcHdl(m_badChanKey, ctx);
  if (!bcHdl.isValid()) {
    ATH_MSG_ERROR("Do not have BadChannelContainer !!!!");
    return StatusCode::FAILURE;
  }
  const LArBadChannelCont* bcCont = *bcHdl;

  SG::ReadCondHandle<LArBadChannelCont> bcSCHdl(m_badSCKey, ctx);
  if (!bcSCHdl.isValid()) {
    ATH_MSG_ERROR("Do not have BadSCContainer !!!!");
    return StatusCode::FAILURE;
  }
  const LArBadChannelCont* bcSCCont = *bcSCHdl;

  // get SuperCellContainer
  SG::ReadHandle<LArRawSCContainer> scHdl(m_SCKey, ctx);
  if (!scHdl.isValid()) {
    ATH_MSG_WARNING("Do not have LArRawSCContainer container with key" << m_SCKey.key());
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG("Reading SuperCell container with key " << m_SCKey.key() << ", size=" << scHdl->size());
  }

  // get regular LArRawChannel container
  SG::ReadHandle<LArRawChannelContainer> rcHdl(m_RCKey, ctx);
  if (!scHdl.isValid()) {
    ATH_MSG_WARNING("Do not have LArRawChannel container with key" << m_RCKey.key());
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG("Reading LArRawChannel container with key " << m_RCKey.key() << ", size=" << rcHdl->size());
  }

  const CaloBCIDAverage* bcidavgshift = nullptr;
  if (!(m_caloBCIDAvg.key().empty())) {
    SG::ReadHandle<CaloBCIDAverage> bcidavgshiftHdl(m_caloBCIDAvg, ctx);
    bcidavgshift = bcidavgshiftHdl.cptr();
  }

  const unsigned int bcid = ctx.eventID().bunch_crossing_id();

  std::vector<std::pair<Monitored::Scalar<float>, Monitored::Scalar<float> > > monVars;
  for (int p = 0; p < MAXPARTITIONS; ++p) {
    monVars.emplace_back(Monitored::Scalar<float>("SCEne_" + m_partitionNames.value()[p], 0.0),
                         Monitored::Scalar<float>("eneSum_" + m_partitionNames.value()[p], 0.0));
  }

  for (const LArRawSC* rawSC : *scHdl) {
    const std::vector<unsigned short>& bcids = rawSC->bcids();
    const std::vector<int>& energies = rawSC->energies();
    const std::vector<bool>& satur = rawSC->satur();

    // Look for bcid:
    float scEne = 0;

    const size_t nBCIDs = bcids.size();
    size_t i = 0;
    for (i = 0; i < nBCIDs && bcids[i] != bcid; i++)
      ;
    if (satur[i])
      continue;

    if (!bcSCCont->status(rawSC->hardwareID()).good())
      continue;

    scEne = energies[i];
    if (scEne < m_scEneCut)
      continue;
    Identifier off_id = cablingSCHdl->cnvToIdentifier(rawSC->hardwareID());

    const int iPart = getPartition(off_id);
    if (iPart < 0) {
      ATH_MSG_ERROR("Got unkonwn partition number " << iPart);
      return StatusCode::FAILURE;
    }

    const std::vector<Identifier>& regularIDs = m_scidtool->superCellToOfflineID(off_id);
    std::set<HWIdentifier> hwids;
    for (const Identifier& id : regularIDs) {
      hwids.insert(cablingHdl->createSignalChannelID(id));
    }
    // Loop over regular RawChannelContainer to find the relevant cells:
    float eneSum = 0;
    const size_t nChans = rcHdl->size();
    bool hasBadChan = false;
    for (size_t i = 0; i < nChans && !hwids.empty(); ++i) {
      const LArRawChannel& rc = rcHdl->at(i);
      if (hwids.contains(rc.hardwareID())) {
        if (m_bcMask.cellShouldBeMasked(bcCont, rc.hardwareID())) {
          hasBadChan = true;
          break;
        }

        bcHdl->status(rc.hardwareID()).deadReadout();
        eneSum += rc.energy();
        if (bcidavgshift)
          eneSum -= bcidavgshift->average(rc.hardwareID());
        hwids.erase(rc.hardwareID());
      }
    }  // end loop over regular raw channels

    if (hasBadChan) {
      ATH_MSG_DEBUG("SuperCell with id 0x" << std::hex << off_id.get_identifier32().get_compact() << std::dec << " E[" << i << "]=" << scEne
                                           << " is connected to at least one bad channel. Ignoring SC.");
      continue;
    }
    if (!hwids.empty()) {
      ATH_MSG_ERROR("SuperCell with id 0x" << std::hex << off_id.get_identifier32().get_compact() << std::dec << ": " << hwids.size()
                                           << " attached regular RawChannels not found");
      continue;  // Ignore this supercell
    }
    const CaloDetDescrElement* scDDE = scDetMgr->get_element(off_id);
    scEne *= 12.5 / scDDE->sinTh();
    ATH_MSG_VERBOSE("SuperCell with id 0x" << std::hex << off_id.get_identifier32().get_compact() << std::dec << ", eta=" << scDDE->eta() << " E[" << i
                                           << "]=" << scEne << " Sum of RC energies=" << eneSum << ", Ratio=" << (eneSum != 0 ? scEne / eneSum : 0.0));

    if (m_warnOffenders && scEne > 15000 && eneSum < 5000) {
      ATH_MSG_WARNING("SuperCell with id 0x" << std::hex << off_id.get_identifier32().get_compact() << std::dec << ", eta=" << scDDE->eta() << " E[" << i
                                             << "]=" << scEne << " Sum of RC energies=" << eneSum);
    }

    auto& monPair = monVars[iPart];
    monPair.first = scEne;
    monPair.second = eneSum;

    fill(m_MonGroupName, monPair.first, monPair.second);
  }

  return StatusCode::SUCCESS;
}

int LArSCvsRawChannelMonAlg::getPartition(const Identifier& id) const {
  const int s = m_calo_id->calo_sample(id);

  // PreSamplerB=0, EMB1, EMB2, EMB3, PreSamplerE, EME1, EME2, EME3, HEC0, HEC1,
  // HEC2, HEC3, TileBar0, TileBar1, TileBar2,TileGap1, TileGap2, TileGap3,
  // TileExt0, TileExt1, TileExt2, FCAL0, FCAL1, FCAL2, MINIFCAL0, MINIFCAL1,
  // MINIFCAL2, MINIFCAL3,
  constexpr std::array<int, CaloSampling::getNumberOfSamplings()> samplingToPartitonMap{
      0,  0,  0,  0,  // Barrel BS to EMB3
      2,  2,  2,  2,  // EMEC PS to EME3
      4,  4,  4,  4,  // HEC0 to HEC3
      -1, -1, -1,     // TileBar0 - 2 (ignored)
      -1, -1, -1,     // Tile Gap 0 -2 (ignored)
      -1, -1, -1,     // TIleExt0 0-2 (ignored)
      6,  6,  6,      // FCAL 0-2
      -1, -1, -1, -1  // MiniFCAL0-3 (ignored)
  };

  const int pNoSide = samplingToPartitonMap[s];
  if (ATH_UNLIKELY(pNoSide < 0)) {
    return MAXPARTITIONS;
  }

  return (m_calo_id->pos_neg(id) < 0) ? pNoSide + 1 : pNoSide;
}