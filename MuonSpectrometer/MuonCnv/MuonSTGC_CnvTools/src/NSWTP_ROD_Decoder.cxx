/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "NSWTP_ROD_Decoder.h"
#include "Identifier/Identifier.h"
#include "eformat/Issue.h"
#include "eformat/SourceIdentifier.h"
#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"
#include "MuonNSWCommonDecode/STGTPPackets.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"


namespace Muon {

using namespace nsw::STGTPSegments;
using STGTPSegmentPacket = nsw::STGTPSegmentPacket;
using STGTPPadPacket = nsw::STGTPPadPacket;
using STGTPMMPacket = nsw::STGTPMMPacket;
using namespace nsw::STGTPMMData;
//=====================================================================
NSWTP_ROD_Decoder::NSWTP_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent)
: AthAlgTool(type, name, parent)
{
  declareInterface<INSWTP_ROD_Decoder>(this);
}

StatusCode NSWTP_ROD_Decoder::initialize() {
  ATH_CHECK(m_idHelperSvc.retrieve());
  return StatusCode::SUCCESS;
}

//=====================================================================
StatusCode NSWTP_ROD_Decoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, xAOD::NSWTPRDOContainer& rdoContainer) const
{
  try {
    fragment.check();
  } catch (const eformat::Issue& ex) {
    ATH_MSG_ERROR(ex.what());
    return StatusCode::FAILURE;
  }

  Muon::nsw::NSWTriggerCommonDecoder nsw_trigger_decoder (fragment, "STGL1A");
 
  for(const auto& baseLink: nsw_trigger_decoder.get_elinks()){
    /// Create the new trigger processor RDO
    xAOD::NSWTPRDO* rdo = new xAOD::NSWTPRDO();
    rdoContainer.push_back(rdo);
    const auto link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerSTGL1AElink>(baseLink);

    const std::shared_ptr<Muon::nsw::NSWResourceId>& elinkID =  link->elinkId ();

    uint32_t moduleID{0};
    encodeIdentifierProperty(ModuleIDProperty::stationID, elinkID->is_large_station(), moduleID );
    encodeIdentifierProperty(ModuleIDProperty::detectorSite, elinkID->station_eta() > 0 , moduleID );
    encodeIdentifierProperty(ModuleIDProperty::stationEta, std::abs(elinkID->station_eta()) , moduleID );
    encodeIdentifierProperty(ModuleIDProperty::stationPhi, elinkID->station_phi() , moduleID );
    rdo->set_moduleID(moduleID); 

    // now filling all the header data
    rdo->set_ROD_L1ID(fragment.rod_lvl1_id ());
    rdo->set_sectID(link->head_sectID());
    rdo->set_EC(link->head_EC());
    rdo->set_BCID(link->head_BCID());
    rdo->set_L1ID(link->L1ID());
    rdo->set_window_open_bcid(link->l1a_open_BCID());
    rdo->set_l1a_request_bcid(link->l1a_req_BCID());
    rdo->set_window_close_bcid(link->l1a_close_BCID());
    rdo->set_config_window_open_bcid_offset(link->l1a_open_BCID_offset());
    rdo->set_config_l1a_request_bcid_offset(link->l1a_req_BCID_offset());
    rdo->set_config_window_close_bcid_offset(link->l1a_close_BCID_offset());



    



    // now we are filling all the pad segment variables 

    const std::vector<STGTPPadPacket>& pad_packets = link->pad_packets();
    for(uint i_packetIndex = 0; i_packetIndex<pad_packets.size(); i_packetIndex++){
      const STGTPPadPacket& pad_packet = pad_packets.at(i_packetIndex);
      for(uint i_candidateIndex=0; i_candidateIndex < 4; i_candidateIndex++){ // we have at most 4 candidates in the input
        if(pad_packet.BandID(i_candidateIndex) == 255 && pad_packet.PhiID(i_candidateIndex) == 63) continue; // ignore candidates that the trigger processor flags as invalid
        uint8_t candidateNumber = (i_packetIndex<<4) | i_candidateIndex;
        rdo->pad_candidateNumber().push_back(candidateNumber);
        rdo->pad_phiID().push_back(pad_packet.PhiID(i_candidateIndex));
        rdo->pad_bandID().push_back(pad_packet.BandID(i_candidateIndex));
      }
      rdo->pad_BCID().push_back(pad_packet.BCID());
      rdo->pad_idleFlag().push_back(pad_packet.PadIdleFlag());
      rdo->pad_coincidence_wedge().push_back(pad_packet.CoincidenceWedge());

    }

    // lets fill the output segments (merged) 
    const std::vector<STGTPSegmentPacket>& segment_packets =  link->segment_packet();
    for(uint i_packetIndex = 0; i_packetIndex<segment_packets.size(); i_packetIndex++){
      const STGTPSegmentPacket& segment_packet = segment_packets.at(i_packetIndex);
      uint8_t i_candidateIndex{0};
       for (const STGTPSegmentPacket::SegmentData& payload : segment_packet.Segments()){
        // we have at most 8 candidates in the output
        if(payload.dTheta == 16) {
           ++i_candidateIndex;
           continue; // ignore candidates that the trigger processor flags as invalid
        }
        uint32_t word{0}; // word containing all information about the candidate
        encodeSegmentProperty(MergedSegmentProperty::Monitor, payload.monitor ,word);
        encodeSegmentProperty(MergedSegmentProperty::Spare, payload.spare ,word);
        encodeSegmentProperty(MergedSegmentProperty::lowRes, payload.lowRes ,word);
        encodeSegmentProperty(MergedSegmentProperty::phiRes, payload.phiRes,word);
        encodeSegmentProperty(MergedSegmentProperty::dTheta, payload.dTheta,word);
        encodeSegmentProperty(MergedSegmentProperty::phiID, payload.phiID,word);
        encodeSegmentProperty(MergedSegmentProperty::rIndex, payload.rIndex ,word);
        uint8_t candidateNumber = (i_packetIndex<<4) | i_candidateIndex;
        ++i_candidateIndex;
        
        rdo->merge_segments().push_back(word);
        rdo->merge_candidateNumber().push_back(candidateNumber);
      }
      // the first 12 bit are used for the bcid and the last 4 for sector ID
      uint16_t merge_BCID_sectorID = (segment_packet.BCID() << 4) | segment_packet.SectorID(); 
      rdo->merge_BCID_sectorID().push_back(merge_BCID_sectorID);
      rdo->merge_valid_segmentSelector().push_back(segment_packet.ValidSegmentSelector()); 
      rdo->merge_nsw_segmentSelector().push_back(segment_packet.NSW_SegmentSelector()); 
      rdo->merge_LUT_choiceSelection().push_back(segment_packet.LUT_ChoiceSelection()); 
    }

    if (link->l1a_versionID() < 3){
       return StatusCode::SUCCESS;
    }
    const std::vector<STGTPMMPacket>& mm_packets =  link->mm_packet();
    for(uint i_packetIndex = 0; i_packetIndex<segment_packets.size(); i_packetIndex++){
      const STGTPMMPacket& mm_packet = mm_packets.at(i_packetIndex);
      uint8_t i_candidateIndex{0};
       for (const STGTPMMPacket::MMSegmentData& payload : mm_packet.Segments()){
        // we have at most 8 candidates in the output
        if(payload.dTheta == 16) {
           ++i_candidateIndex;
           continue; // ignore candidates that the trigger processor flags as invalid
        }
        uint32_t word{0}; // word containing all information about the candidate
        encodeSegmentProperty(MergedSegmentProperty::Monitor, payload.monitor ,word);
        encodeSegmentProperty(MergedSegmentProperty::Spare, payload.spare ,word);
        encodeSegmentProperty(MergedSegmentProperty::lowRes, payload.lowRes ,word);
        encodeSegmentProperty(MergedSegmentProperty::phiRes, payload.phiRes,word);
        encodeSegmentProperty(MergedSegmentProperty::dTheta, payload.dTheta,word);
        encodeSegmentProperty(MergedSegmentProperty::phiID, payload.phiID,word);
        encodeSegmentProperty(MergedSegmentProperty::rIndex, payload.rIndex ,word);
        ++i_candidateIndex;

        rdo->NSWTP_mm_segments().push_back(word);
      }
      // the first 12 bit are used for the bcid and the last 4 for sector ID
      rdo->NSWTP_mm_BCID().push_back(mm_packet.BCID());
    }

    }
    return StatusCode::SUCCESS;
  
}
}  // namespace Muon
