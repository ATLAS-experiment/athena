/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigmuRoI.h"
#include "TrigT1Result/RoIBResult.h"
#include "TrigT1Interfaces/RecMuonRoI.h"
#include "CxxUtils/phihelper.h"

// ================================================================================
// ================================================================================

TrigmuRoI::TrigmuRoI(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{   
}


// ================================================================================
// ================================================================================

StatusCode TrigmuRoI::initialize()
{

  // Retrieve the tools/services
  ATH_CHECK(m_roisWriteHandleKey.initialize());
  ATH_CHECK(m_L1OutOfTimeRoIBCm2Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_L1OutOfTimeRoIBCm1Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_L1OutOfTimeRoIBCp1Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_L1OutOfTimeRoIBCp2Key.initialize(SG::AllowEmpty));

  if (!m_monTool.empty()) {
    ATH_MSG_DEBUG("Retrieving monTool");
    CHECK(m_monTool.retrieve());
  } else {
    ATH_MSG_INFO("No monTool configured => NO MONITORING");
  }

  return StatusCode::SUCCESS;
}


// ================================================================================
// ================================================================================

unsigned int TrigmuRoI::getBitMaskValue( const unsigned int uintValue, const unsigned int mask ) const {
  unsigned int result;
  unsigned int maskcopy;
  maskcopy = mask;
  result = uintValue & mask;
  if ( mask != 0 ) {
    while ( ( maskcopy & 0x00000001 ) == 0 ) {
      maskcopy = maskcopy >> 1;
      result = result >> 1;
    }
  }
  return result;
}


// ================================================================================
// ================================================================================

StatusCode TrigmuRoI::readAndAppendTrigRoiDescriptors(const EventContext& ctx, TrigRoiDescriptorCollection* roiColl, const SG::ReadHandleKey<xAOD::MuonRoIContainer>& readHandleKey, int bc_shift, unsigned int& roi_id, std::vector<int>& RpcBCShift, std::vector<int>& TgcBCShift, std::vector<float>& RoIEta, std::vector<float>& RoIPhi) const 
{

  // check validity of the key
  if (readHandleKey.empty()) {
    ATH_MSG_VERBOSE("Empty ReadHandleKey for the out-of-time muon RoIs.");
    return StatusCode::SUCCESS;
  }

  // create the read handle from the key
  SG::ReadHandle<xAOD::MuonRoIContainer> rh_outOfTimeRoIs(readHandleKey, ctx);

  // check validity of the read handle
  ATH_CHECK(rh_outOfTimeRoIs.isPresent());
  ATH_CHECK(rh_outOfTimeRoIs.isValid());

  // loop over the RoIs
  for (const xAOD::MuonRoI* roi : *rh_outOfTimeRoIs) {

    // debug messages
    ATH_MSG_DEBUG("====== (late-muon) RoI debug info for out-of-time RoI =====");
    ATH_MSG_DEBUG("out-of-time bunch crossing shift : " << bc_shift);
    ATH_MSG_DEBUG("RoI ID (within TrigmuRoI)        : " << roi_id);
    ATH_MSG_DEBUG("RoI pT threshold                 : name: " << roi->thrName() << ", number: "<< roi->getThrNumber() << ", value: " << roi->thrValue());
    ATH_MSG_DEBUG("RoI eta                          : " << roi->eta());
    ATH_MSG_DEBUG("RoI phi                          : " << roi->phi());
    ATH_MSG_DEBUG("RoI word                         : 0x" << MSG::hex << roi->roiWord() << MSG::dec);
    ATH_MSG_DEBUG("Sector ID                        : " << roi->getSectorID());
    ATH_MSG_DEBUG("Sector address                   : 0x" << MSG::hex << roi->getSectorAddress() << MSG::dec);
    ATH_MSG_DEBUG("==========================================================");

    // fill monitoring collections: monitor the roi's eta and phi
    RoIEta.push_back(roi->eta());
    RoIPhi.push_back(roi->phi());

    // fill monitoring collections: construct the system ID
    unsigned int temp_sysID = getBitMaskValue(roi->getSectorAddress(), LVL1::SysIDMask );
    unsigned int sysID = 0;                // Barrel
    if( temp_sysID & 0x2 ) sysID = 1;      // Endcap
    else if( temp_sysID & 0x1 ) sysID = 2; // Forward

    // fill monitoring collections: monitor the covered bunch crossing shift values for RPC and TGC
    if ( sysID == 0 ) RpcBCShift.push_back(bc_shift);
    else              TgcBCShift.push_back(bc_shift);
    
    // now create the new RoI descriptor and add it to the collection
    roiColl->push_back(std::make_unique<TrigRoiDescriptor>(
          roi->roiWord(), 0u, roi_id,
          roi->eta(), roi->eta()-m_roiHalfWidthEta, roi->eta()+m_roiHalfWidthEta,
          roi->phi(), CxxUtils::wrapToPi(roi->phi()-m_roiHalfWidthPhi), CxxUtils::wrapToPi(roi->phi()+m_roiHalfWidthPhi)
        ));
  
    // bump up ID
    roi_id += 1;
  }
    
  // all done
  return StatusCode::SUCCESS;

}

// ================================================================================
// ================================================================================

StatusCode TrigmuRoI::execute(const EventContext& ctx) const
{

  // set up monitoring
  std::vector<int> RpcBCShift, TgcBCShift;
  std::vector<float> RoIEta, RoIPhi;
  auto Rpc_OutOfTimeBCShift = Monitored::Collection("Rpc_OutOfTimeBCShift", RpcBCShift);
  auto Tgc_OutOfTimeBCShift = Monitored::Collection("Tgc_OutOfTimeBCShift", TgcBCShift);
  auto OutOfTimeRoI_Eta = Monitored::Collection("OutOfTimeRoI_Eta", RoIEta);
  auto OutOfTimeRoI_Phi = Monitored::Collection("OutOfTimeRoI_Phi", RoIPhi);
  auto mon = Monitored::Group(m_monTool, Rpc_OutOfTimeBCShift, Tgc_OutOfTimeBCShift, OutOfTimeRoI_Eta, OutOfTimeRoI_Phi);

  // make an empty roi descriptor collection
  SG::WriteHandle<TrigRoiDescriptorCollection> wh_roiCollection(m_roisWriteHandleKey, ctx);
  ATH_CHECK(wh_roiCollection.record(std::make_unique<TrigRoiDescriptorCollection>()));
  TrigRoiDescriptorCollection* roiColl = wh_roiCollection.ptr();

  // set up an id counter
  unsigned int roi_id = 0;

  // construct the trigger RoI descriptors for the four out-of-time bunch crossings (they are then added to the roiColl)
  ATH_CHECK(readAndAppendTrigRoiDescriptors(ctx, roiColl, m_L1OutOfTimeRoIBCm2Key, -2, roi_id, RpcBCShift, TgcBCShift, RoIEta, RoIPhi));
  ATH_CHECK(readAndAppendTrigRoiDescriptors(ctx, roiColl, m_L1OutOfTimeRoIBCm1Key, -1, roi_id, RpcBCShift, TgcBCShift, RoIEta, RoIPhi));
  ATH_CHECK(readAndAppendTrigRoiDescriptors(ctx, roiColl, m_L1OutOfTimeRoIBCp1Key, +1, roi_id, RpcBCShift, TgcBCShift, RoIEta, RoIPhi));
  ATH_CHECK(readAndAppendTrigRoiDescriptors(ctx, roiColl, m_L1OutOfTimeRoIBCp2Key, +2, roi_id, RpcBCShift, TgcBCShift, RoIEta, RoIPhi));

  // all done
  return StatusCode::SUCCESS;

}


