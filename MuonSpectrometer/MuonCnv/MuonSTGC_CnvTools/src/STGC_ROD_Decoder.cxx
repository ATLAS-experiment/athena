/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <unordered_map>

#include "MuonRDO/STGC_RawData.h"
#include "MuonRDO/STGC_RawDataContainer.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "MuonNSWCommonDecode/NSWCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWElink.h"
#include "MuonNSWCommonDecode/VMMChannel.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
 
#include "Identifier/Identifier.h"
#include "eformat/Issue.h"
#include "STGC_ROD_Decoder.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

//==============================================================================
Muon::STGC_ROD_Decoder::STGC_ROD_Decoder(const std::string& t, const std::string& n, const IInterface*  p) 
: AthAlgTool(t, n, p)
{
  declareInterface<ISTGC_ROD_Decoder>(this);
}


//==============================================================================
StatusCode Muon::STGC_ROD_Decoder::initialize()
{
  ATH_CHECK(detStore()->retrieve(m_stgcIdHelper, "STGCIDHELPER"));
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_dscKey.initialize(!m_dscKey.empty()));
  ATH_CHECK(m_cablingKey.initialize(!m_cablingKey.empty()));
  return StatusCode::SUCCESS;
}


//===============================================================================
// Processes a ROB fragment and fills the RDO container. 
// If the vector of IdentifierHashes is not empty, then seeded mode is assumed 
// (only requested modules are decoded). This must be made here, because the 
// trigger granularity is quadruplet, whereas ROB granularity is a whole sector 
// (or wedge). Therefore, refined selection is needed with decoded information.
StatusCode Muon::STGC_ROD_Decoder::fillCollection(const EventContext& ctx,
                                                  const ROBFragment& robFrag, 
                                                  const std::vector<IdentifierHash>& rdoIdhVect, 
                                                  std::unordered_map<IdentifierHash, std::unique_ptr<STGC_RawDataCollection>>& rdo_map) const
{

  // check fragment for errors
  try {
    robFrag.check();
  } catch (const eformat::Issue &ex) {
    ATH_MSG_WARNING(ex.what());
    return StatusCode::SUCCESS;
  }
  
  const NswDcsDbData* dcsData{nullptr};
  if(!m_dscKey.empty()) {
     SG::ReadCondHandle<NswDcsDbData> readCondHandle{m_dscKey, ctx};
     if(!readCondHandle.isValid()){
        ATH_MSG_ERROR("Cannot find the NSW DcsCondDataObj "<<m_dscKey.fullKey());
        return StatusCode::FAILURE;
     }
     dcsData = readCondHandle.cptr();
  }

  const Nsw_CablingMap* sTgcCablingMap{nullptr};
  if (!m_cablingKey.empty()) {
      SG::ReadCondHandle<Nsw_CablingMap> readCondHandle{m_cablingKey, ctx};
      if(!readCondHandle.isValid()){
        ATH_MSG_ERROR("Cannot find Micromegas cabling map!");
        return StatusCode::FAILURE;
      }
      sTgcCablingMap = readCondHandle.cptr();
  }

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> muonGeoMgrHandle{m_DetectorManagerKey, ctx};

  if (!muonGeoMgrHandle.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve the detector manager from the conditions store");
      return StatusCode::FAILURE;
  }

  const MuonGM::MuonDetectorManager* muonGeoMgr = *muonGeoMgrHandle;

  // if the vector of hashes is not empty, then we are in seeded mode
  bool seeded_mode(!rdoIdhVect.empty());

  // have the NSWCommonDecoder take care of the decoding
  Muon::nsw::NSWCommonDecoder common_decoder(robFrag);  
  const std::vector<Muon::nsw::NSWElink *>&   elinks = common_decoder.get_elinks();  
  ATH_MSG_DEBUG("Retrieved "<<elinks.size()<<" elinks");
  if (elinks.empty()) return StatusCode::SUCCESS;

  // loop on elinks. for STGCs a "module" is a quadruplet
  // therefore, we need an RDO (collection) per quadruplet!
  for (auto* elink : elinks) {

    // skip null packets
    if (elink->isNull()) continue;
    
    // get the offline ID hash (module ctx) to be passed to the RDO 
    // also specifies the index of the RDO in the container.
    const char*  station_name = elink->elinkId()->is_large_station() ? "STL" : "STS";
    int          station_eta  = (int)elink->elinkId()->station_eta();
    unsigned int station_phi  = (unsigned int)elink->elinkId()->station_phi();
    unsigned int multi_layer  = (unsigned int)elink->elinkId()->multi_layer();
    unsigned int gas_gap      = (unsigned int)elink->elinkId()->gas_gap();
    Identifier   module_ID    = m_stgcIdHelper->elementID(station_name, station_eta, station_phi);

    IdentifierHash module_hashID;
    m_stgcIdHelper->get_module_hash(module_ID, module_hashID);
    const MuonGM::sTgcReadoutElement* roElement = muonGeoMgr->getsTgcReadoutElement(module_ID);
    // if we are in ROI-seeded mode, check if this hashID is requested
    if (seeded_mode && std::find(rdoIdhVect.begin(), rdoIdhVect.end(), module_hashID) == rdoIdhVect.end()) continue;
    std::unique_ptr<STGC_RawDataCollection>& rdo = rdo_map[module_hashID];
    if (!rdo) rdo = std::make_unique<STGC_RawDataCollection>(module_hashID);
  
    // loop on all channels of this elink to fill the collection
    const std::vector<Muon::nsw::VMMChannel*>& channels = elink->get_channels();
    for (auto *channel : channels) {
       unsigned int channel_number = channel->channel_number();
       unsigned int channel_type   = channel->channel_type();
       if (channel_number == 0) continue; // skip disconnected vmm channels

       Identifier channel_ID = m_stgcIdHelper->channelID(module_ID, multi_layer, gas_gap, channel_type, channel_number); // not validating the IDs (too slow)
       if (sTgcCablingMap) {
          std::optional<Identifier> correctedChannelId = sTgcCablingMap->correctChannel(channel_ID, msgStream());
          if (!correctedChannelId) {
              ATH_MSG_DEBUG("Channel was shifted outside its connector and is therefore not decoded into and RDO");
              continue;
          }
          channel_ID = (*correctedChannelId);
       }
       bool isOuterQ1{false};
       if(std::abs(station_eta)==1 && dcsData){ // only the innermost quad is split in two hv sections, no need to check the others, also no need to check if no dcs data is loaded
          Amg::Vector2D localPos{Amg::Vector2D::Zero()};
          if (!roElement->stripPosition(channel_ID, localPos)) continue; // can also handle wires and pads
          isOuterQ1 = !(roElement->isEtaZero(channel_ID, localPos));
       }

       if (dcsData && !dcsData->isGood(ctx, channel_ID, isOuterQ1)) continue;
       bool timeAndChargeInCounts = true; // always true for data from detector
       rdo->push_back(new STGC_RawData(channel_ID, channel->rel_bcid(), channel->tdo(), channel->pdo(), false,timeAndChargeInCounts)); // isDead = false (ok?)
    }
  }

  return StatusCode::SUCCESS;
}



