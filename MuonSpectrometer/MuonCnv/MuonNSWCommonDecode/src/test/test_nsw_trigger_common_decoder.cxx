/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// STL include files

#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <math.h>
#include <regex>
#include <span>

// TDAQ include files

#include "ers/SampleIssues.h"
#include "EventStorage/pickDataReader.h"
#include "eformat/eformat.h"
#include "eformat/write/eformat.h"

#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMMMonElink.h"
#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/MMARTPacket.h"
#include "MuonNSWCommonDecode/MMTrigPacket.h"
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"
#include "MuonNSWCommonDecode/STGTPPackets.h"

#include <TFile.h>
#include <TTree.h>

#include "test_nsw_trigger_common_decoder_aux.h"

void test_nsw_trigger_common_decoder_help (char *progname) {
  std::cout << "Usage: " << progname
	    << " [-h] [-n events] [-p] [-v [-v [-v]]] [-d MML1A,MMMon,PadL1A,STGL1A] [-t] infile1, infile2, ..." << std::endl;
  std::cout << std::endl;
  std::cout << "\t\t[-h] Print this message and exit" << std::endl;
  std::cout << "\t\t[-n events] Maximum number of events to read (default = all)" << std::endl;
  std::cout << "\t\t[-p] Only print raw fragments" << std::endl;
  std::cout << "\t\tMultiple [-v] options increase printout detail level" << std::endl;
  std::cout << "\t\t[-d MML1A,MMMon,PadL1A,STGL1A] Elink type; comma separated list of elink types to be decoded" << std::endl;
  std::cout << "\t\t[-t] Input is from netio; if -t is set, then -d accepts only one elink type" << std::endl;

}

int test_nsw_trigger_common_decoder_opt (int argc, char **argv, Params& params) {
  
  //reading options
  for (int i = 1; i < argc; ++i){
    if (argv[i][0] == '-'){
      switch (argv[i][1]){
      case 'h':
        test_nsw_trigger_common_decoder_help (argv[0]);
        return 1;
      case 'n':
	params.max_events = static_cast <uint32_t> (strtol(argv[++i], nullptr, 10));
	break;
      case 'p':
	params.print_only = true;
	break;
      case 'v':
	++params.printout_level;
	break;
      case 'd': {
	std::stringstream ss(argv[++i]);
	std::string tmp;
	while (std::getline(ss, tmp, ',')) {
	  params.elink_types.push_back(tmp);
	}
	break;
      }
      case 't':
	params.is_netio = true;
	break;
      default:
	test_nsw_trigger_common_decoder_help (argv[0]);
	return 1;
      }
    } else {
      std::string data_file_name (argv[i]);
      params.file_names.push_back (data_file_name);
    }
  }
  
  //checking options
  if (params.file_names.size () == 0) {
    test_nsw_trigger_common_decoder_help (argv[0]);
    std::cout << "\t\tNo input files provided" << std::endl;
    return 2;
  }

  if (params.is_netio && params.elink_types.size()>1 ) {
    std::cout << "Cannot have multiple elink types for netio input!" << std::endl;
    return 2;
  }

  for (const auto& elink_type: params.elink_types){
    if (elink_type != "" &&
	elink_type != "MML1A" &&
	elink_type != "MMMon" &&
	elink_type != "PadL1A" &&
	elink_type != "STGL1A" ) {
      std::cout << "\n\tNo valid elink type provided: " << elink_type << std::endl;
      return 2;
    }
  }

  return 0;
}

int test_nsw_trigger_common_decoder_end (Statistics &statistics)
{
  statistics.avg_decoding_time = statistics.total_decoding_time/statistics.nevents;
  statistics.avg_robs_per_event/=statistics.nevents;
  statistics.avg_robs_wErrors/=statistics.nevents;


  std::cout << "Total event read                                          = " << statistics.nevents << std::endl;
  std::cout << "Total frags not readable (all found, not just NSWTrigger) = " << statistics.nfrags_not_readable << std::endl;
  std::cout << "Total requested frags with decoding issues      = " << statistics.nfrags_not_decoded << std::endl;
  std::cout << "Total requested frags with ROB errors           = " << statistics.nfrags_sus_rob_stat << std::endl;
  std::cout << "Total elinks with felix errors                  = " << statistics.nlinks_sus_felix_stat << std::endl;
  std::cout << "Total decoding-only time, including fails (ms)  = " << statistics.total_decoding_time << std::endl;
  std::cout << "Aver. decoding time (ms)                        = " << statistics.avg_decoding_time << std::endl;
  std::cout << "Aver. requested ROBs per event                  = " << statistics.avg_robs_per_event << std::endl;
  std::cout << "Aver. requested ROBs with errors per event      = " << statistics.avg_robs_wErrors << std::endl;

  std::cout << "NOTES" << std::endl;
  std::cout << "If using -p, only 'Total event read' and 'Total frags not readable' are meaningful" << std::endl;
  std::cout << "frags = ROBFragments; so potentially multiple elinks:" << std::endl;
  std::cout << "1 frag not decoded it means that NSWTriggerCommonDecoder has an error, so at least 1 elink packet is problematic" << std::endl;
  std::cout << "'Total elinks with felix errors' is in the loop over the events, over the frags, over the links" << std::endl;
  return 0;
}

