/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCelldeadOTXAlg.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "LArElecCalib/LArProvenance.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "StoreGate/WriteHandle.h"

StatusCode LArCelldeadOTXAlg::initialize() {

  ATH_CHECK(m_deadOTXFromSCKey.initialize());
  ATH_CHECK(m_SCKey.initialize());
  ATH_CHECK(m_MFKey.initialize());
  ATH_CHECK(m_badSCKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_cablingSCKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_onlineSCID, "LArOnline_SuperCellID"));
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));

  ATH_CHECK(m_scidtool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode LArCelldeadOTXAlg::execute(const EventContext& ctx) const {

  ATH_MSG_VERBOSE(" in execute...");

  StatusCode sc = StatusCode::SUCCESS;
  std::call_once(m_onceFlag, &LArCelldeadOTXAlg::buildMap, this, ctx, sc);

  if (sc.isFailure()) {
    ATH_MSG_ERROR("Call to LArCelldeadOTXAlg::buidMap returned an error");
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<LArDeadOTXFromSC> deadHdl(m_deadOTXFromSCKey,ctx);
  std::unique_ptr<LArDeadOTXFromSC> deadHandle = std::make_unique<LArDeadOTXFromSC>();

  // get SuperCellContainer
  SG::ReadHandle<LArRawSCContainer> scHdl(m_SCKey, ctx);
  if (!scHdl.isValid()) {
    ATH_MSG_WARNING("Do not have SuperCell container no patching !!!!");
    return StatusCode::SUCCESS;
  }

  const unsigned int bcid = ctx.eventID().bunch_crossing_id();

  // get the SC, container is unordered, so have to loop
  const LArRawSCContainer* scells = scHdl.cptr();
  std::vector<float> vecEnergies;
  vecEnergies.reserve(m_onlineSCID->channelHashMax());
  vecEnergies.resize(m_onlineSCID->channelHashMax(),0.0);
  for (const auto* supercell : *scells) {
    if (!supercell)
      continue;
    const HWIdentifier scHwid = supercell->hardwareID();
    uint32_t scIDHash = (m_onlineSCID->channel_Hash(scHwid)).value();

    const std::vector<unsigned short>& bcids = supercell->bcids();
    const std::vector<int>& energies = supercell->energies();
    const std::vector<bool>& satur = supercell->satur();

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
    vecEnergies[scIDHash]=scEne;
  }  // End loop over SuperCell container

  for(size_t i=0;i<m_channels.size();i++){
        std::vector<uint32_t>& chans = m_channels.at(i);
        std::vector<float>& mults = m_multipliers.at(i);
        std::vector<float> cellEnergies;
        cellEnergies.resize(128,0);
        for(size_t j=0; j<chans.size(); j++) {
            if ( chans[j] < 0xfffffffe ) cellEnergies[j] = mults[j]*vecEnergies[chans[j]];
        }
	deadHandle->addFEB(m_febs[i],cellEnergies);
  }
  ATH_CHECK(deadHdl.record(std::move(deadHandle)) );

    /// keep this code here, but commented for future developement and testing purposes
    //cell->setProvenance(cell->provenance() | LArProv::PATCHED);

  return StatusCode::SUCCESS;
}

void LArCelldeadOTXAlg::buildMap(const EventContext& ctx, StatusCode& sc) const {

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

  const std::vector<std::pair<unsigned int, LArBadFeb> >& badFebs = mfHdl->fullCont();

  unsigned nDeadFebs = 0;
  for (const auto& idBF : badFebs) {
    if (idBF.second.deadReadout()) {
      ++nDeadFebs;
      const HWIdentifier febid(idBF.first);
      m_febs.push_back(febid);
      ATH_MSG_INFO("FEB " << m_onlineID->channel_name(febid) << " labelled as dead");
      std::vector<float> vector_of_multipliers;
      std::vector<uint32_t> vector_of_chans;
      const unsigned nChans = m_onlineID->channelInSlotMax(febid);
      vector_of_multipliers.resize(nChans,0.0);
      vector_of_chans.resize(nChans,0xffffffff);
      for (unsigned ch = 0; ch < nChans; ++ch) {
        const HWIdentifier chid = m_onlineID->channel_Id(febid, ch);
        const Identifier id = oflCabling->cnvToIdentifier(chid);
        const IdentifierHash hashId = m_calo_id->calo_cell_hash(id);
        const Identifier scID = m_scidtool->offlineToSuperCellID(id);
        const HWIdentifier scHwid = scCabling->createSignalChannelID(scID);
        const IdentifierHash hashidSC = m_onlineSCID->channel_Hash(scHwid);
        if (!bcSCCont->status(scHwid).good()) {
          ATH_MSG_DEBUG("SuperCell with id 0x" << std::hex << scHwid.get_identifier32().get_compact() << std::dec
                                               << " is ignored b/c of it's bad-channel word. Connected to deadFEB channel " << m_onlineID->channel_name(chid));
          continue;
        }
        const unsigned nCell = (m_scidtool->superCellToOfflineID(scID)).size();
        const CaloDetDescrElement* dde = caloDDM->get_element(hashId);
        if (ATH_UNLIKELY(!dde)) {
          ATH_MSG_INFO("No DetDescElement for cell hash : " << hashId);
	} else {
          // 12.5: Convert SC ADC to MeV (Et), et ->e, scale by the number of regular cells connected to this super-cell
          const float convFactor = 12.5 * (1.0 / nCell) * (1.0 / dde->sinTh());
          vector_of_multipliers[ch]=convFactor;
          vector_of_chans[ch]=hashidSC.value();
        }
      }  // end loop over channels of one dead FEB
      m_multipliers.push_back(vector_of_multipliers);
      m_channels.push_back(vector_of_chans);
    }  // end if feb is deadAll
  }  // end loop over dead febs

  sc = StatusCode::SUCCESS;
  return;
}

