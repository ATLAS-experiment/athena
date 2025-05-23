/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCelldeadOTXTool.h"

#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"
#include "LArElecCalib/LArProvenance.h"

StatusCode LArCelldeadOTXTool::initialize() {

  ATH_CHECK(m_SCKey.initialize());
  ATH_CHECK(m_factors.initialize());

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

  SG::ReadCondHandle<LArDeadOTXCorrFactors> scToDeadHdl{m_factors,ctx};
  const LArDeadOTXCorrFactors::payload_t& scToDead=scToDeadHdl->get();
  if (scToDead.empty()) {
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
    auto itr = scToDead.find(scHwid);
    if (itr == scToDead.end())
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
      ATH_MSG_VERBOSE("SuperCell value " << scEne << " below threshold, ignoring");
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