int test_nsw_trigger_common_decoder_fragment (const eformat::read::ROBFragment &r, std::string &robType, outBranches &data, Params &params, Statistics &statistics) {

  //re-computing sid, s, m for ntuple filling only
  uint32_t sid = r.rob_source_id ();
  eformat::helper::SourceIdentifier source_id (sid);
  eformat::SubDetector s = source_id.subdetector_id ();
  uint16_t m = source_id.module_id ();
  
  const uint32_t *bs = r.rod_data ();
  
  // Print out raw fragment
  if (params.print_only || params.printout_level > 2) {
    std::cout << "ROD Fragment sizes in words:" << std::endl;
    std::cout << "ROD Total:       " << r.rod_fragment_size_word () << std::endl;
    std::cout << "ROD Header:      " << r.rod_header_size_word () << std::endl;
    std::cout << "ROD Trailer:     " << r.rod_trailer_size_word () << std::endl;
    std::cout << "ROD L1 ID (hex): " << std::hex << r.rod_lvl1_id () << std::dec << std::endl;
    std::cout << "ROD Data words:  " << r.rod_ndata () << std::endl;
    
    std::cout << "Printing raw data (ignoring any internal structure)" << std::endl;
    std::cout << std::hex;
    for (unsigned int i = 0; i < r.rod_ndata (); ++i) {
      std::cout << " " << std::setfill('0') << std::setw(8) << bs[i];
      if (i % 4 == 3) std::cout << std::endl;
    }
    std::cout << std::dec;
    std::cout << std::endl;
  }

  if (!params.print_only) {

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now ();
    Muon::nsw::NSWTriggerCommonDecoder nsw_trigger_decoder (r, robType);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
    unsigned int time_elapsed = std::chrono::duration_cast <std::chrono::microseconds> (end - begin).count ();
    float time_elapsed_ms = static_cast <float> (time_elapsed) / 1000;
    
    if (params.printout_level > 1) {
      std::cout << "Time for decoding this event (ms): " << time_elapsed_ms << std::endl;
      std::cout << std::endl;
    }
    statistics.total_decoding_time += time_elapsed_ms;
    
    //reporting errors with this frag 
    if (nsw_trigger_decoder.has_error()) {
      ++statistics.nfrags_not_decoded;
      if (params.printout_level > 0) {
        std::cout << "Errors while decoding this fragment: error id " << nsw_trigger_decoder.error_id() << std::endl;
        std::cout << std::endl;
      }
      return 1;
    }

    if (robType == "PadL1A") {
      for (const auto& baseLink : nsw_trigger_decoder.get_elinks()) {
        const auto link = std::dynamic_pointer_cast<Muon::nsw::NSWPadTriggerL1a>(baseLink);
        if (link->status()) {
          ++statistics.nlinks_sus_felix_stat;
        }
        data.b_PadL1A_ROB_sourceID.push_back(r.rob_source_id());
        data.b_PadL1A_ROB_status.emplace_back(r.status(), r.status()+r.nstatus());
        data.b_PadL1A_ROD_sourceID.push_back(sid);
        data.b_PadL1A_ROD_subdetID.push_back(s);
        data.b_PadL1A_ROD_moduleID.push_back(m);
        data.b_PadL1A_ROD_L1ID.push_back(r.rod_lvl1_id());
        data.b_PadL1A_ROD_BCID.push_back(r.rod_bc_id());
        data.b_PadL1A_ROD_n_words.push_back(r.rod_ndata());
        data.b_PadL1A_ROD_status.emplace_back(r.rod_status(), r.rod_status()+r.rod_nstatus());
        data.b_PadL1A_flags.push_back(link->getFlags());
        data.b_PadL1A_ec.push_back(link->getEc());
        data.b_PadL1A_fragid.push_back(link->getFragid());
        data.b_PadL1A_secid.push_back(link->getSecid());
        data.b_PadL1A_spare.push_back(link->getSpare());
        data.b_PadL1A_orbit.push_back(link->getOrbit());
        data.b_PadL1A_bcid.push_back(link->getBcid());
        data.b_PadL1A_l1id.push_back(link->getL1id());
        data.b_PadL1A_hit_n.push_back(link->getNumberOfHits());
        data.b_PadL1A_pfeb_n.push_back(link->getNumberOfPfebs());
        data.b_PadL1A_trigger_n.push_back(link->getNumberOfTriggers());
        data.b_PadL1A_bcid_n.push_back(link->getNumberOfBcids());
        data.b_PadL1A_hit_relbcid.push_back(link->getHitRelBcids());
        data.b_PadL1A_hit_pfeb.push_back(link->getHitPfebs());
        data.b_PadL1A_hit_tdschannel.push_back(link->getHitTdsChannels());
        data.b_PadL1A_hit_vmmchannel.push_back(link->getHitVmmChannels());
        data.b_PadL1A_hit_vmm.push_back(link->getHitVmms());
        data.b_PadL1A_hit_padchannel.push_back(link->getHitPadChannels());
        data.b_PadL1A_pfeb_addr.push_back(link->getPfebAddresses());
        data.b_PadL1A_pfeb_nchan.push_back(link->getPfebNChannels());
        data.b_PadL1A_pfeb_disconnected.push_back(link->getPfebDisconnecteds());
        data.b_PadL1A_trigger_bandid.push_back(link->getTriggerBandIds());
        data.b_PadL1A_trigger_phiid.push_back(link->getTriggerPhiIds());
        data.b_PadL1A_trigger_relbcid.push_back(link->getTriggerRelBcids());
        data.b_PadL1A_bcid_rel.push_back(link->getBcidRels());
        data.b_PadL1A_bcid_status.push_back(link->getBcidStatuses());
        data.b_PadL1A_bcid_multzero.push_back(link->getBcidMultZeros());
        data.b_PadL1A_CRC.push_back(link->getCRC());
        data.b_PadL1A_CRC_ok.push_back(link->isCRCValid());
      }
    }
    if (robType == "MML1A") {
      for (const auto& baseLink : nsw_trigger_decoder.get_elinks()) {
        const auto link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMML1AElink>(baseLink);
        if (link->status()) {
          ++statistics.nlinks_sus_felix_stat;
        }
        data.b_MML1A_ROB_sourceID.push_back(r.rob_source_id());
        data.b_MML1A_ROB_status.emplace_back(r.status(), r.status()+r.nstatus());
        data.b_MML1A_ROD_sourceID.push_back(sid);
        data.b_MML1A_ROD_subdetID.push_back(s);
        data.b_MML1A_ROD_moduleID.push_back(m);
        data.b_MML1A_ROD_L1ID.push_back(r.rod_lvl1_id());
        data.b_MML1A_ROD_BCID.push_back(r.rod_bc_id());
        data.b_MML1A_ROD_n_words.push_back(r.rod_ndata());
        data.b_MML1A_ROD_status.emplace_back(r.rod_status(), r.rod_status()+r.rod_nstatus());
        data.b_MML1A_link_id.push_back(link->elinkWord());
        data.b_MML1A_link_status.push_back(link->status());
        data.b_MML1A_head_fragID.push_back(link->head_fragID());
        data.b_MML1A_head_sectID.push_back(link->head_sectID());
        data.b_MML1A_head_EC.push_back(link->head_EC());
        data.b_MML1A_head_flags.push_back(link->head_flags());
        data.b_MML1A_head_BCID.push_back(link->head_BCID());
        data.b_MML1A_head_orbit.push_back(link->head_orbit());
        data.b_MML1A_head_spare.push_back(link->head_spare());
        data.b_MML1A_L1ID.push_back(link->L1ID());
        data.b_MML1A_l1a_versionID.push_back(link->l1a_versionID());
        data.b_MML1A_l1a_local_req_BCID.push_back(link->l1a_local_req_BCID());
        data.b_MML1A_l1a_local_rel_BCID.push_back(link->l1a_local_rel_BCID());
        data.b_MML1A_l1a_open_BCID.push_back(link->l1a_open_BCID());
        data.b_MML1A_l1a_req_BCID.push_back(link->l1a_req_BCID());
        data.b_MML1A_l1a_close_BCID.push_back(link->l1a_close_BCID());
        data.b_MML1A_l1a_timeout.push_back(link->l1a_timeout());
        data.b_MML1A_l1a_open_BCID_offset.push_back(link->l1a_open_BCID_offset());
        data.b_MML1A_l1a_req_BCID_offset.push_back(link->l1a_req_BCID_offset());
        data.b_MML1A_l1a_close_BCID_offset.push_back(link->l1a_close_BCID_offset());
        data.b_MML1A_l1a_timeout_config.push_back(link->l1a_timeout_config());
        data.b_MML1A_l1a_busy_thr.push_back(link->l1a_busy_thr());
        data.b_MML1A_l1a_engine_snapshot.push_back(link->l1a_engine_snapshot());
        data.b_MML1A_l1a_link_const.push_back(link->l1a_link_const());
        data.b_MML1A_l1a_padding.push_back(link->l1a_padding());
        data.b_MML1A_stream_head_nbits.push_back(link->stream_head_nbits());
        data.b_MML1A_stream_head_nwords.push_back(link->stream_head_nwords());
        data.b_MML1A_stream_head_fifo_size.push_back(link->stream_head_fifo_size());
        data.b_MML1A_stream_head_streamID.push_back(link->stream_head_streamID());

        const std::vector<std::shared_ptr<Muon::nsw::MMARTPacket>>& arts = link->art_packets();
        std::vector<uint32_t> tmp_art_BCIDs;
        std::vector<uint32_t> tmp_art_pipeIDs;
        std::vector<uint32_t> tmp_art_fiberIDs;
        std::vector<uint32_t> tmp_art_layers;
        std::vector<uint32_t> tmp_art_channels;
        for (const auto& art : arts) {
          for (const auto& c : art->channels()) {
            // so that there's a timestamp per channel
            tmp_art_layers.push_back(c.first);
            tmp_art_channels.push_back(c.second);
            tmp_art_BCIDs.push_back(art->art_BCID());
            tmp_art_pipeIDs.push_back(art->art_pipeID());
            tmp_art_fiberIDs.push_back(art->art_fiberID());
          }
        }
        data.b_MML1A_art_BCID.push_back(tmp_art_BCIDs);
        data.b_MML1A_art_pipeID.push_back(tmp_art_pipeIDs);
        data.b_MML1A_art_fiberID.push_back(tmp_art_fiberIDs);
        data.b_MML1A_art_layers.push_back(tmp_art_layers);
        data.b_MML1A_art_channels.push_back(tmp_art_channels);

        const std::vector<std::shared_ptr<Muon::nsw::MMTrigPacket>>& trigs = link->trig_packets();
        std::vector<uint32_t> tmp_trig_globalX;
        std::vector<uint32_t> tmp_trig_globalU;
        std::vector<uint32_t> tmp_trig_BCID;
        std::vector<uint32_t> tmp_trig_dTheta;
        std::vector<uint32_t> tmp_trig_phiBin;
        std::vector<uint32_t> tmp_trig_rBin;
        for (const auto& trig : trigs) {
          tmp_trig_globalX.push_back(trig->trig_globalX());
          tmp_trig_globalU.push_back(trig->trig_globalU());
          tmp_trig_BCID.push_back(trig->trig_BCID());
          tmp_trig_dTheta.push_back(trig->trig_dTheta());
          tmp_trig_phiBin.push_back(trig->trig_phiBin());
          tmp_trig_rBin.push_back(trig->trig_rBin());
        }
        data.b_MML1A_trig_globalX.push_back(tmp_trig_globalX);
        data.b_MML1A_trig_globalU.push_back(tmp_trig_globalU);
        data.b_MML1A_trig_BCID.push_back(tmp_trig_BCID);
        data.b_MML1A_trig_dTheta.push_back(tmp_trig_dTheta);
        data.b_MML1A_trig_phiBin.push_back(tmp_trig_phiBin);
        data.b_MML1A_trig_rBin.push_back(tmp_trig_rBin);

        data.b_MML1A_trailer_CRC.push_back(link->trailer_CRC());
        data.b_MML1A_CRC_ok.push_back(link->is_crc_ok());
      }
    }
    if (robType == "MMMon") {
      uint bs_pointer = 0;
      for (const auto& baseLink : nsw_trigger_decoder.get_elinks()) {
        const auto link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMMMonElink>(baseLink);
        if (link->status()) {
          ++statistics.nlinks_sus_felix_stat;
        }
        data.b_MMMon_ROB_sourceID.push_back(r.rob_source_id());
        data.b_MMMon_ROB_status.emplace_back(r.status(), r.status()+r.nstatus());
        data.b_MMMon_ROD_sourceID.push_back(sid);
        data.b_MMMon_ROD_subdetID.push_back(s);
        data.b_MMMon_ROD_moduleID.push_back(m);
        data.b_MMMon_ROD_L1ID.push_back(r.rod_lvl1_id());
        data.b_MMMon_ROD_BCID.push_back(r.rod_bc_id());
        data.b_MMMon_ROD_n_words.push_back(r.rod_ndata());
        data.b_MMMon_ROD_status.emplace_back(r.rod_status(), r.rod_status()+r.rod_nstatus());
        data.b_MMMon_link_id.push_back(link->elinkWord());
        data.b_MMMon_link_status.push_back(link->status());
        data.b_MMMon_head_fragID.push_back(link->head_fragID());
        data.b_MMMon_head_sectID.push_back(link->head_sectID());
        data.b_MMMon_head_EC.push_back(link->head_EC());
        data.b_MMMon_head_flags.push_back(link->head_flags());
        data.b_MMMon_head_BCID.push_back(link->head_BCID());
        data.b_MMMon_head_orbit.push_back(link->head_orbit());
        data.b_MMMon_head_spare.push_back(link->head_spare());
        data.b_MMMon_L1ID.push_back(link->L1ID());
        data.b_MMMon_head_coincBCID.push_back(link->head_coincBCID());
        data.b_MMMon_head_regionCount.push_back(link->head_regionCount());
        data.b_MMMon_head_coincRegion.push_back(link->head_coincRegion());
        data.b_MMMon_head_reserved.push_back(link->head_reserved());
        data.b_MMMon_finder_streamID.push_back(link->finder_streamID());
        data.b_MMMon_finder_regionCount.push_back(link->finder_regionCount());
        data.b_MMMon_finder_triggerID.push_back(link->finder_triggerID());
        data.b_MMMon_finder_V1.push_back(link->finder_V1());
        data.b_MMMon_finder_V0.push_back(link->finder_V0());
        data.b_MMMon_finder_U1.push_back(link->finder_U1());
        data.b_MMMon_finder_U0.push_back(link->finder_U0());
        data.b_MMMon_finder_X3.push_back(link->finder_X3());
        data.b_MMMon_finder_X2.push_back(link->finder_X2());
        data.b_MMMon_finder_X1.push_back(link->finder_X1());
        data.b_MMMon_finder_X0.push_back(link->finder_X0());
        data.b_MMMon_fitter_streamID.push_back(link->fitter_streamID());
        data.b_MMMon_fitter_regionCount.push_back(link->fitter_regionCount());
        data.b_MMMon_fitter_triggerID.push_back(link->fitter_triggerID());
        data.b_MMMon_fitter_filler.push_back(link->fitter_filler());
        data.b_MMMon_fitter_mxG.push_back(link->fitter_mxG());
        data.b_MMMon_fitter_muG.push_back(link->fitter_muG());
        data.b_MMMon_fitter_mvG.push_back(link->fitter_mvG());
        data.b_MMMon_fitter_mxL.push_back(link->fitter_mxL());
        data.b_MMMon_fitter_mx_ROI.push_back(link->fitter_mx_ROI());
        data.b_MMMon_fitter_dTheta.push_back(link->fitter_dTheta());
        data.b_MMMon_fitter_zero.push_back(link->fitter_zero());
        data.b_MMMon_fitter_phiSign.push_back(link->fitter_phiSign());
        data.b_MMMon_fitter_phiBin.push_back(link->fitter_phiBin());
        data.b_MMMon_fitter_rBin.push_back(link->fitter_rBin());
        data.b_MMMon_trailer_CRC.push_back(link->trailer_CRC());
        CRCMonHelper CRCMon(0xffff, 0x11021);
        std::span<const uint32_t> load{bs + bs_pointer + 2, link->nwordsFlx() - 2};
        bs_pointer += link->nwordsFlx();
        data.b_MMMon_CRC_ok.push_back((CRCMon.getCRC(load) == link->trailer_CRC()));
      }
    }
    if (robType == "STGL1A") {
      // resize vectors
      unsigned int n_elinks = nsw_trigger_decoder.get_elinks().size();
      data.b_STGL1A_pad_BCID.resize(n_elinks);
      data.b_STGL1A_pad_header.resize(n_elinks);
      data.b_STGL1A_pad_bandID_0.resize(n_elinks);
      data.b_STGL1A_pad_bandID_1.resize(n_elinks);
      data.b_STGL1A_pad_bandID_2.resize(n_elinks);
      data.b_STGL1A_pad_bandID_3.resize(n_elinks);
      data.b_STGL1A_pad_phiID_0.resize(n_elinks);
      data.b_STGL1A_pad_phiID_1.resize(n_elinks);
      data.b_STGL1A_pad_phiID_2.resize(n_elinks);
      data.b_STGL1A_pad_phiID_3.resize(n_elinks);
      data.b_STGL1A_pad_coincidence_wedge.resize(n_elinks);
      data.b_STGL1A_pad_idleFlag.resize(n_elinks);
      data.b_STGL1A_merge_LUT_choiceSelection.resize(n_elinks);
      data.b_STGL1A_merge_nsw_segmentSelector.resize(n_elinks);
      data.b_STGL1A_merge_valid_segmentSelector.resize(n_elinks);

      const auto resize_segment = [&n_elinks](auto& segments) {
        for (auto& segment : segments) {
          segment.resize(n_elinks);
        }
      };

      resize_segment(data.b_STGL1A_merge_monitor_segments);
      resize_segment(data.b_STGL1A_merge_spare_segments);
      resize_segment(data.b_STGL1A_merge_lowRes_segments);
      resize_segment(data.b_STGL1A_merge_phiRes_segments);
      resize_segment(data.b_STGL1A_merge_dTheta_segments);
      resize_segment(data.b_STGL1A_merge_phiID_segments);
      resize_segment(data.b_STGL1A_merge_RIndex_segments);

      data.b_STGL1A_merge_BCID.resize(n_elinks);
      data.b_STGL1A_merge_header.resize(n_elinks);
      data.b_STGL1A_merge_sectorID.resize(n_elinks);

      resize_segment(data.b_STGL1A_mm_monitor_segments);
      resize_segment(data.b_STGL1A_mm_spare_segments);
      resize_segment(data.b_STGL1A_mm_lowRes_segments);
      resize_segment(data.b_STGL1A_mm_phiRes_segments);
      resize_segment(data.b_STGL1A_mm_dTheta_segments);
      resize_segment(data.b_STGL1A_mm_phiID_segments);
      resize_segment(data.b_STGL1A_mm_RIndex_segments);

      data.b_STGL1A_mm_BCID.resize(n_elinks);
      data.b_STGL1A_mm_header.resize(n_elinks);
      uint i_pad   = 0;
      uint i_mm    = 0;
      uint i_merge = 0;
      uint i_elink = 0;
      for (const auto& baseLink : nsw_trigger_decoder.get_elinks()) {
        const auto link = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerSTGL1AElink>(baseLink);
        if (link->status()) {
          ++statistics.nlinks_sus_felix_stat;
        }
        data.b_STGL1A_ROB_sourceID.push_back(r.rob_source_id());
        data.b_STGL1A_ROB_status.emplace_back(r.status(), r.status()+r.nstatus());
        data.b_STGL1A_ROD_sourceID.push_back(sid);
        data.b_STGL1A_ROD_subdetID.push_back(s);
        data.b_STGL1A_ROD_moduleID.push_back(m);
        data.b_STGL1A_ROD_L1ID.push_back(r.rod_lvl1_id());
        data.b_STGL1A_ROD_BCID.push_back(r.rod_bc_id());
        data.b_STGL1A_ROD_n_words.push_back(r.rod_ndata());
        data.b_STGL1A_ROD_status.emplace_back(r.rod_status(), r.rod_status()+r.rod_nstatus());
        data.b_STGL1A_head_fragID.push_back(link->head_fragID());
        data.b_STGL1A_head_sectID.push_back(link->head_sectID());
        data.b_STGL1A_head_EC.push_back(link->head_EC());
        data.b_STGL1A_head_flags.push_back(link->head_flags());
        data.b_STGL1A_head_BCID.push_back(link->head_BCID());
        data.b_STGL1A_head_orbit.push_back(link->head_orbit());
        data.b_STGL1A_head_spare.push_back(link->head_spare());
        data.b_STGL1A_L1ID.push_back(link->L1ID());
        data.b_STGL1A_l1a_versionID.push_back(link->l1a_versionID());
        data.b_STGL1A_l1a_local_req_BCID.push_back(link->l1a_local_req_BCID());
        data.b_STGL1A_l1a_local_rel_BCID.push_back(link->l1a_local_rel_BCID());
        data.b_STGL1A_l1a_open_BCID.push_back(link->l1a_open_BCID());
        data.b_STGL1A_l1a_req_BCID.push_back(link->l1a_req_BCID());
        data.b_STGL1A_l1a_close_BCID.push_back(link->l1a_close_BCID());
        data.b_STGL1A_l1a_timeout.push_back(link->l1a_timeout());
        data.b_STGL1A_l1a_open_BCID_offset.push_back(link->l1a_open_BCID_offset());
        data.b_STGL1A_l1a_req_BCID_offset.push_back(link->l1a_req_BCID_offset());
        data.b_STGL1A_l1a_close_BCID_offset.push_back(link->l1a_close_BCID_offset());
        data.b_STGL1A_l1a_timeout_config.push_back(link->l1a_timeout_config());
        data.b_STGL1A_l1a_busy_thr.push_back(link->l1a_busy_thr());
        data.b_STGL1A_l1a_engine_snapshot.push_back(link->l1a_engine_snapshot());
        data.b_STGL1A_l1a_link_const.push_back(link->l1a_link_const());
        data.b_STGL1A_l1a_padding.push_back(link->l1a_padding());
        data.b_STGL1A_stream_head_nbits.push_back(link->stream_head_nbits());
        data.b_STGL1A_stream_head_nwords.push_back(link->stream_head_nwords());
        data.b_STGL1A_stream_head_fifo_size.push_back(link->stream_head_fifo_size());
        data.b_STGL1A_stream_head_streamID.push_back(link->stream_head_streamID());
        
        // pad block
        const auto& pad_packets = link->pad_packets();
        for (auto packet : pad_packets) {
          data.b_STGL1A_pad_BCID[i_pad].push_back(packet.BCID());
          data.b_STGL1A_pad_header[i_pad].push_back(i_elink);
          data.b_STGL1A_pad_bandID_0[i_pad].push_back(packet.BandID(0));
          data.b_STGL1A_pad_bandID_1[i_pad].push_back(packet.BandID(1));
          data.b_STGL1A_pad_bandID_2[i_pad].push_back(packet.BandID(2));
          data.b_STGL1A_pad_bandID_3[i_pad].push_back(packet.BandID(3));
          data.b_STGL1A_pad_phiID_0[i_pad].push_back(packet.PhiID(0));
          data.b_STGL1A_pad_phiID_1[i_pad].push_back(packet.PhiID(1));
          data.b_STGL1A_pad_phiID_2[i_pad].push_back(packet.PhiID(2));
          data.b_STGL1A_pad_coincidence_wedge[i_pad].push_back(packet.CoincidenceWedge());
          data.b_STGL1A_pad_phiID_3[i_pad].push_back(packet.PhiID(3));
          data.b_STGL1A_pad_idleFlag[i_pad].push_back(packet.PadIdleFlag());
        }
        
        const auto& segment_packets = link->segment_packet();
        for (auto packet : segment_packets) {
          data.b_STGL1A_merge_LUT_choiceSelection[i_merge].push_back(packet.LUT_ChoiceSelection());
          data.b_STGL1A_merge_nsw_segmentSelector[i_merge].push_back(packet.NSW_SegmentSelector());
          data.b_STGL1A_merge_valid_segmentSelector[i_merge].push_back(packet.ValidSegmentSelector());

          for (std::size_t i_seg = 0; i_seg < Muon::nsw::STGTPSegments::num_segments; ++i_seg) {
            data.b_STGL1A_merge_monitor_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).monitor);
            data.b_STGL1A_merge_spare_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).spare);
            data.b_STGL1A_merge_lowRes_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).lowRes);
            data.b_STGL1A_merge_phiRes_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).phiRes);
            data.b_STGL1A_merge_dTheta_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).dTheta);
            data.b_STGL1A_merge_phiID_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).phiID);
            data.b_STGL1A_merge_RIndex_segments.at(i_seg).at(i_merge).push_back(packet.Segment(i_seg).rIndex);
          }

          data.b_STGL1A_merge_BCID[i_merge].push_back(packet.BCID());
          data.b_STGL1A_merge_header[i_merge].push_back(i_elink);
          data.b_STGL1A_merge_header[i_merge].push_back(i_elink);
          data.b_STGL1A_merge_sectorID[i_merge].push_back(packet.SectorID());

        }  // end of merge packets

        const auto& mm_packets = link->mm_packet();
        for (auto packet : mm_packets) {
          for (std::size_t i_seg = 0; i_seg < Muon::nsw::STGTPMMData::num_mm; ++i_seg) {
            data.b_STGL1A_mm_monitor_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).monitor);
            data.b_STGL1A_mm_spare_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).spare);
            data.b_STGL1A_mm_lowRes_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).lowRes);
            data.b_STGL1A_mm_phiRes_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).phiRes);
            data.b_STGL1A_mm_dTheta_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).dTheta);
            data.b_STGL1A_mm_phiID_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).phiID);
            data.b_STGL1A_mm_RIndex_segments.at(i_seg).at(i_mm).push_back(packet.Segment(i_seg).rIndex);
          }
          data.b_STGL1A_mm_BCID[i_mm].push_back(packet.BCID());
          data.b_STGL1A_mm_header[i_mm].push_back(i_elink);
        }  // end of mm packets

        //// indeces /////
        if (pad_packets.size() > 0)
        {
           i_pad++;
        }
        if (segment_packets.size() > 0)
        {
           i_merge++;
        }
        if (mm_packets.size() > 0)
        {
           i_mm++;
        }
        i_elink++;
      }  // end of stgc elink loop
    }  // end of stgc l1a block

  }  // not print only

  return 0;

}

