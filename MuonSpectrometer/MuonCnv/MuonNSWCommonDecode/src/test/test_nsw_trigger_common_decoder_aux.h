/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


//support header for structs and branch managment

//params for command line options
struct Params
{
  bool          print_only {false};
  bool          is_netio {false};
  uint32_t      printout_level {0};
  uint32_t      max_events {0};
  std::vector<std::string>   elink_types {};  //comma separated list of allowed technologies/elinks
  std::vector<std::string>   file_names  {};  //will create independent out files, nothing smarter for now
};

//statistics to be print out at the end
struct Statistics
{
  uint32_t nevents {0};                  //total in input
  uint32_t nfrags_not_readable {0};      //any fragment, not just the ones we are interested in (unfornately we can't know the type, before checking)
  uint32_t nfrags_not_decoded {0};       //problems during decoding of the relevant ROBs
  uint32_t nfrags_sus_rob_stat {0};      //(request) rob status flag not null; could still be decoded anyway potentially
  uint32_t nlinks_sus_felix_stat {0};    //felix status flag not null; could still be decoded anyway potentially
  double   total_decoding_time {0};      //time spent by the decoder, not by the TTree fill, etc. (events with decoding failures also count)
  double   avg_decoding_time {0};        //avg time spent by the decoder
  double   avg_robs_per_event {0};       //avg number of requested ROBs per event
  double   avg_robs_wErrors {0};         //avg number of requested ROBs with errors
};


//all the possible branches
struct outBranches
{
  //ATLAS level info
  uint32_t b_run_number;
  uint32_t b_run_type;
  uint32_t b_lumi_block;
  uint32_t b_L1ID;
  uint32_t b_BCID;
  uint32_t b_BC_time_seconds; // unix time
  uint32_t b_BC_time_nanoseconds; // nanoseconds additionally to the amount of seconds

