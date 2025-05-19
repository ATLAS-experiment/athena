/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCelldeadOTXTool.h"

#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArElecCalib/LArProvenance.h"
#include "LArIdentifier/LArOnlineID.h"

StatusCode LArCelldeadOTXTool::initialize() {

  ATH_CHECK(m_SCKey.initialize());
  ATH_CHECK(m_MFKey.initialize());
  ATH_CHECK(m_badSCKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_cablingSCKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));

  ATH_CHECK(m_scidtool.retrieve());

  if (m_testMode) {
    ATH_MSG_WARNING(
        "Test mode activated with additional debug output. Makes only sense if we try to patch a FEB that is actually there, so we have a reference");
  }

  return StatusCode::SUCCESS;
}

StatusCode LArCelldeadOTXTool::process(CaloCellContainer* cellCollection, const EventContext& ctx) const {

  ATH_MSG_VERBOSE(" in process...");
  if (!cellCollection) {
    ATH_MSG_ERROR("Cell Correction tool receives invalid cell Collection");
    return StatusCode::FAILURE;
  }

  if (!(cellCollection->hasCalo(CaloCell_ID::LAREM) ||
	cellCollection->hasCalo(CaloCell_ID::LARHEC) ||
	cellCollection->hasCalo(CaloCell_ID::LARFCAL))) {
    ATH_MSG_VERBOSE("No LAr cell in CellContainer. Do nothing");
    return StatusCode::SUCCESS;    
  }

  StatusCode sc = StatusCode::SUCCESS;
  std::call_once(m_onceFlag, &LArCelldeadOTXTool::buildMap, this, ctx, m_scToDead, sc);

  if (sc.isFailure()) {
    ATH_MSG_ERROR("Call to LArCelldeadOTXTool::buidMap returned an error");
    return StatusCode::FAILURE;
  }

  if (m_scToDead.empty()) {
    return StatusCode::SUCCESS;  // No dead FEBs, do nothing
  }
  
  // get SuperCellContainer
  SG::ReadHandle<LArRawSCContainer> scHdl(m_SCKey, ctx);
  if (!scHdl.isValid()) {
    if (msgLvl(MSG::WARNING) &&  m_nWarnings < 5) { 
      ATH_MSG_WARNING("Do not have SuperCell container no patching !!!!");
      ++m_nWarnings;
    }
    return StatusCode::SUCCESS;
  }

  const unsigned int bcid = ctx.eventID().bunch_crossing_id();

  // get the SC, container is unordered, so have to loop
  const LArRawSCContainer* scells = scHdl.cptr();
  for (const auto* sc : *scells) {
    if (!sc)
      continue;
    const HWIdentifier scHwid = sc->hardwareID();
    auto itr = m_scToDead.find(scHwid);
    if (itr == m_scToDead.end())
      continue;  // This SC is not connected to any deadFEB cell

    const std::vector<unsigned short>& bcids = sc->bcids();
    const std::vector<int>& energies = sc->energies();
    const std::vector<bool>& satur = sc->satur();

    // Look for bcid:
    float scEne = 0;
    const size_t nBCIDs = bcids.size();
    size_t i = 0;
    for (i = 0; i < nBCIDs && bcids[i] != bcid; i++)
      ;

    if (ATH_LIKELY(!satur[i]))
      scEne = energies[i];
    if (scEne < m_scCut) {
      ATH_MSG_VERBOSE("SC value " << scEne << " below threshold, ignoring");
      continue;
    }
    float cellESum = 0;
    float patchEneSum = 0;
    for (const auto& [h, convFactor] : itr->second) {  // Loop over all deadFEB cells connected to this SC
      CaloCell* cell = cellCollection->findCell(h);
      if (cell) {
        const float patchEne = scEne * convFactor;  // Convert ET (coming from LATOMEs) into Energy
        if (m_testMode) {
          cellESum += cell->energy();
          patchEneSum += patchEne;
        }
        ATH_MSG_DEBUG("Cell id 0x" << std::hex << cell->ID().get_identifier32().get_compact() << " Replacing energy " << cell->energy() << " " << patchEne
                                   << ", SCene=" << scEne);
        cell->setEnergy(patchEne);
        cell->setProvenance(cell->provenance() | LArProv::PATCHED);
      }  // end if cell obj found
    }  // end loop over all deadFEB cells connected to this SC
    if (m_testMode) {
      const float ratio = patchEneSum != 0 ? cellESum / patchEneSum : 0;
      ATH_MSG_DEBUG("ESums=" << cellESum << "/" << patchEneSum << "=" << ratio);
      std::scoped_lock l(m_mtx);
      auto& entry = m_testMap[scEne];
      entry.first += ratio;
      entry.second++;
    }  // end if testMode
  }  // End loop over SuperCell container

  return StatusCode::SUCCESS;
}