int test_nsw_trigger_common_decoder_event(eformat::read::FullEventFragment& f, outBranches& data, Params& params, Statistics& statistics) {

  int err;

  data.b_run_number = f.run_no();
  data.b_run_type = f.run_type();
  data.b_lumi_block = f.lumi_block();
  data.b_L1ID = f.lvl1_id();
  data.b_BCID = f.bc_id();
  data.b_BC_time_seconds = f.bc_time_seconds();
  data.b_BC_time_nanoseconds = f.bc_time_nanoseconds();

  std::vector<eformat::read::ROBFragment> robs;
  f.robs(robs);

  int n_nswtrigger_robs = 0;
  int n_requested_robs = 0;
  int n_requested_robs_wErrors = 0;

  for (const auto& r : robs) {
    // check fragment for errors
    try{ r.check (); }
    catch(...) { 
      std::cout << "Something wrong with current ROBFragment" << std::endl; 
      ++statistics.nfrags_not_readable; 
      continue;
    }

    bool is_nsw = false, is_mmg = false, is_stg = false;
    bool is_tp = false, is_pt = false, is_l1a = false, is_mon = false; //is_l1a and is_mon are actually reduntant
  
    uint32_t sid = r.rob_source_id ();
    eformat::helper::SourceIdentifier source_id (sid);
    eformat::SubDetector s = source_id.subdetector_id();
    uint16_t m = source_id.module_id ();

    if (s == eformat::MUON_MMEGA_ENDCAP_A_SIDE ||
        s == eformat::MUON_MMEGA_ENDCAP_C_SIDE) {
      is_nsw = is_mmg = true;
    }
    if (s == eformat::MUON_STGC_ENDCAP_A_SIDE ||
        s == eformat::MUON_STGC_ENDCAP_C_SIDE) {
      is_nsw = is_stg = true;
    }

    if      ((m & 0xf0) == 0x10) {is_tp = true; is_l1a = true;}
    else if ((m & 0xf0) == 0x20) {is_pt = true; is_l1a = true;}
    else if ((m & 0xf0) == 0x50) {is_tp = true; is_mon = true;}

    if (is_nsw && (is_pt || is_tp)) {
      ++n_nswtrigger_robs;
      if (params.printout_level > 0) {
        std::cout << "NSW Trigger fragment found; length: " << r.rod_ndata () << std::endl;
      }
      if (params.printout_level > 1) {
        std::cout << "ROB source ID       = 0x" << std::hex << sid << std::dec << std::endl;
        std::cout << "ROB subdet ID       = 0x" << std::hex << s   << std::dec << std::endl;
        std::cout << "ROB module ID       = 0x" << std::hex << m   << std::dec << std::endl;
        std::cout << "is_mmg              = " << (is_mmg?"Yes":"No") << std::endl;
        std::cout << "is_stg              = " << (is_stg?"Yes":"No") << std::endl;
        std::cout << "is_pt               = " << (is_pt?"Yes":"No") << std::endl;
        std::cout << "is_tp               = " << (is_tp?"Yes":"No") << std::endl;
        std::cout << "is_l1a              = " << (is_l1a?"Yes":"No") << std::endl;
        std::cout << "is_mon              = " << (is_mon?"Yes":"No") << std::endl;
      }

      std::string robType = std::string(is_pt?"Pad":(is_mmg?"MM":"STG")) + std::string(is_l1a?"L1A":(is_mon?"Mon":""));

      if ( std::find(params.elink_types.begin(), params.elink_types.end(), robType) == params.elink_types.end() ) {
        if (params.printout_level > 1) { 
          std::cout << "Not decoding this ROBFragment since it's not requested" << std::endl;
        }
      } else {
        ++n_requested_robs;
        if (r.status()[0] != 0) {
          ++statistics.nfrags_sus_rob_stat; //this is the swROD status, not what is added by the swROD plugin by us
        }
        err = test_nsw_trigger_common_decoder_fragment(r, robType, data, params, statistics);
        n_requested_robs_wErrors += err;
      }

    } else {
      if (params.printout_level > 1) {
        std::cout << "Not decoding this ROBFragment since it's not belonging NSW Trigger: sourceID " << std::hex << sid << std::dec << std::endl;
      }
    }
 
  }

  if (params.printout_level > 2) {
    std::cout << "Number of NSW Trigger ROB fragments found: " << n_nswtrigger_robs << std::endl;
    std::cout << "Number of requested NSW Trigger ROB fragments: " << n_requested_robs << std::endl;
    std::cout << "Number of requested NSW Trigger ROB fragments with errors: " << n_requested_robs_wErrors << std::endl;
  }

  statistics.avg_robs_per_event += n_requested_robs;
  statistics.avg_robs_wErrors += n_requested_robs_wErrors;

  if (n_requested_robs>0) {
    return 0; // all ok
  }

  // events with robs but all of them with errors will have empty branches

  return 1;
}