  //each event has multple ROBs (e.g. multiple sectors)
  //each ROB then has multiple elinks
  //MML1A - comtemplating multiple elinks,
  //so the vector is on elinks for all the sectors
  //NB: for NSW ROB==ROD
  std::vector<uint32_t> b_MML1A_ROB_sourceID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_ROB_status = {} ;
  std::vector<uint32_t> b_MML1A_ROD_sourceID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_subdetID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_moduleID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_L1ID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_BCID = {} ;
  std::vector<uint32_t> b_MML1A_ROD_n_words = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_ROD_status = {} ; 
  std::vector<uint32_t> b_MML1A_link_id = {} ;
  std::vector<uint32_t> b_MML1A_link_status = {} ;
  std::vector<uint32_t> b_MML1A_head_fragID = {} ;
  std::vector<uint32_t> b_MML1A_head_sectID = {} ;
  std::vector<uint32_t> b_MML1A_head_EC = {} ;
  std::vector<uint32_t> b_MML1A_head_flags = {} ;
  std::vector<uint32_t> b_MML1A_head_BCID = {} ;
  std::vector<uint32_t> b_MML1A_head_orbit = {} ;
  std::vector<uint32_t> b_MML1A_head_spare = {} ;
  std::vector<uint32_t> b_MML1A_L1ID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_versionID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_local_req_BCID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_local_rel_BCID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_open_BCID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_req_BCID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_close_BCID = {} ;
  std::vector<uint32_t> b_MML1A_l1a_timeout = {} ;
  std::vector<uint32_t> b_MML1A_l1a_open_BCID_offset = {} ;
  std::vector<uint32_t> b_MML1A_l1a_req_BCID_offset = {} ;
  std::vector<uint32_t> b_MML1A_l1a_close_BCID_offset = {} ;
  std::vector<uint32_t> b_MML1A_l1a_timeout_config = {} ;
  std::vector<uint32_t> b_MML1A_l1a_busy_thr = {} ;
  std::vector<uint32_t> b_MML1A_l1a_engine_snapshot = {} ;
  std::vector<uint32_t> b_MML1A_l1a_link_const = {} ;
  std::vector<uint32_t> b_MML1A_l1a_padding = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_nbits = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_nwords = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_fifo_size = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_stream_head_streamID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_BCID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_pipeID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_fiberID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_layers = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_art_channels = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_trig_globalX = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_trig_globalU = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_trig_BCID = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_trig_dTheta = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_trig_phiBin = {} ;
  std::vector<std::vector<uint32_t>> b_MML1A_trig_rBin = {} ;
  std::vector<uint32_t> b_MML1A_trailer_CRC = {} ;
  std::vector<bool> b_MML1A_CRC_ok = {} ;
  //MMMon - comtemplating multiple elinks (even if only one possible in current design)
  //so the vector is on elinks for all the sectors
  std::vector<uint32_t> b_MMMon_ROB_sourceID = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_ROB_status = {} ;
  std::vector<uint32_t> b_MMMon_ROD_sourceID = {} ;
  std::vector<uint32_t> b_MMMon_ROD_subdetID = {} ;
  std::vector<uint32_t> b_MMMon_ROD_moduleID = {} ;
  std::vector<uint32_t> b_MMMon_ROD_L1ID = {} ;
  std::vector<uint32_t> b_MMMon_ROD_BCID = {} ;
  std::vector<uint32_t> b_MMMon_ROD_n_words = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_ROD_status = {} ;
  std::vector<uint32_t> b_MMMon_link_id = {} ;
  std::vector<uint32_t> b_MMMon_link_status = {} ;
  std::vector<uint32_t> b_MMMon_head_fragID = {} ;
  std::vector<uint32_t> b_MMMon_head_sectID = {} ;
  std::vector<uint32_t> b_MMMon_head_EC = {} ;
  std::vector<uint32_t> b_MMMon_head_flags = {} ;
  std::vector<uint32_t> b_MMMon_head_BCID = {} ;
  std::vector<uint32_t> b_MMMon_head_orbit = {} ;
  std::vector<uint32_t> b_MMMon_head_spare = {} ;
  std::vector<uint32_t> b_MMMon_L1ID = {} ;
  std::vector<uint32_t> b_MMMon_head_coincBCID = {} ;
  std::vector<uint32_t> b_MMMon_head_regionCount = {} ;
  std::vector<uint32_t> b_MMMon_head_coincRegion = {} ;
  std::vector<uint32_t> b_MMMon_head_reserved = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_streamID = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_regionCount = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_triggerID = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_V1 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_V0 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_U1 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_U0 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_X3 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_X2 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_X1 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_finder_X0 = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_streamID = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_regionCount = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_triggerID = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_filler = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_mxG = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_muG = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_mvG = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_mxL = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_mx_ROI = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_dTheta = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_zero = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_phiSign = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_phiBin = {} ;
  std::vector<std::vector<uint32_t>> b_MMMon_fitter_rBin = {} ;
  std::vector<uint32_t> b_MMMon_trailer_CRC = {} ;
  std::vector<bool> b_MMMon_CRC_ok = {} ;
  //PadL1A - comtemplating multiple elinks
  //so the vector is on elinks for all the sectors
  std::vector<uint32_t> b_PadL1A_ROB_sourceID = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_ROB_status = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_sourceID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_subdetID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_moduleID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_L1ID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_BCID = {} ;
  std::vector<uint32_t> b_PadL1A_ROD_n_words = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_ROD_status = {} ;
  std::vector<uint32_t> b_PadL1A_flags = {} ;
  std::vector<uint32_t> b_PadL1A_ec = {} ;
  std::vector<uint32_t> b_PadL1A_fragid = {} ;
  std::vector<uint32_t> b_PadL1A_secid = {} ;
  std::vector<uint32_t> b_PadL1A_spare = {} ;
  std::vector<uint32_t> b_PadL1A_orbit = {} ;
  std::vector<uint32_t> b_PadL1A_bcid = {} ;
  std::vector<uint32_t> b_PadL1A_l1id = {} ;
  std::vector<uint32_t> b_PadL1A_hit_n = {} ;
  std::vector<uint32_t> b_PadL1A_pfeb_n = {} ;
  std::vector<uint32_t> b_PadL1A_trigger_n = {} ;
  std::vector<uint32_t> b_PadL1A_bcid_n = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_relbcid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_pfeb = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_tdschannel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_vmmchannel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_vmm = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_hit_padchannel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_pfeb_addr = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_pfeb_nchan = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_pfeb_disconnected = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_trigger_bandid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_trigger_phiid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_trigger_relbcid = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_bcid_rel = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_bcid_status = {} ;
  std::vector<std::vector<uint32_t>> b_PadL1A_bcid_multzero = {} ;
  std::vector<uint16_t> b_PadL1A_CRC = {} ;
  std::vector<bool> b_PadL1A_CRC_ok = {} ;
  //STGL1A - comtemplating multiple elinks
  //so the vector is on elinks for all the sectors
  std::vector<uint32_t> b_STGL1A_ROB_sourceID = {} ;
  std::vector<std::vector<uint32_t>> b_STGL1A_ROB_status = {} ;
  std::vector<uint32_t> b_STGL1A_ROD_sourceID = {} ;
  std::vector<uint32_t> b_STGL1A_ROD_subdetID = {} ;
  std::vector<uint32_t> b_STGL1A_ROD_moduleID = {} ;
  std::vector<uint32_t> b_STGL1A_ROD_L1ID = {} ;
  std::vector<uint32_t> b_STGL1A_ROD_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_ROD_n_words = {} ;
  std::vector<std::vector<uint32_t>> b_STGL1A_ROD_status = {} ;
  std::vector<uint32_t> b_STGL1A_head_fragID = {} ;
  std::vector<uint32_t> b_STGL1A_head_sectID = {} ;
  std::vector<uint32_t> b_STGL1A_head_EC = {} ;
  std::vector<uint32_t> b_STGL1A_head_flags = {} ;
  std::vector<uint32_t> b_STGL1A_head_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_head_orbit = {} ;
  std::vector<uint32_t> b_STGL1A_head_spare = {} ;
  std::vector<uint32_t> b_STGL1A_L1ID = {} ;

