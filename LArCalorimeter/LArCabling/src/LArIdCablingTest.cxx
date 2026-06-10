/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdCablingTest.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"


LArIdCablingTest::~LArIdCablingTest() = default;


StatusCode LArIdCablingTest::initialize() {

  ATH_MSG_INFO ( "initialize()" );
  if (m_isSC && m_cablingKey.key()=="LArOnOffIdMap") {
    ATH_MSG_ERROR("Unexpected cabling key for SuperCellCase, found " <<  m_cablingKey);
  }
  
  ATH_CHECK(m_cablingKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode LArIdCablingTest::execute(const EventContext& ctx) {

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping* cabling=(*cablingHdl);

  const LArOnlineID_Base* larOnlineID = nullptr;
  if (m_isSC) {
    const LArOnline_SuperCellID* ll = nullptr;
    ATH_CHECK(detStore()->retrieve(ll, "LArOnline_SuperCellID"));
    larOnlineID = ll;
    ATH_MSG_DEBUG("Found the LArOnlineID helper");

  } else {  // m_isSC
    const LArOnlineID* ll= nullptr;
    ATH_CHECK(detStore()->retrieve(ll, "LArOnlineID"));
    larOnlineID = ll;
    ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
  }

  const CaloCell_Base_ID* caloId;
  if (m_isSC) {
    const CaloCell_SuperCell_ID* cs = nullptr;
    ATH_CHECK(detStore()->retrieve(cs, "CaloCell_SuperCell_ID"));
    caloId = cs;
  } else {  // m_isSC
    const CaloCell_ID* cc = nullptr;
    ATH_CHECK(detStore()->retrieve(cc, "CaloCell_ID"));
    caloId = cc;
  }

  unsigned nConnected = 0;
  std::array<unsigned,2> nEMECIW{0,0};
  std::array<unsigned,2> nEMECOW{0,0};
  std::array<unsigned,2> nBarrel{0,0}; 
  std::array<unsigned,2> nHEC{0,0};
  std::array<unsigned,2> nFCAL{0,0};



  for ( const HWIdentifier chid : larOnlineID->channel_range()) {
    if (!cabling->isOnlineConnected(chid)) continue;
    ++nConnected;
    const Identifier id=cabling->cnvToIdentifier(chid);
    if (!id.is_valid()) {
      ATH_MSG_ERROR("Invalid offline id for online id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " <<  larOnlineID->channel_name(chid));
    }
    else {
      std::string subdet="Unkown";
      int posneg=larOnlineID->pos_neg(chid);
      if (caloId->is_em_barrel(id)) {subdet="Barrel"; ++nBarrel[posneg];}
      if (caloId->is_em_endcap_inner(id)) {subdet="EMECIW"; ++nEMECIW[posneg];}
      if (caloId->is_em_endcap_outer(id)) {subdet="EMECOW"; ++nEMECOW[posneg];}
      if (caloId->is_hec(id)) {subdet="HEC"; ++nHEC[posneg];}
      if (caloId->is_fcal(id)) {subdet="FCAL"; ++nFCAL[posneg];}

      ATH_MSG_DEBUG("Channel 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                  << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec <<  "   " << subdet);
      if (larOnlineID->isEMBchannel(chid) != caloId->is_em_barrel(id)) {
        ATH_MSG_ERROR("isEMB mismatch online Id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                  << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec << " onl "
                                                  << larOnlineID->isEMBchannel(chid) << " / ofl " << caloId->is_em_barrel(id));
      }
      if (larOnlineID->isEMECIW(chid) != caloId->is_em_endcap_inner(id)) {
        ATH_MSG_ERROR("isEMECIW mismatch online Id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                     << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec << " onl "
                                                     << larOnlineID->isEMECIW(chid) << " / ofl " << caloId->is_em_endcap_inner(id));
      }
      if (larOnlineID->isEMECOW(chid) != caloId->is_em_endcap_outer(id)) {
        ATH_MSG_ERROR("isEMECOW mismatch online Id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                     << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec << " onl "
                                                     << larOnlineID->isEMECOW(chid) << " / ofl " << caloId->is_em_endcap_outer(id));
      }
      if (larOnlineID->isEMECchannel(chid) != caloId->is_em_endcap(id)) {
        ATH_MSG_ERROR("isEMEC mismatch online Id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                     << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec << " onl "
                                                     << larOnlineID->isEMECchannel(chid) << " / ofl " << caloId->is_em_endcap(id));
      }
      if (larOnlineID->isHECchannel(chid) != caloId->is_hec(id)) {
        ATH_MSG_ERROR("isHEC mismatch online Id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                  << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec << " onl "
                                                  << larOnlineID->isHECchannel(chid) << " / ofl " << caloId->is_hec(id));
      }
      if (larOnlineID->isFCALchannel(chid) != caloId->is_fcal(id)) {
        ATH_MSG_ERROR("isFCAL mismatch online Id 0x" << std::hex << chid.get_identifier32().get_compact() << std::dec << " " << larOnlineID->channel_name(chid)
                                                   << " offline id 0x" << std::hex << id.get_identifier32().get_compact() << std::dec << " onl "
                                                   << larOnlineID->isFCALchannel(chid) << "/ ofl" << caloId->is_fcal(id));
      }

    
    }//end if connected
  } //end loop over online identifiers

  if (m_isSC) {
    ATH_MSG_INFO("checked " << nConnected << " super-cell channels.");
  } else {
    ATH_MSG_INFO("checked " << nConnected << " regular channels.");
  }
  ATH_MSG_INFO("Number of channels A/C side:");
  ATH_MSG_INFO("Barrel " << nBarrel[0] << "/" << nBarrel[1]);
  if ( nBarrel[0] != nBarrel[1]) ATH_MSG_ERROR("MISMATCH!");
  ATH_MSG_INFO("EMECOW " << nEMECIW[0] << "/" << nEMECIW[1]);
  if (nEMECIW[0]!=nEMECIW[1]) ATH_MSG_ERROR("MISMATCH!");
  ATH_MSG_INFO("EMECIW " << nEMECOW[0] << "/" << nEMECOW[1]);
  if (nEMECOW[0]!=nEMECOW[1]) ATH_MSG_ERROR("MISMATCH!");
  ATH_MSG_INFO("HEC " << nHEC[0] << "/" << nHEC[1]);
  if (nHEC[0]!=nHEC[1]) ATH_MSG_ERROR("MISMATCH!");
  ATH_MSG_INFO("FCAL " << nFCAL[0] << "/" << nFCAL[1]);
  if (nFCAL[0]!=nFCAL[1]) ATH_MSG_ERROR("MISMATCH!");
  return StatusCode::SUCCESS;
}