void LArCelldeadOTXTool::buildMap(const EventContext& ctx, scToDeadCellMap_t& scToHwidMap, StatusCode& sc) const {
  scToHwidMap.clear();  // Just to be sure ...

  sc = StatusCode::FAILURE;

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey, ctx);
  if (!cablingHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map !!!!");
    return;
  }
  const LArOnOffIdMapping* oflCabling = cablingHdl.cptr();

  SG::ReadCondHandle<LArOnOffIdMapping> cablingSCHdl(m_cablingSCKey, ctx);
  if (!cablingSCHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map for SuperCells !!!!");
    return;
  }

  const LArOnOffIdMapping* scCabling = cablingSCHdl.cptr();

  SG::ReadCondHandle<LArBadFebCont> mfHdl(m_MFKey, ctx);
  if (!mfHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Missing FEBs container !!!!");
    return;
  }

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
  if (!caloMgrHandle.isValid()) {
    ATH_MSG_ERROR("Do not have CaloDetDescManager !!!");
    return;
  }

  const CaloDetDescrManager* caloDDM = *caloMgrHandle;

  SG::ReadCondHandle<LArBadChannelCont> bcSCHdl(m_badSCKey, ctx);
  if (!bcSCHdl.isValid()) {
    ATH_MSG_ERROR("Do not have BadSCContainer !!!!");
    return;
  }
  const LArBadChannelCont* bcSCCont = *bcSCHdl;

  const auto& badFebs = mfHdl->fullCont();

  unsigned nDeadFebs = 0;
  for (const auto& idBF : badFebs) {
    if (idBF.second.deadReadout()) {
      ++nDeadFebs;
      const HWIdentifier febid(idBF.first);
      ATH_MSG_INFO("FEB " << m_onlineID->channel_name(febid) << " labelled as deadReadout");
      const unsigned nChans = m_onlineID->channelInSlotMax(febid);
      for (unsigned ch = 0; ch < nChans; ++ch) {
        const HWIdentifier chid = m_onlineID->channel_Id(febid, ch);
        const Identifier id = oflCabling->cnvToIdentifier(chid);
        if (!id.is_valid()) {
          ATH_MSG_DEBUG("Regular channel " << m_onlineID->channel_name(chid) << " disconnected. Ignoring.");
          continue; //Disconnected channel (regular readout)
        }
        const IdentifierHash hashId = m_calo_id->calo_cell_hash(id);
        const Identifier scID = m_scidtool->offlineToSuperCellID(id);
        const HWIdentifier scHwid = scCabling->createSignalChannelID(scID);
        if (!bcSCCont->status(scHwid).good()) {
          ATH_MSG_DEBUG("SuperCell with id 0x" << std::hex << scHwid.get_identifier32().get_compact() << std::dec
                                               << " is ignored b/c of it's bad-channel word. Connected to deadFEB channel " << m_onlineID->channel_name(chid));
          continue;
        }
        const unsigned nCell = (m_scidtool->superCellToOfflineID(scID)).size();
        const CaloDetDescrElement* dde = caloDDM->get_element(hashId);
        if (ATH_UNLIKELY(!dde)) {
          ATH_MSG_ERROR("No DetDescElement for cell hash " << hashId);
          return;
        }
        const float convFactor = 12.5 * (1.0 / nCell) * (1.0 / dde->sinTh());
        // 12.5: Convert SC ADC to MeV (Et), et ->e, scale by the number of regular cells connected to this super-cell
        scToHwidMap[scHwid].emplace_back(hashId, convFactor);
      }  // end loop over channels of one dead FEB
    }  // end if feb is deadAll
  }  // end loop over dead febs

  // bit of log-output ...
  ATH_MSG_INFO("Number of deadReadout FEBs for this run: " << nDeadFebs);
  if (msgLvl(MSG::DEBUG)) {
    for (const auto& p : scToHwidMap) {
      ATH_MSG_DEBUG("  SuperCell with id 0x" << std::hex << p.first.get_identifier32().get_compact() << std::dec << " connected to " << p.second.size()
                                             << " deadFEB channels.");
      for (const auto& [h, convFactor] : p.second) {
        const HWIdentifier hwid = cablingHdl->createSignalChannelIDFromHash(h);
        ATH_MSG_DEBUG("      " << m_onlineID->channel_name(hwid) << " " << convFactor);
      }
    }
  }
  sc = StatusCode::SUCCESS;
  return;
}

StatusCode LArCelldeadOTXTool::finalize() {
  if (m_testMode) {
    ATH_MSG_INFO("Test mode for cell-patching:");
    std::vector<std::pair<float, float> > avgList;
    for (auto& [scEne, entry] : m_testMap) {
      avgList.emplace_back(scEne, entry.first / entry.second);
    }
    auto ordering = [](const std::pair<float, float>& a, std::pair<float, float>& b) { return (a.first < b.first); };
    std::sort(avgList.begin(), avgList.end(), ordering);
    for (auto& p : avgList) {
      ATH_MSG_INFO("SCEne=" << p.first << "Avg patching ratio=" << p.second);
    }
  }
  return StatusCode::SUCCESS;
}
