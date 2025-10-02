/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "AtlasDetectorIDHelper.h"
#include "IdDict/IdDictDefs.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include <iostream>

AtlasDetectorIDHelper::AtlasDetectorIDHelper() :
  AthMessaging("AtlasDetectorIDHelper") {
}

int
AtlasDetectorIDHelper::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
  if (m_initialized) return(0);

  m_initialized = true;

  AtlasDetectorID atlas_id ("", "");

  const IdDictDictionary* dict = dict_mgr.find_dictionary("InnerDetector");
  
  auto assignRegionFromAtlasID = [this, &dict](const ExpandedIdentifier& id, 
                                               size_type& regionIdx,
                                               const std::string& techType) {
      if (dict->find_region(id, regionIdx)) {
        ATH_MSG_WARNING("initialize_from_dictionary - unable to find "<<techType<<" region index: id, reg "
          << id << " " << regionIdx);
      }
  };

  auto assignRegionIdxFromGrp = [this, &dict](const std::string& grp, size_type& regionIdx) {
    IdDictGroup* group = dict->find_group(grp);
    if (!group || !group->regions().size()) {
        ATH_MSG_VERBOSE("The group "<<grp<<" is not present.");
        regionIdx = UNDEFINED;
        return;
    }
    regionIdx = group->regions().front()->index();
    ATH_MSG_VERBOSE("Region index for "<<grp<<" will be assigned to "<<regionIdx);

  };

  auto assignRegionIdxFromRegion = [this, &dict](const std::string& grp, size_type& regionIdx) {
    IdDictRegion* region = dict->find_region(grp);
    if (!region) {
        ATH_MSG_VERBOSE("The group "<<grp<<" is not present.");
        regionIdx = UNDEFINED;
        return;
    }
    regionIdx = region->index();
    ATH_MSG_VERBOSE("Region index for "<<grp<<" will be assigned to "<<regionIdx);
  };
  

  if (!dict) {
    ATH_MSG_ERROR("initialize_from_dictionary - cannot access InnerDetector dictionary");
    return 1;
  } 

  // Check if this is High Luminosity LHC layout
  if (dict->version() == "ITkHGTD" || dict->version() == "ITkHGTDPLR" || dict->version() == "P2-RUN4") {
    m_isHighLuminosityLHC = true;
  }
  assignRegionFromAtlasID(atlas_id.pixel_exp(), m_pixel_region_index, "pixel");
  // for High Luminosity LHC layout one cannot get the sct region as below, nor
  // is there any trt regions
  if (!m_isHighLuminosityLHC) {
    assignRegionFromAtlasID(atlas_id.sct_exp(), m_sct_region_index, "sct");
    assignRegionFromAtlasID(atlas_id.trt_exp(), m_trt_region_index, "trt");
  }

  dict = dict_mgr.find_dictionary("LArCalorimeter");
  if (!dict) {
    ATH_MSG_WARNING("initialize_from_dictionary - cannot access LArCalorimeter dictionary");
    return 1;
  } 
  assignRegionFromAtlasID(atlas_id.lar_em_exp(), m_lar_em_region_index, "lar_em");
  assignRegionFromAtlasID(atlas_id.lar_hec_exp(), m_lar_hec_region_index, "lar_hec");
  assignRegionFromAtlasID(atlas_id.lar_fcal_exp(), m_lar_fcal_region_index, "lar_fcal");
  // Get Calorimetry dictionary for both LVL1 and Dead material
  dict = dict_mgr.find_dictionary("Calorimeter");
  if (!dict) {
    ATH_MSG_WARNING("initialize_from_dictionary - cannot access Calorimeter dictionary");
    return 1;
  }
  // Save index to a LVL1 region for unpacking
  assignRegionIdxFromRegion("Lvl1_0", m_lvl1_region_index);
  // Save index to a Dead Material region for unpacking
  assignRegionIdxFromRegion("DM_4_1_0_0", m_dm_region_index);

  dict = dict_mgr.find_dictionary("TileCalorimeter");
  if (!dict) {
    ATH_MSG_WARNING("initialize_from_dictionary - cannot access TileCalorimeter dictionary");
    return 1;
  } 
  
  assignRegionFromAtlasID(atlas_id.tile_exp(), m_tile_region_index, "tile");

  dict = dict_mgr.find_dictionary("MuonSpectrometer");
  if (!dict) {
    ATH_MSG_WARNING("initialize_from_dictionary - cannot access MuonSpectrometer dictionary");
    return 1;
  } 
  m_station_field = dict->find_field("stationName");
  if (!m_station_field) {
    ATH_MSG_WARNING("initialize_from_dictionary - cannot access stationName field");
    return 1;
  } else {
    m_muon_station_index = m_station_field->index();
  }
  assignRegionIdxFromGrp("mdt", m_mdt_region_index);
  assignRegionIdxFromGrp("csc", m_csc_region_index);
  assignRegionIdxFromGrp("rpc", m_rpc_region_index);
  assignRegionIdxFromGrp("tgc", m_tgc_region_index);
  assignRegionIdxFromGrp("mm", m_mm_region_index);
  assignRegionIdxFromGrp("stgc", m_stgc_region_index);

  dict = dict_mgr.find_dictionary("ForwardDetectors");
  if (!dict) {
    ATH_MSG_WARNING("initialize_from_dictionary - cannot access ForwardDetectors dictionary");
    return 1;
  } 
  
  assignRegionFromAtlasID(atlas_id.alfa_exp(), m_alfa_region_index, "alfa");
  assignRegionFromAtlasID(atlas_id.bcm_exp(), m_bcm_region_index, "bcm");
  assignRegionFromAtlasID(atlas_id.lucid_exp(), m_lucid_region_index, "lucid");
  assignRegionFromAtlasID(atlas_id.zdc_exp(), m_zdc_region_index, "zdc");

  ATH_MSG_VERBOSE( "AtlasDetectorIDHelper::initialize_from_dictionary ");
  ATH_MSG_VERBOSE( " pixel_region_index     " << m_pixel_region_index);
  ATH_MSG_VERBOSE( " sct_region_index       " << m_sct_region_index);
  ATH_MSG_VERBOSE( " trt_region_index       " << m_trt_region_index);
  ATH_MSG_VERBOSE( " lar_em_region_index    " << m_lar_em_region_index);
  ATH_MSG_VERBOSE( " lar_hec_region_index   " << m_lar_hec_region_index);
  ATH_MSG_VERBOSE( " lar_fcal_region_index  " << m_lar_fcal_region_index);
  ATH_MSG_VERBOSE( " lvl1_region_index      " << m_lvl1_region_index);
  ATH_MSG_VERBOSE( " tile_region_index      " << m_tile_region_index);
  ATH_MSG_VERBOSE( " mdt_region_index       " << m_mdt_region_index);
  ATH_MSG_VERBOSE( " csc_region_index       " << m_csc_region_index);
  ATH_MSG_VERBOSE( " rpc_region_index       " << m_rpc_region_index);
  ATH_MSG_VERBOSE( " tgc_region_index       " << m_tgc_region_index);
  ATH_MSG_VERBOSE( " muon_station_index     " << m_muon_station_index);
  return 0;
}
