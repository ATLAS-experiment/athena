/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
using STGTPStripPacket = nsw::STGTPStripPacket;
using namespace nsw::STGTPMMData;
using namespace nsw::STGTPStrips;
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

  if (nsw_trigger_decoder.has_error()) {
    ATH_MSG_DEBUG("NSW sTGC TP Common Decoder found exceptions while reading this STGL1A fragment from " + std::to_string(fragment.rob_source_id()) + ". Skipping. Error id: "+std::to_string(nsw_trigger_decoder.error_id()));
    return StatusCode::SUCCESS;
  }

  if (nsw_trigger_decoder.get_elinks().size()==1) {
    if (static_cast<Muon::nsw::NSWTriggerSTGL1AElink*>(nsw_trigger_decoder.get_elinks()[0].get())->l1a_versionID() >= 3){
      ATH_MSG_DEBUG("NSW sTGC TP Common Decoder found only one elink in output but inconsistent L1A version: something off with this fragment. Skipping.");
      return StatusCode::SUCCESS;
    }
  } else if (nsw_trigger_decoder.get_elinks().size()!=3 && nsw_trigger_decoder.get_elinks().size()!=5) {
    // this is a severe requirement: a single elink missing would imply the whole event is problematic 
    ATH_MSG_DEBUG("NSW sTGC TP Common Decoder didn't give 3 or 5 elinks in output for: something off with this fragment. Skipping.");
    return StatusCode::SUCCESS;
  }

  bool consistent = true;
  const auto l0 = static_cast<Muon::nsw::NSWTriggerSTGL1AElink*>(nsw_trigger_decoder.get_elinks()[0].get());
  for(const auto& baseLink: nsw_trigger_decoder.get_elinks()) {
    const auto l = static_cast<Muon::nsw::NSWTriggerSTGL1AElink*>(baseLink.get());
    if (l0->head_sectID() != l->head_sectID()) {consistent = false; break;}
    if (l0->L1ID() != l->L1ID()) {consistent = false; break;}
    if (l0->l1a_versionID() != l->l1a_versionID()) {consistent = false; break;}
    if (l0->l1a_req_BCID() != l->l1a_req_BCID()) {consistent = false; break;}
  }
  if (!consistent) {
    ATH_MSG_WARNING("NSW sTGC TP Common Decoder found inconsistent header parameters in the elinks: something off with this fragment. Skipping.");
    return StatusCode::SUCCESS;
  }

  xAOD::NSWTPRDO* rdo = new xAOD::NSWTPRDO();
  rdoContainer.push_back(rdo);
 
  for(const auto& baseLink: nsw_trigger_decoder.get_elinks()){
    /// Create the new trigger processor RDO
    const auto link = static_cast<Muon::nsw::NSWTriggerSTGL1AElink*>(baseLink.get());
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
        if(pad_packet.BandID(i_candidateIndex) == Muon::nsw::STGTPPad::pad_stream_invalid_band_id) continue; // ignore candidates that the trigger processor flags as invalid
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
        if(payload.dTheta == Muon::nsw::STGTPSegments::merge_stream_invalid_dTheta) {
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
    for(uint i_packetIndex = 0; i_packetIndex<mm_packets.size(); i_packetIndex++){
      const STGTPMMPacket& mm_packet = mm_packets.at(i_packetIndex);
      uint8_t i_candidateIndex{0};
       for (const STGTPMMPacket::MMSegmentData& payload : mm_packet.Segments()){
        // we have at most 8 candidates in the output - non valid segments marked with a bit, are already skipped
        if(payload.dTheta == Muon::nsw::STGTPMMData::mm_stream_invalid_dTheta) {
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
        
        rdo->NSWTP_mm_segments().push_back(word);
        rdo->NSWTP_mm_candidateNumber().push_back(candidateNumber);
      }
      // the first 12 bit are used for the bcid and the last 4 for sector ID
      rdo->NSWTP_mm_BCID().push_back(mm_packet.BCID());
    }

    // Process strip packets (v3+ only)
    const std::vector<STGTPStripPacket>& strip_packets = link->strip_packet();
    for(uint i_packetIndex = 0; i_packetIndex < strip_packets.size(); i_packetIndex++){
      const STGTPStripPacket& strip_packet = strip_packets.at(i_packetIndex);
      
      // Store all 112 strip ADC values as bands_charge
      for(std::size_t i_strip = 0; i_strip < num_strips; ++i_strip) {
        rdo->NSWTP_strip_bands_charge().push_back(static_cast<uint8_t>(strip_packet.Strip(i_strip)));
        // Calculate layer: each 14 strips come from a layer in order
        rdo->NSWTP_strip_bands_layer().push_back(static_cast<uint8_t>(i_strip / 14));
        rdo->NSWTP_strip_bands_phiID().push_back(static_cast<uint8_t>(strip_packet.PhiIdValue()));
        rdo->NSWTP_strip_bands_bandID().push_back(static_cast<uint8_t>(strip_packet.BandId()));
        rdo->NSWTP_strip_bands_BCID().push_back(static_cast<uint16_t>(strip_packet.BCID()));
        // HLbit not available yet - leave empty or set to 0
        rdo->NSWTP_strip_bands_HLbit().push_back(0);
      }
      
      // Store all 8 offset values as centroids_offset
      for(std::size_t i_offset = 0; i_offset < num_offsets; ++i_offset) {
        rdo->NSWTP_strip_centroids_offset().push_back(strip_packet.Offset(i_offset));
        // Set corresponding layer info for offsets
        rdo->NSWTP_strip_centroids_layer().push_back(static_cast<uint32_t>(i_offset));
        rdo->NSWTP_strip_centroids_phiID().push_back(strip_packet.PhiIdValue());
        rdo->NSWTP_strip_centroids_bandID().push_back(strip_packet.BandId());
        // loc not available yet - leave empty or set to 0
        rdo->NSWTP_strip_centroids_loc().push_back(0);
      }
      
      // BBbit not available yet - leave empty for now
    }

  }
  return StatusCode::SUCCESS;
  
}
}  // namespace Muon