int test_nsw_trigger_common_decoder_loop_txt(Params& params, Statistics& statistics) {

  outBranches data;

  for (const std::string& filename : params.file_names) {

    TFile* outfile = nullptr;
    TTree* outtree = nullptr;
    // no need for a smart pointer for ttrees, given ROOT
    // behaviour (deleting ttree's when closing files)

    std::string data_file_name(filename);
    std::string out_file_name = data_file_name.substr(data_file_name.find_last_of("/\\") + 1) + ".decoded.root";

    std::cout << "Reading file " << data_file_name << std::endl;

    if (!params.print_only) {
      std::cout << "Saving here file " << out_file_name << std::endl;
      outfile = new TFile(out_file_name.c_str(), "recreate");
      outtree = new TTree("decoded_data", "decoded_data");
      test_nsw_trigger_common_decoder_init_tree(*outtree, data, params);
    }

    // building a dummy A01
    uint32_t dummy_sid = 0;
    if (params.elink_types[0] == "MML1A") {
      dummy_sid = (static_cast<uint32_t>(eformat::MUON_MMEGA_ENDCAP_A_SIDE) << 16) + 0x10;
    } else if (params.elink_types[0] == "STGL1A") {
      dummy_sid = (static_cast<uint32_t>(eformat::MUON_STGC_ENDCAP_A_SIDE) << 16) + 0x10;
    } else if (params.elink_types[0] == "PadL1A") {
      dummy_sid = (static_cast<uint32_t>(eformat::MUON_STGC_ENDCAP_A_SIDE) << 16) + 0x20;
    } else if (params.elink_types[0] == "MMMon") {
      dummy_sid = (static_cast<uint32_t>(eformat::MUON_MMEGA_ENDCAP_A_SIDE) << 16) + 0x50;
    }

    std::ifstream in_file(data_file_name);
    std::regex pattern("([[:xdigit:]]{2} )+[[:xdigit:]]{2}( )*");
    std::string line;

    while (in_file.peek() != EOF && (params.max_events == 0 || statistics.nevents < params.max_events)) {

      if (!(statistics.nevents % static_cast<int>(std::pow(10, static_cast<int>(4 - params.printout_level))))) {
        std::cout << "Processing event: " << statistics.nevents << std::endl;
      }

      // find next available packet
      do {
        std::getline(in_file, line);
      } while (!std::regex_match(line, pattern) && in_file.peek() != EOF);
      // rechecking in case it was EOF but last line was not a packet
      if (!std::regex_match(line, pattern)) {
        break;
      }

      // remove spaces
      line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
      // first byte in netio output is felix status flag
      uint16_t status = static_cast<uint32_t>(std::strtol(line.substr(0, 2).c_str(), nullptr, 16));
      line.erase(0, 2);  // removing mini felix header
      // length
      uint16_t length = line.size() / 2;  // number of bytes before zero padding

      // composing a real header
      std::stringstream tmpstr;

      // appending a fake elink status 
      tmpstr << std::setfill('0') << std::setw(4) << std::hex << status;
      // felix reporting how many 32b words, header included
      tmpstr << std::setfill('0') << std::setw(4) << std::hex << 2 + (1 + (length - 1) / 4);  
      // dummy elink id
      tmpstr << std::setfill('0') << std::setw(8) << std::hex << "CAFECAFE";  
      // zero padding to complete last 32b word, as the swROD would do
      tmpstr << std::left << std::setfill('0') << std::setw((1 + (line.size() - 1) / 8) * 8) << line;  

      line = tmpstr.str();

      // convert string into read::ROBFragment
      uint wordsSize = line.size() / 8;
      std::unique_ptr<uint32_t[]> words(new uint32_t[wordsSize]);
      for (uint i = 0; i < wordsSize; ++i) {
        words[i] = std::strtol(line.substr(i * 8, 8).c_str(), nullptr, 16);
      }
      // args: source_id, run_no, lvl1_id, bc_id, lvl1_type, detev_type, ndata, data, status_position
      eformat::write::ROBFragment writer = eformat::write::ROBFragment(dummy_sid, 0, 0, 0, 0, 0, wordsSize, words.get(), 0);
      
      std::unique_ptr<uint32_t[]> serialized(new uint32_t[writer.size_word()]);
      const eformat::write::node_t* top(writer.bind());
      eformat::write::copy(*top, serialized.get(), writer.size_word());
      eformat::helper::u32slice slice =
          eformat::helper::u32slice(serialized.get(), writer.size_word());
      eformat::read::ROBFragment r(slice);

      try{ r.check (); }
      catch(...) { 
        std::cout << "Something wrong with current ROBFragment" << std::endl; 
        ++statistics.nfrags_not_readable; 
        continue;
      }

      data = outBranches();

      int report = 0;
      if ((report = test_nsw_trigger_common_decoder_fragment(r, params.elink_types[0], data, params, statistics))) {
        std::cout << "Cannot decode properly event " << statistics.nevents << "; skipping it! \n" << std::endl;
        ++statistics.nevents;
        continue; 
      }
      ++statistics.nevents;
      if (!params.print_only){
        outtree->Fill();
      }
    }

    if (!params.print_only) {
      outtree->Write();
      outfile->Close();
    }

  }

  return 0;
}


