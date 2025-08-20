//dear emacs, this is -*-c++-*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArMCSymCondAlg.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include <cmath> //std::abs


StatusCode LArMCSymCondAlg::initialize() {

  ATH_MSG_DEBUG("initializing");

  // Read Handles
  ATH_CHECK( m_readKey.initialize() );
  ATH_CHECK( m_writeKey.initialize() );

  return StatusCode::SUCCESS;
}


StatusCode LArMCSymCondAlg::execute() {
    
  SG::WriteCondHandle<LArMCSym> writeHandle{m_writeKey};
  
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("Found valid write handle");
    return StatusCode::SUCCESS;
  }  

  SG::ReadCondHandle<LArOnOffIdMapping> readHandle{m_readKey};
  const LArOnOffIdMapping* cabling{*readHandle};

  if (cabling==nullptr) {
    ATH_MSG_ERROR("Failed to retrieve CondAttributeListCollection with key " << m_readKey.key());
    return StatusCode::FAILURE;
  }
  writeHandle.addDependency(readHandle);

  const LArOnlineID_Base *larOnlineID=nullptr;
  const CaloCell_Base_ID *caloCellID=nullptr;
  if(m_isSC){
     const LArOnline_SuperCellID *ll;
     ATH_CHECK(detStore()->retrieve(ll,"LArOnline_SuperCellID"));
     larOnlineID=static_cast<const LArOnlineID_Base*>(ll);
     const CaloCell_SuperCell_ID *cal;
     ATH_CHECK(detStore()->retrieve(cal,"CaloCell_SuperCell_ID"));
     caloCellID=static_cast<const CaloCell_Base_ID *>(cal);
  } else {
     const LArOnlineID *ll;
     ATH_CHECK(detStore()->retrieve(ll,"LArOnlineID"));
     larOnlineID=static_cast<const LArOnlineID_Base*>(ll);
     const CaloCell_ID *cal;
     ATH_CHECK(detStore()->retrieve(cal,"CaloCell_ID"));
     caloCellID=static_cast<const CaloCell_Base_ID *>(cal);
  }

  const LArEM_Base_ID* lar_em_id=caloCellID->em_idHelper();
  const LArHEC_Base_ID* lar_hec_id=caloCellID->hec_idHelper();
  const LArFCAL_Base_ID* lar_fcal_id=caloCellID->fcal_idHelper();

  const unsigned ncellem=lar_em_id->channel_hash_max();
  const unsigned ncellhec=lar_hec_id->channel_hash_max();
  const unsigned ncellfcal=lar_fcal_id->channel_hash_max();

  const unsigned larHashMax=ncellem+ncellhec+ncellfcal;

  const unsigned onlHashMax=larOnlineID->channelHashMax();

  std::vector<HWIdentifier> oflHashtoSymOnl(larHashMax);
  std::vector<HWIdentifier> onlHashtoSymOnl(onlHashMax);
 
  std::set<HWIdentifier> symIds;
 
  ATH_MSG_DEBUG("Start looping over EM calo cells");
  for ( unsigned int idhash=0; idhash<ncellem;idhash++){
    const Identifier id=lar_em_id->channel_id (idhash);
    const HWIdentifier hwid=cabling->createSignalChannelID(id);
    const IdentifierHash hwid_hash=larOnlineID->channel_Hash(hwid);
    const int barrel_ec = std::abs( lar_em_id->barrel_ec(id) ) ;
    const int sampling  = lar_em_id->sampling(id);
    const int region    = lar_em_id->region(id);
    const Identifier regId = lar_em_id->region_id(barrel_ec, sampling, region );
    const int eta       = lar_em_id->eta(id);
    const int phi       = lar_em_id->phi_min(regId);
    const Identifier symOffId = lar_em_id->channel_id(barrel_ec, sampling, region, eta, phi );
    const HWIdentifier symOnId = cabling->createSignalChannelID(symOffId);
    const IdentifierHash idHash = lar_em_id->channel_hash(id);
    oflHashtoSymOnl[idHash] = symOnId;
    onlHashtoSymOnl[hwid_hash]= symOnId;
    symIds.insert(symOnId);
  }

  ATH_MSG_DEBUG("start loop over HEC calo");

  for ( unsigned int idhash=0; idhash<ncellhec;idhash++){
    const Identifier id=lar_hec_id->channel_id (idhash);
    const HWIdentifier hwid=cabling->createSignalChannelID(id);
    const IdentifierHash hwid_hash=larOnlineID->channel_Hash(hwid);

    const int pos_neg   = std::abs( lar_hec_id->pos_neg(id) ) ;
    const int sampling  = lar_hec_id->sampling(id);
    const int region    = lar_hec_id->region(id);
    const Identifier regId = lar_hec_id->region_id(pos_neg, sampling, region );
    const int eta       = lar_hec_id->eta(id);
    const int phi       = lar_hec_id->phi_min(regId);
    const Identifier symOffId = lar_hec_id->channel_id(pos_neg, sampling, region, eta, phi );
    const HWIdentifier symOnId = cabling->createSignalChannelID(symOffId);
    const IdentifierHash idHash=lar_hec_id->channel_hash(id);
    oflHashtoSymOnl[ncellem+idHash] = symOnId;
    onlHashtoSymOnl[hwid_hash]= symOnId;
    symIds.insert(symOnId);
  }

  ATH_MSG_DEBUG("start loop over FCAL calo");

  const bool isTB= lar_fcal_id->dictionaryVersion() == "H6TestBeam"; 
  for ( unsigned int idhash=0; idhash<lar_fcal_id->channel_hash_max();
       idhash++){
    const Identifier id=lar_fcal_id->channel_id (idhash);
    const HWIdentifier hwid=cabling->createSignalChannelID(id);
    const IdentifierHash hwid_hash=larOnlineID->channel_Hash(hwid);


    const int pos_neg   = std::abs( lar_fcal_id->pos_neg(id) ) ;
    const int module    = lar_fcal_id->module(id);

// symmetry for FCAL: save all y>0 part of the +side module
//                             y<0 side deduced by point symmetry around x=y=0
    const int eta  = lar_fcal_id->eta(id);
    int phi  = lar_fcal_id->phi(id);
    if (!isTB) {
      if (phi > 7) phi = phi-8;
    }
    Identifier symOffId = lar_fcal_id->channel_id(pos_neg, module, eta, phi );
    HWIdentifier symOnId = cabling->createSignalChannelID(symOffId);
    IdentifierHash idHash=lar_fcal_id->channel_hash(id);
    oflHashtoSymOnl[ncellem+ncellhec+idHash] = symOnId;
    onlHashtoSymOnl[hwid_hash]= symOnId;
    symIds.insert(symOnId);
  }


  std::vector<HWIdentifier> symIdVec(symIds.begin(),symIds.end());
  std::unique_ptr<LArMCSym> mcSym=std::make_unique<LArMCSym>(larOnlineID,caloCellID,
							     std::move(oflHashtoSymOnl),
							     std::move(onlHashtoSymOnl),
							     std::move(symIdVec));


  
  if(writeHandle.record(std::move(mcSym)).isFailure()) {
    ATH_MSG_ERROR("Could not record LArMCSym object with " 
		  << writeHandle.key() 
		  << " with EventRange " << writeHandle.getRange()
		  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

 
  return StatusCode::SUCCESS;
}

