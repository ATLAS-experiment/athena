/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Support header: branch struct and tree-initialisation function for test_nsw_common_decoder.
// One TTree entry per event.  Vectors are indexed by elink (across all ROBs in the event);
// the innermost vectors hold per-channel data for that elink.

#include <TTree.h>

// ---------------------------------------------------------------------------
// Branch container
// ---------------------------------------------------------------------------
struct outBranches
{
  // -------------------------------------------------------------------------
  // ATLAS / full-event scalars (one value per entry)
  // -------------------------------------------------------------------------
  uint32_t b_run_number          {0};
  uint32_t b_run_type            {0};
  uint32_t b_lumi_block          {0};
  uint32_t b_L1ID                {0};
  uint32_t b_BCID                {0};
  uint32_t b_BC_time_seconds     {0};   // unix time
  uint32_t b_BC_time_nanoseconds {0};   // sub-second part

  // -------------------------------------------------------------------------
  // ROB / ROD level (one element per elink; ROB info is repeated for each
  // elink that belongs to the same ROB so that indexing is consistent)
  // -------------------------------------------------------------------------
  std::vector<uint32_t>              b_ROB_sourceID {};
  std::vector<std::vector<uint32_t>> b_ROB_status   {};   // status words

  std::vector<uint32_t>              b_ROD_sourceID {};
  std::vector<uint32_t>              b_ROD_subdetID {};
  std::vector<uint32_t>              b_ROD_moduleID {};
  std::vector<uint32_t>              b_ROD_L1ID     {};
  std::vector<uint32_t>              b_ROD_BCID     {};
  std::vector<uint32_t>              b_ROD_n_words  {};
  std::vector<std::vector<uint32_t>> b_ROD_status   {};   // status words

  // -------------------------------------------------------------------------
  // Elink header (from NSWElink accessors), one element per elink
  // -------------------------------------------------------------------------
  std::vector<uint32_t> b_elink_word       {};   // raw logical-ID / elink word
  std::vector<uint32_t> b_elink_status     {};   // packet status (FELIX errors)
  std::vector<uint32_t> b_elink_l1Id       {};
  std::vector<uint32_t> b_elink_bcId       {};
  std::vector<uint32_t> b_elink_rocId      {};
  std::vector<uint32_t> b_elink_orbit      {};
  std::vector<uint32_t> b_elink_nhits      {};   // number of decoded hits
  std::vector<bool>     b_elink_noTdc      {};
  std::vector<bool>     b_elink_isNull     {};
  std::vector<bool>     b_elink_tout       {};
  std::vector<bool>     b_elink_extended   {};
  std::vector<uint32_t> b_elink_checksum   {};
  std::vector<uint32_t> b_elink_nhitsTrail {};   // hits count from trailer
  std::vector<uint32_t> b_elink_l0Id       {};
  std::vector<uint32_t> b_elink_flagMiss   {};

  // -------------------------------------------------------------------------
  // NSWResourceId fields (from elinkId()), one element per elink
  // -------------------------------------------------------------------------
  std::vector<uint32_t> b_rid_elink        {};
  std::vector<uint32_t> b_rid_radius       {};
  std::vector<uint32_t> b_rid_layer        {};
  std::vector<uint32_t> b_rid_sector       {};
  std::vector<uint32_t> b_rid_resourceType {};
  std::vector<uint32_t> b_rid_dataType     {};
  std::vector<uint32_t> b_rid_version      {};
  std::vector<uint32_t> b_rid_detId        {};

  // Offline-decoded geometry (from NSWResourceId), one element per elink
  std::vector<bool>     b_rid_is_large_station {};
  std::vector<int32_t>  b_rid_station_eta      {};
  std::vector<uint32_t> b_rid_station_phi      {};
  std::vector<uint32_t> b_rid_multi_layer      {};
  std::vector<uint32_t> b_rid_gas_gap          {};

  // -------------------------------------------------------------------------
  // VMM channel data: outer index = elink, inner = channel on that elink
  // parity / neighbor / parity_ok stored as uint32_t to avoid
  // vector<vector<bool>> specialisation issues with ROOT
  // -------------------------------------------------------------------------
  std::vector<std::vector<uint32_t>> b_vmm_word           {};
  std::vector<std::vector<uint32_t>> b_vmm_roc_vmm        {};   // VMM id on the ROC (raw)
  std::vector<std::vector<uint32_t>> b_vmm_vmm            {};   // offline-remapped VMM id
  std::vector<std::vector<uint32_t>> b_vmm_channel        {};   // VMM channel number
  std::vector<std::vector<uint32_t>> b_vmm_rel_bcid       {};   // relative BCID
  std::vector<std::vector<uint32_t>> b_vmm_pdo            {};   // ADC amplitude
  std::vector<std::vector<uint32_t>> b_vmm_tdo            {};   // peaking time
  std::vector<std::vector<uint32_t>> b_vmm_parity         {};   // stored parity bit
  std::vector<std::vector<uint32_t>> b_vmm_neighbor       {};   // neighbor flag
  std::vector<std::vector<uint32_t>> b_vmm_parity_ok      {};   // calculated parity matches stored
  std::vector<std::vector<uint32_t>> b_vmm_channel_type   {};   // offline channel type
  std::vector<std::vector<uint32_t>> b_vmm_channel_number {};   // offline channel number
};