int test_nsw_trigger_common_decoder_loop(Params& params, Statistics& statistics) {
  outBranches data;

  for (const std::string& filename : params.file_names) {

    TFile* outfile = nullptr;
    TTree* outtree = nullptr;
    // no need for a smart pointer for ttrees, given ROOT
    // behaviour (deleting ttree's when closing files)

    std::string data_file_name(filename);
    std::string out_file_name = data_file_name.substr(data_file_name.find_last_of("/\\") + 1) + ".decoded.root";

    std::cout << "Reading file " << data_file_name << std::endl;

    if (!params.print_only) {
      std::cout << "Saving here file " << out_file_name << std::endl;
      outfile = new TFile(out_file_name.c_str(), "recreate");
      outtree = new TTree("decoded_data", "decoded_data");
      test_nsw_trigger_common_decoder_init_tree(*outtree, data, params);
    }

    std::unique_ptr<DataReader> in_file;
    in_file.reset(pickDataReader(filename));
    if (!in_file->good()) {
      std::cout << "Skipping to next file, since broken" << std::endl;
      continue;
    }

    while (!in_file->endOfFile() && (params.max_events == 0 || statistics.nevents < params.max_events)) {

      if (!(statistics.nevents % static_cast<int>(std::pow(10, static_cast<int>(4 - params.printout_level))))) {
        std::cout << "Processing event: " << statistics.nevents << std::endl;
      }

      char* buf = nullptr;
      unsigned int size = 0;
      DRError err = in_file->getData(size, &buf);

      if (err != EventStorage::DROK) {
        std::cout << "Cannot get data properly from file; breaking" << std::endl;
        if (buf) {
          delete[] buf;
        }
        break;
      }

      eformat::read::FullEventFragment p((unsigned int*)(buf));

      data = outBranches();

      int report = 0;
      if ((report = test_nsw_trigger_common_decoder_event(p, data, params, statistics))) {
        std::cout << "No requested ROBs found for event " << statistics.nevents << "; skipping it! \n" << std::endl;
        ++statistics.nevents;
        if (buf) {
          delete[] buf;
        }
        continue;  // this way, an event without needed/wanted ROBs will not show up entirely
      }
      ++statistics.nevents;
      if (!params.print_only) {
        outtree->Fill();
      }
      if (buf) {
        delete[] buf;
      }
    }
    if (!params.print_only) {
      outtree->Write();
      outfile->Close();
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  Params params;
  Statistics statistics;

  int err;

  // option parsing
  if ((err = test_nsw_trigger_common_decoder_opt(argc, argv, params)))
    return err;

  // loop over files and events
  if (params.is_netio) {
    if ((err = test_nsw_trigger_common_decoder_loop_txt(params, statistics)))
      return err;
  } else {
    if ((err = test_nsw_trigger_common_decoder_loop(params, statistics)))
      return err;
  }

  // some stat printouts
  if ((err = test_nsw_trigger_common_decoder_end(statistics)))
    return err;

  return err;
}
