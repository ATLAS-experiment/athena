/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDeadOTXCondAlg.h"
#include "LArIdentifier/LArOnlineID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

StatusCode LArDeadOTXCondAlg::initialize() {
 
  ATH_CHECK(m_MFKey.initialize());
  ATH_CHECK(m_badSCKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_cablingSCKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());

  ATH_CHECK(m_outputKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));

  ATH_CHECK(m_scidtool.retrieve());   


  return StatusCode::SUCCESS;
}

StatusCode LArDeadOTXCondAlg::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("executing");

  SG::WriteCondHandle<LArDeadOTXCorrFactors> outputHdl{m_outputKey,ctx};
  
  if (outputHdl.isValid()) {
    ATH_MSG_DEBUG("Found valid write handle");
    return StatusCode::SUCCESS;
  }  

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey, ctx);
  if (!cablingHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map !!!!");
    return StatusCode::FAILURE;
  }
  outputHdl.addDependency(cablingHdl);
  const LArOnOffIdMapping* oflCabling = cablingHdl.cptr();

  SG::ReadCondHandle<LArOnOffIdMapping> cablingSCHdl(m_cablingSCKey, ctx);
  if (!cablingSCHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map for SuperCells !!!!");
    return StatusCode::FAILURE;
  }
  outputHdl.addDependency(cablingSCHdl);
  const LArOnOffIdMapping* scCabling = cablingSCHdl.cptr();

  SG::ReadCondHandle<LArBadFebCont> mfHdl(m_MFKey, ctx);
  if (!mfHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Missing FEBs container !!!!");
    return StatusCode::FAILURE;
  }
  outputHdl.addDependency(mfHdl);

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
  if (!caloMgrHandle.isValid()) {
    ATH_MSG_ERROR("Do not have CaloDetDescManager !!!");
    return StatusCode::FAILURE;
  }
  outputHdl.addDependency(caloMgrHandle);
  const CaloDetDescrManager* caloDDM = *caloMgrHandle;

  SG::ReadCondHandle<LArBadChannelCont> bcSCHdl(m_badSCKey, ctx);
  if (!bcSCHdl.isValid()) {
    ATH_MSG_ERROR("Do not have BadSCContainer !!!!");
    return StatusCode::FAILURE;
  }
  outputHdl.addDependency(bcSCHdl);
  const LArBadChannelCont* bcSCCont = *bcSCHdl;

  const auto& badFebs = mfHdl->fullCont();

  std::unique_ptr<LArDeadOTXCorrFactors> output=std::make_unique<LArDeadOTXCorrFactors>();

  auto& scToHwidMap=output->get();

  unsigned nDeadFebs = 0;
  for (const auto& idBF : badFebs) {
    if (idBF.second.deadReadout()) {
      ++nDeadFebs;
      const HWIdentifier febid(idBF.first);
      ATH_MSG_INFO("FEB " << m_onlineID->channel_name(febid) << " labelled as deadReadout");
      const int nChans = m_onlineID->channelInSlotMax(febid);
      if (nChans < 1) continue; //channelInSlotMax can return -999
      for (int ch = 0; ch < nChans; ++ch) {
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
          return StatusCode::FAILURE;
        }
        const float convFactor = 12.5 * (1.0 / nCell) * (1.0 / dde->sinTh());
        // 12.5: Convert SC ADC to MeV (Et), et ->e, scale by the number of regular cells connected to this super-cell
        scToHwidMap[scHwid].emplace_back(hashId, convFactor); //scToHWidMap is a reference to the payload of the output cond obj
      }  // end loop over channels of one dead FEB
    }  // end if feb is deadAll
  }  // end loop over dead febs

  if(outputHdl.record(std::move(output)).isFailure()) {
    ATH_MSG_ERROR("Could not record LArOnOffMapping object with " 
                  << outputHdl.key() 
                  << " with EventRange " << outputHdl.getRange()
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new " << outputHdl.key() << " with range " << outputHdl.getRange() << " into Conditions Store");



  // bit of log-output ...
  ATH_MSG_INFO("Number of deadReadout FEBs for this IOV: " << nDeadFebs);
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
  return StatusCode::SUCCESS;
}