  std::vector<uint32_t> b_STGL1A_l1a_versionID = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_local_req_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_local_rel_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_open_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_req_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_close_BCID = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_timeout = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_open_BCID_offset = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_req_BCID_offset = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_close_BCID_offset = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_timeout_config = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_busy_thr = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_engine_snapshot = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_link_const = {} ;
  std::vector<uint32_t> b_STGL1A_l1a_padding = {} ;

  std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_nbits = {} ;
  std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_nwords = {} ;
  std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_fifo_size = {} ;
  std::vector<std::vector<uint32_t>> b_STGL1A_stream_head_streamID = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_coincidence_wedge = {};
  
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_3 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_2 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_1 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_phiID_0 = {};
  
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_3 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_2 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_1 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_bandID_0 = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_BCID = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_pad_header = {};

  std::vector<std::vector<uint32_t>> b_STGL1A_pad_idleFlag = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_merge_LUT_choiceSelection = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_merge_nsw_segmentSelector = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_merge_valid_segmentSelector = {};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_monitor_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_spare_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_lowRes_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_phiRes_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_dTheta_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_phiID_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPSegments::num_segments> b_STGL1A_merge_RIndex_segments{};
  std::vector<std::vector<uint32_t>> b_STGL1A_merge_BCID = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_merge_sectorID = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_merge_header = {};

  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_monitor_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_spare_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_lowRes_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_phiRes_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_dTheta_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_phiID_segments{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPMMData::num_mm> b_STGL1A_mm_RIndex_segments{};
  std::vector<std::vector<uint32_t>> b_STGL1A_mm_BCID = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_mm_header = {};

  // Strip packet data
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPStrips::num_strips> b_STGL1A_strip_adc{};
  std::array<std::vector<std::vector<uint32_t>>, Muon::nsw::STGTPStrips::num_offsets> b_STGL1A_strip_offsets{};
  std::vector<std::vector<uint32_t>> b_STGL1A_strip_phiIdValue = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_strip_phiIdSign = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_strip_bandId = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_strip_BCID = {};
  std::vector<std::vector<uint32_t>> b_STGL1A_strip_header = {};

  std::vector<uint32_t> b_STGL1A_trailer_CRC = {} ;
};


int test_nsw_trigger_common_decoder_init_tree (TTree &outtree, outBranches &data, Params &params) {
  //ATLAS level info
  outtree.Branch( "run_number", &data.b_run_number);
  outtree.Branch( "run_type", &data.b_run_type);
  outtree.Branch( "lumi_block", &data.b_lumi_block);
  outtree.Branch( "L1ID", &data.b_L1ID);
  outtree.Branch( "BCID", &data.b_BCID);
  outtree.Branch( "BC_time_seconds", &data.b_BC_time_seconds);
  outtree.Branch( "BC_time_nanoseconds", &data.b_BC_time_nanoseconds);

  //not the most elegant way, can change if the branch number is diverging (must change the branch struct too)
  if ( std::find(params.elink_types.begin(), params.elink_types.end(), "MML1A") != params.elink_types.end() ) {
    outtree.Branch( "MML1A_ROB_sourceID", &data.b_MML1A_ROB_sourceID);
    outtree.Branch( "MML1A_ROB_status", &data.b_MML1A_ROB_status);
    outtree.Branch( "MML1A_ROD_sourceID", &data.b_MML1A_ROD_sourceID);
    outtree.Branch( "MML1A_ROD_subdetID", &data.b_MML1A_ROD_subdetID);
    outtree.Branch( "MML1A_ROD_moduleID", &data.b_MML1A_ROD_moduleID);
    outtree.Branch( "MML1A_ROD_L1ID", &data.b_MML1A_ROD_L1ID);
    outtree.Branch( "MML1A_ROD_BCID", &data.b_MML1A_ROD_BCID);
    outtree.Branch( "MML1A_ROD_n_words", &data.b_MML1A_ROD_n_words);
    outtree.Branch( "MML1A_ROD_status", &data.b_MML1A_ROD_status);
    outtree.Branch( "MML1A_link_id", &data.b_MML1A_link_id);
    outtree.Branch( "MML1A_link_status", &data.b_MML1A_link_status);
    outtree.Branch( "MML1A_head_fragID", &data.b_MML1A_head_fragID);
    outtree.Branch( "MML1A_head_sectID", &data.b_MML1A_head_sectID);
    outtree.Branch( "MML1A_head_EC", &data.b_MML1A_head_EC);
    outtree.Branch( "MML1A_head_flags", &data.b_MML1A_head_flags);
    outtree.Branch( "MML1A_head_BCID", &data.b_MML1A_head_BCID);
    outtree.Branch( "MML1A_head_orbit", &data.b_MML1A_head_orbit);
    outtree.Branch( "MML1A_head_spare", &data.b_MML1A_head_spare);
    outtree.Branch( "MML1A_L1ID", &data.b_MML1A_L1ID);
    outtree.Branch( "MML1A_l1a_versionID", &data.b_MML1A_l1a_versionID);
    outtree.Branch( "MML1A_l1a_local_req_BCID", &data.b_MML1A_l1a_local_req_BCID);
    outtree.Branch( "MML1A_l1a_local_rel_BCID", &data.b_MML1A_l1a_local_rel_BCID);
    outtree.Branch( "MML1A_l1a_open_BCID", &data.b_MML1A_l1a_open_BCID);
    outtree.Branch( "MML1A_l1a_req_BCID", &data.b_MML1A_l1a_req_BCID);
    outtree.Branch( "MML1A_l1a_close_BCID", &data.b_MML1A_l1a_close_BCID);
    outtree.Branch( "MML1A_l1a_timeout", &data.b_MML1A_l1a_timeout);
    outtree.Branch( "MML1A_l1a_open_BCID_offset", &data.b_MML1A_l1a_open_BCID_offset);
    outtree.Branch( "MML1A_l1a_req_BCID_offset", &data.b_MML1A_l1a_req_BCID_offset);
    outtree.Branch( "MML1A_l1a_close_BCID_offset", &data.b_MML1A_l1a_close_BCID_offset);
    outtree.Branch( "MML1A_l1a_timeout_config", &data.b_MML1A_l1a_timeout_config);
    outtree.Branch( "MML1A_l1a_busy_thr", &data.b_MML1A_l1a_busy_thr);
    outtree.Branch( "MML1A_l1a_engine_snapshot", &data.b_MML1A_l1a_engine_snapshot);
    outtree.Branch( "MML1A_l1a_link_const", &data.b_MML1A_l1a_link_const);
    outtree.Branch( "MML1A_l1a_padding", &data.b_MML1A_l1a_padding);
    outtree.Branch( "MML1A_stream_head_nbits", &data.b_MML1A_stream_head_nbits);
    outtree.Branch( "MML1A_stream_head_nwords", &data.b_MML1A_stream_head_nwords);
    outtree.Branch( "MML1A_stream_head_fifo_size", &data.b_MML1A_stream_head_fifo_size);
    outtree.Branch( "MML1A_stream_head_streamID", &data.b_MML1A_stream_head_streamID);
    outtree.Branch( "MML1A_trailer_CRC", &data.b_MML1A_trailer_CRC);
    outtree.Branch( "MML1A_art_BCID", &data.b_MML1A_art_BCID);
    outtree.Branch( "MML1A_art_pipeID", &data.b_MML1A_art_pipeID);
    outtree.Branch( "MML1A_art_fiberID", &data.b_MML1A_art_fiberID);
    outtree.Branch( "MML1A_art_layers", &data.b_MML1A_art_layers);
    outtree.Branch( "MML1A_art_channels", &data.b_MML1A_art_channels);
    outtree.Branch( "MML1A_trig_globalX", &data.b_MML1A_trig_globalX);
    outtree.Branch( "MML1A_trig_globalU", &data.b_MML1A_trig_globalU);
    outtree.Branch( "MML1A_trig_BCID", &data.b_MML1A_trig_BCID);
    outtree.Branch( "MML1A_trig_dTheta", &data.b_MML1A_trig_dTheta);
    outtree.Branch( "MML1A_trig_phiBin", &data.b_MML1A_trig_phiBin);
    outtree.Branch( "MML1A_trig_rBin", &data.b_MML1A_trig_rBin);
    outtree.Branch( "MML1A_CRC_ok", &data.b_MML1A_CRC_ok);
  }
  if ( std::find(params.elink_types.begin(), params.elink_types.end(), "MMMon") != params.elink_types.end() ) {
    outtree.Branch("MMMon_ROB_sourceID", &data.b_MMMon_ROB_sourceID);
    outtree.Branch("MMMon_ROB_status", &data.b_MMMon_ROB_status);
    outtree.Branch("MMMon_ROD_sourceID", &data.b_MMMon_ROD_sourceID);
    outtree.Branch("MMMon_ROD_subdetID", &data.b_MMMon_ROD_subdetID);
    outtree.Branch("MMMon_ROD_moduleID", &data.b_MMMon_ROD_moduleID);
    outtree.Branch("MMMon_ROD_L1ID", &data.b_MMMon_ROD_L1ID);
    outtree.Branch("MMMon_ROD_BCID", &data.b_MMMon_ROD_BCID);
    outtree.Branch("MMMon_ROD_n_words", &data.b_MMMon_ROD_n_words);
    outtree.Branch("MMMon_ROD_status", &data.b_MMMon_ROD_status);
    outtree.Branch("MMMon_link_id", &data.b_MMMon_link_id);
    outtree.Branch("MMMon_link_status", &data.b_MMMon_link_status);
    outtree.Branch("MMMon_head_fragID", &data.b_MMMon_head_fragID);
    outtree.Branch("MMMon_head_sectID", &data.b_MMMon_head_sectID);
    outtree.Branch("MMMon_head_EC", &data.b_MMMon_head_EC);
    outtree.Branch("MMMon_head_flags", &data.b_MMMon_head_flags);
    outtree.Branch("MMMon_head_BCID", &data.b_MMMon_head_BCID);
    outtree.Branch("MMMon_head_orbit", &data.b_MMMon_head_orbit);
    outtree.Branch("MMMon_head_spare", &data.b_MMMon_head_spare);
    outtree.Branch("MMMon_L1ID", &data.b_MMMon_L1ID);
    outtree.Branch("MMMon_head_coincBCID", &data.b_MMMon_head_coincBCID);
    outtree.Branch("MMMon_head_regionCount", &data.b_MMMon_head_regionCount);
    outtree.Branch("MMMon_head_coincRegion", &data.b_MMMon_head_coincRegion);
    outtree.Branch("MMMon_head_reserved", &data.b_MMMon_head_reserved);
    outtree.Branch("MMMon_finder_streamID", &data.b_MMMon_finder_streamID);
    outtree.Branch("MMMon_finder_regionCount", &data.b_MMMon_finder_regionCount);
    outtree.Branch("MMMon_finder_triggerID", &data.b_MMMon_finder_triggerID);
    outtree.Branch("MMMon_finder_V1", &data.b_MMMon_finder_V1);
    outtree.Branch("MMMon_finder_V0", &data.b_MMMon_finder_V0);
    outtree.Branch("MMMon_finder_U1", &data.b_MMMon_finder_U1);
    outtree.Branch("MMMon_finder_U0", &data.b_MMMon_finder_U0);
    outtree.Branch("MMMon_finder_X3", &data.b_MMMon_finder_X3);
    outtree.Branch("MMMon_finder_X2", &data.b_MMMon_finder_X2);
    outtree.Branch("MMMon_finder_X1", &data.b_MMMon_finder_X1);
    outtree.Branch("MMMon_finder_X0", &data.b_MMMon_finder_X0);
    outtree.Branch("MMMon_fitter_streamID", &data.b_MMMon_fitter_streamID);
    outtree.Branch("MMMon_fitter_regionCount", &data.b_MMMon_fitter_regionCount);
    outtree.Branch("MMMon_fitter_triggerID", &data.b_MMMon_fitter_triggerID);
    outtree.Branch("MMMon_fitter_filler", &data.b_MMMon_fitter_filler);
    outtree.Branch("MMMon_fitter_mxG", &data.b_MMMon_fitter_mxG);
    outtree.Branch("MMMon_fitter_muG", &data.b_MMMon_fitter_muG);
    outtree.Branch("MMMon_fitter_mvG", &data.b_MMMon_fitter_mvG);
    outtree.Branch("MMMon_fitter_mxL", &data.b_MMMon_fitter_mxL);
    outtree.Branch("MMMon_fitter_mx_ROI", &data.b_MMMon_fitter_mx_ROI);
    outtree.Branch("MMMon_fitter_dTheta", &data.b_MMMon_fitter_dTheta);
    outtree.Branch("MMMon_fitter_zero", &data.b_MMMon_fitter_zero);
    outtree.Branch("MMMon_fitter_phiSign", &data.b_MMMon_fitter_phiSign);
    outtree.Branch("MMMon_fitter_phiBin", &data.b_MMMon_fitter_phiBin);
    outtree.Branch("MMMon_fitter_rBin", &data.b_MMMon_fitter_rBin);
    outtree.Branch("MMMon_trailer_CRC", &data.b_MMMon_trailer_CRC);
    outtree.Branch("MMMon_CRC_ok", &data.b_MMMon_CRC_ok);
  }
  if ( std::find(params.elink_types.begin(), params.elink_types.end(), "PadL1A") != params.elink_types.end() ) {
    outtree.Branch( "PadL1A_ROB_sourceID", &data.b_PadL1A_ROB_sourceID);
    outtree.Branch( "PadL1A_ROB_status", &data.b_PadL1A_ROB_status);
    outtree.Branch( "PadL1A_ROD_sourceID", &data.b_PadL1A_ROD_sourceID);
    outtree.Branch( "PadL1A_ROD_subdetID", &data.b_PadL1A_ROD_subdetID);
    outtree.Branch( "PadL1A_ROD_moduleID", &data.b_PadL1A_ROD_moduleID);
    outtree.Branch( "PadL1A_ROD_L1ID", &data.b_PadL1A_ROD_L1ID);
    outtree.Branch( "PadL1A_ROD_BCID", &data.b_PadL1A_ROD_BCID);
    outtree.Branch( "PadL1A_ROD_n_words", &data.b_PadL1A_ROD_n_words);
    outtree.Branch( "PadL1A_ROD_status", &data.b_PadL1A_ROD_status);
    outtree.Branch( "PadL1A_flags", &data.b_PadL1A_flags);
    outtree.Branch( "PadL1A_ec", &data.b_PadL1A_ec);
    outtree.Branch( "PadL1A_fragid", &data.b_PadL1A_fragid);
    outtree.Branch( "PadL1A_secid", &data.b_PadL1A_secid);
    outtree.Branch( "PadL1A_spare", &data.b_PadL1A_spare);
    outtree.Branch( "PadL1A_orbit", &data.b_PadL1A_orbit);
    outtree.Branch( "PadL1A_bcid", &data.b_PadL1A_bcid);
    outtree.Branch( "PadL1A_l1id", &data.b_PadL1A_l1id);
    outtree.Branch( "PadL1A_hit_n", &data.b_PadL1A_hit_n);
    outtree.Branch( "PadL1A_pfeb_n", &data.b_PadL1A_pfeb_n);
    outtree.Branch( "PadL1A_trigger_n", &data.b_PadL1A_trigger_n);
    outtree.Branch( "PadL1A_bcid_n", &data.b_PadL1A_bcid_n);
    outtree.Branch( "PadL1A_hit_relbcid", &data.b_PadL1A_hit_relbcid);
    outtree.Branch( "PadL1A_hit_pfeb", &data.b_PadL1A_hit_pfeb);
    outtree.Branch( "PadL1A_hit_tdschannel", &data.b_PadL1A_hit_tdschannel);
    outtree.Branch( "PadL1A_hit_vmmchannel", &data.b_PadL1A_hit_vmmchannel);
    outtree.Branch( "PadL1A_hit_vmm", &data.b_PadL1A_hit_vmm);
    outtree.Branch( "PadL1A_hit_padchannel", &data.b_PadL1A_hit_padchannel);
    outtree.Branch( "PadL1A_pfeb_addr", &data.b_PadL1A_pfeb_addr);
    outtree.Branch( "PadL1A_pfeb_nchan", &data.b_PadL1A_pfeb_nchan);
    outtree.Branch( "PadL1A_pfeb_disconnected", &data.b_PadL1A_pfeb_disconnected);
    outtree.Branch( "PadL1A_trigger_bandid", &data.b_PadL1A_trigger_bandid);
    outtree.Branch( "PadL1A_trigger_phiid", &data.b_PadL1A_trigger_phiid);
    outtree.Branch( "PadL1A_trigger_relbcid", &data.b_PadL1A_trigger_relbcid);
    outtree.Branch( "PadL1A_bcid_rel", &data.b_PadL1A_bcid_rel);
    outtree.Branch( "PadL1A_bcid_status", &data.b_PadL1A_bcid_status);
    outtree.Branch( "PadL1A_bcid_multzero", &data.b_PadL1A_bcid_multzero);
    outtree.Branch( "PadL1A_CRC", &data.b_PadL1A_CRC);
    outtree.Branch( "PadL1A_CRC_ok", &data.b_PadL1A_CRC_ok);
  }
  if ( std::find(params.elink_types.begin(), params.elink_types.end(), "STGL1A") != params.elink_types.end() ) {
    outtree.Branch( "STGL1A_ROB_sourceID", &data.b_STGL1A_ROB_sourceID);
    outtree.Branch( "STGL1A_ROB_status", &data.b_STGL1A_ROB_status);
    outtree.Branch( "STGL1A_ROD_sourceID", &data.b_STGL1A_ROD_sourceID);
    outtree.Branch( "STGL1A_ROD_subdetID", &data.b_STGL1A_ROD_subdetID);
    outtree.Branch( "STGL1A_ROD_moduleID", &data.b_STGL1A_ROD_moduleID);
    outtree.Branch( "STGL1A_ROD_L1ID", &data.b_STGL1A_ROD_L1ID);
    outtree.Branch( "STGL1A_ROD_BCID", &data.b_STGL1A_ROD_BCID);
    outtree.Branch( "STGL1A_ROD_n_words", &data.b_STGL1A_ROD_n_words);
    outtree.Branch( "STGL1A_ROD_status", &data.b_STGL1A_ROD_status);
    outtree.Branch( "STGL1A_head_fragID", &data.b_STGL1A_head_fragID);
    outtree.Branch( "STGL1A_head_sectID", &data.b_STGL1A_head_sectID);
    outtree.Branch( "STGL1A_head_EC", &data.b_STGL1A_head_EC);
    outtree.Branch( "STGL1A_head_flags", &data.b_STGL1A_head_flags);
    outtree.Branch( "STGL1A_head_BCID", &data.b_STGL1A_head_BCID);
    outtree.Branch( "STGL1A_head_orbit", &data.b_STGL1A_head_orbit);
    outtree.Branch( "STGL1A_head_spare", &data.b_STGL1A_head_spare);
    outtree.Branch( "STGL1A_L1ID", &data.b_STGL1A_L1ID);
    outtree.Branch( "STGL1A_l1a_versionID", &data.b_STGL1A_l1a_versionID);
    outtree.Branch( "STGL1A_l1a_local_req_BCID", &data.b_STGL1A_l1a_local_req_BCID);
    outtree.Branch( "STGL1A_l1a_local_rel_BCID", &data.b_STGL1A_l1a_local_rel_BCID);
    outtree.Branch( "STGL1A_l1a_open_BCID", &data.b_STGL1A_l1a_open_BCID);
    outtree.Branch( "STGL1A_l1a_req_BCID", &data.b_STGL1A_l1a_req_BCID);
    outtree.Branch( "STGL1A_l1a_close_BCID", &data.b_STGL1A_l1a_close_BCID);
    outtree.Branch( "STGL1A_l1a_timeout", &data.b_STGL1A_l1a_timeout);
    outtree.Branch( "STGL1A_l1a_open_BCID_offset", &data.b_STGL1A_l1a_open_BCID_offset);
    outtree.Branch( "STGL1A_l1a_req_BCID_offset", &data.b_STGL1A_l1a_req_BCID_offset);
    outtree.Branch( "STGL1A_l1a_close_BCID_offset", &data.b_STGL1A_l1a_close_BCID_offset);
    outtree.Branch( "STGL1A_l1a_timeout_config", &data.b_STGL1A_l1a_timeout_config);
    outtree.Branch( "STGL1A_l1a_busy_thr", &data.b_STGL1A_l1a_busy_thr);
    outtree.Branch( "STGL1A_l1a_engine_snapshot", &data.b_STGL1A_l1a_engine_snapshot);
    outtree.Branch( "STGL1A_l1a_link_const", &data.b_STGL1A_l1a_link_const);
    outtree.Branch( "STGL1A_stream_head_nbits", &data.b_STGL1A_stream_head_nbits);
    outtree.Branch( "STGL1A_stream_head_nwords", &data.b_STGL1A_stream_head_nwords);
    outtree.Branch( "STGL1A_stream_head_fifo_size", &data.b_STGL1A_stream_head_fifo_size);
    outtree.Branch( "STGL1A_stream_head_streamID", &data.b_STGL1A_stream_head_streamID);
    
    outtree.Branch( "STGL1A_pad_coincidence_wedge", &data.b_STGL1A_pad_coincidence_wedge);
    outtree.Branch( "STGL1A_pad_phiID_3", &data.b_STGL1A_pad_phiID_3);
    outtree.Branch( "STGL1A_pad_phiID_2", &data.b_STGL1A_pad_phiID_2);
    outtree.Branch( "STGL1A_pad_phiID_1", &data.b_STGL1A_pad_phiID_1);
    outtree.Branch( "STGL1A_pad_phiID_0", &data.b_STGL1A_pad_phiID_0);
    
    outtree.Branch( "STGL1A_pad_bandID_3", &data.b_STGL1A_pad_bandID_3);
    outtree.Branch( "STGL1A_pad_bandID_2", &data.b_STGL1A_pad_bandID_2);
    outtree.Branch( "STGL1A_pad_bandID_1", &data.b_STGL1A_pad_bandID_1);
    outtree.Branch( "STGL1A_pad_bandID_0", &data.b_STGL1A_pad_bandID_0);
    
    outtree.Branch( "STGL1A_pad_BCID", &data.b_STGL1A_pad_BCID);
    outtree.Branch( "STGL1A_pad_header", &data.b_STGL1A_pad_header);
    outtree.Branch( "STGL1A_pad_idleFlag", &data.b_STGL1A_pad_idleFlag);
    
    outtree.Branch( "STGL1A_merge_LUT_choiceSelection", &data.b_STGL1A_merge_LUT_choiceSelection);
    outtree.Branch( "STGL1A_merge_nsw_segmentSelector", &data.b_STGL1A_merge_nsw_segmentSelector);
    outtree.Branch( "STGL1A_merge_valid_segmentSelector", &data.b_STGL1A_merge_valid_segmentSelector);
    
    for (std::size_t i=0; i < Muon::nsw::STGTPSegments::num_segments; ++i) {
      outtree.Branch( Muon::nsw::format("STGL1A_merge_monitor_segment{}", i).c_str(), &data.b_STGL1A_merge_monitor_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_merge_spare_segment{}", i).c_str(), &data.b_STGL1A_merge_spare_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_merge_lowRes_segment{}", i).c_str(), &data.b_STGL1A_merge_lowRes_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_merge_phiRes_segment{}", i).c_str(), &data.b_STGL1A_merge_phiRes_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_merge_dTheta_segment{}", i).c_str(), &data.b_STGL1A_merge_dTheta_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_merge_phiID_segment{}", i).c_str(), &data.b_STGL1A_merge_phiID_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_merge_RIndex_segment{}", i).c_str(), &data.b_STGL1A_merge_RIndex_segments.at(i));
    }
    
    outtree.Branch( "STGL1A_merge_BCID", &data.b_STGL1A_merge_BCID);
    outtree.Branch( "STGL1A_merge_header", &data.b_STGL1A_merge_header);
    outtree.Branch( "STGL1A_merge_sectorID", &data.b_STGL1A_merge_sectorID);


    for (std::size_t i=0; i < Muon::nsw::STGTPMMData::num_mm; ++i) {
      outtree.Branch( Muon::nsw::format("STGL1A_mm_monitor_segment{}", i).c_str(), &data.b_STGL1A_mm_monitor_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_mm_spare_segment{}", i).c_str(), &data.b_STGL1A_mm_spare_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_mm_lowRes_segment{}", i).c_str(), &data.b_STGL1A_mm_lowRes_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_mm_phiRes_segment{}", i).c_str(), &data.b_STGL1A_mm_phiRes_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_mm_dTheta_segment{}", i).c_str(), &data.b_STGL1A_mm_dTheta_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_mm_phiID_segment{}", i).c_str(), &data.b_STGL1A_mm_phiID_segments.at(i));
      outtree.Branch( Muon::nsw::format("STGL1A_mm_RIndex_segment{}", i).c_str(), &data.b_STGL1A_mm_RIndex_segments.at(i));
    }

    outtree.Branch( "STGL1A_mm_BCID", &data.b_STGL1A_mm_BCID);
    outtree.Branch( "STGL1A_mm_header", &data.b_STGL1A_mm_header);

    // Strip packet branches
    for (std::size_t i=0; i < Muon::nsw::STGTPStrips::num_strips; ++i) {
      outtree.Branch( Muon::nsw::format("STGL1A_strip_adc{}", i).c_str(), &data.b_STGL1A_strip_adc.at(i));
    }
    for (std::size_t i=0; i < Muon::nsw::STGTPStrips::num_offsets; ++i) {
      outtree.Branch( Muon::nsw::format("STGL1A_strip_offset{}", i).c_str(), &data.b_STGL1A_strip_offsets.at(i));
    }
    outtree.Branch( "STGL1A_strip_phiIdValue", &data.b_STGL1A_strip_phiIdValue);
    outtree.Branch( "STGL1A_strip_phiIdSign", &data.b_STGL1A_strip_phiIdSign);
    outtree.Branch( "STGL1A_strip_bandId", &data.b_STGL1A_strip_bandId);
    outtree.Branch( "STGL1A_strip_BCID", &data.b_STGL1A_strip_BCID);
    outtree.Branch( "STGL1A_strip_header", &data.b_STGL1A_strip_header);

    outtree.Branch( "STGL1A_trailer_CRC", &data.b_STGL1A_trailer_CRC);
  }
  return 0;
}