// ---------------------------------------------------------------------------
// Register all branches with the TTree
// ---------------------------------------------------------------------------
int test_nsw_common_decoder_init_tree (TTree &outtree, outBranches &data)
{
  // ATLAS event-level
  outtree.Branch ("run_number",           &data.b_run_number);
  outtree.Branch ("run_type",             &data.b_run_type);
  outtree.Branch ("lumi_block",           &data.b_lumi_block);
  outtree.Branch ("L1ID",                 &data.b_L1ID);
  outtree.Branch ("BCID",                 &data.b_BCID);
  outtree.Branch ("BC_time_seconds",      &data.b_BC_time_seconds);
  outtree.Branch ("BC_time_nanoseconds",  &data.b_BC_time_nanoseconds);

  // ROB / ROD (per elink)
  outtree.Branch ("ROB_sourceID",  &data.b_ROB_sourceID);
  outtree.Branch ("ROB_status",    &data.b_ROB_status);
  outtree.Branch ("ROD_sourceID",  &data.b_ROD_sourceID);
  outtree.Branch ("ROD_subdetID",  &data.b_ROD_subdetID);
  outtree.Branch ("ROD_moduleID",  &data.b_ROD_moduleID);
  outtree.Branch ("ROD_L1ID",      &data.b_ROD_L1ID);
  outtree.Branch ("ROD_BCID",      &data.b_ROD_BCID);
  outtree.Branch ("ROD_n_words",   &data.b_ROD_n_words);
  outtree.Branch ("ROD_status",    &data.b_ROD_status);

  // Elink header
  outtree.Branch ("elink_word",       &data.b_elink_word);
  outtree.Branch ("elink_status",     &data.b_elink_status);
  outtree.Branch ("elink_l1Id",       &data.b_elink_l1Id);
  outtree.Branch ("elink_bcId",       &data.b_elink_bcId);
  outtree.Branch ("elink_rocId",      &data.b_elink_rocId);
  outtree.Branch ("elink_orbit",      &data.b_elink_orbit);
  outtree.Branch ("elink_nhits",      &data.b_elink_nhits);
  outtree.Branch ("elink_noTdc",      &data.b_elink_noTdc);
  outtree.Branch ("elink_isNull",     &data.b_elink_isNull);
  outtree.Branch ("elink_tout",       &data.b_elink_tout);
  outtree.Branch ("elink_extended",   &data.b_elink_extended);
  outtree.Branch ("elink_checksum",   &data.b_elink_checksum);
  outtree.Branch ("elink_nhitsTrail", &data.b_elink_nhitsTrail);
  outtree.Branch ("elink_l0Id",       &data.b_elink_l0Id);
  outtree.Branch ("elink_flagMiss",   &data.b_elink_flagMiss);

  // NSWResourceId
  outtree.Branch ("rid_elink",        &data.b_rid_elink);
  outtree.Branch ("rid_radius",       &data.b_rid_radius);
  outtree.Branch ("rid_layer",        &data.b_rid_layer);
  outtree.Branch ("rid_sector",       &data.b_rid_sector);
  outtree.Branch ("rid_resourceType", &data.b_rid_resourceType);
  outtree.Branch ("rid_dataType",     &data.b_rid_dataType);
  outtree.Branch ("rid_version",      &data.b_rid_version);
  outtree.Branch ("rid_detId",        &data.b_rid_detId);

  // Offline geometry
  outtree.Branch ("rid_is_large_station", &data.b_rid_is_large_station);
  outtree.Branch ("rid_station_eta",      &data.b_rid_station_eta);
  outtree.Branch ("rid_station_phi",      &data.b_rid_station_phi);
  outtree.Branch ("rid_multi_layer",      &data.b_rid_multi_layer);
  outtree.Branch ("rid_gas_gap",          &data.b_rid_gas_gap);

  // VMM channel data (vector of vectors, inner dim = channels per elink)
  outtree.Branch ("vmm_word",           &data.b_vmm_word);
  outtree.Branch ("vmm_roc_vmm",        &data.b_vmm_roc_vmm);
  outtree.Branch ("vmm_vmm",            &data.b_vmm_vmm);
  outtree.Branch ("vmm_channel",        &data.b_vmm_channel);
  outtree.Branch ("vmm_rel_bcid",       &data.b_vmm_rel_bcid);
  outtree.Branch ("vmm_pdo",            &data.b_vmm_pdo);
  outtree.Branch ("vmm_tdo",            &data.b_vmm_tdo);
  outtree.Branch ("vmm_parity",         &data.b_vmm_parity);
  outtree.Branch ("vmm_neighbor",       &data.b_vmm_neighbor);
  outtree.Branch ("vmm_parity_ok",      &data.b_vmm_parity_ok);
  outtree.Branch ("vmm_channel_type",   &data.b_vmm_channel_type);
  outtree.Branch ("vmm_channel_number", &data.b_vmm_channel_number);

  return 0;
}