class CRCMonHelper {
 public:
 CRCMonHelper(uint preset, uint polynomial) : m_preset(preset), m_polynomial(polynomial) {
    for (uint i = 0; i<256; ++i) m_tab[i] = set_init(i);
  }
  
  template <typename Source>
    uint getCRC(const std::span<const Source> words) {
    uint crc = m_preset;
    uint N = sizeof(Source); //N 8b subwords per word
    
    double pad = 1;
    if (words[std::size(words)-1] & 0x0000FFFF) {pad = 0;} 

    for (uint i = 0; i< std::size(words)-pad; ++i) {
      for (uint j = 0; j<N-(i==(std::size(words)-1) && pad == 0 ? 2 : 0 ); ++j) {
	crc = update_crc(crc, (words[i] >> (N*8-(j+1)*8)) & 0xFF);
      }
    }
    return crc;
  }
  
 private:
  uint m_preset;
  uint m_polynomial;
  uint m_tab[256];

  uint set_init(uint i) {
    uint crc = 0;
    uint c = i << 8;
    for (uint j = 0; j<8; ++j) {
      if ((crc ^ c) & 0x8000) crc = (crc << 1) ^ m_polynomial;
      else crc = crc << 1;
      c = c << 1;
    }
    return crc;
  }

  uint update_crc(uint crc, uint c) {
    uint cc = 0xff & c;
    uint tmp = (crc >> 8) ^ cc;
    crc = (crc << 8) ^ m_tab[tmp & 0xff];
    crc = crc & 0xffff;
    return crc;
  }

};
