/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

/***************************************************************************
   Inner Detector identifier package
   -------------------------------------------
***************************************************************************/


#include "InDetIdentifier/PLR_ID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/IdentifierHash.h"
#include <set>
#include <algorithm>
#include <iostream>




PLR_ID::PLR_ID(): PixelID(){
  // changes compared to PixelID:
  m_BARREL_EC_INDEX = 3;
  m_LAYER_DISK_INDEX = 4;
  m_PHI_MODULE_INDEX = 5;
  m_ETA_MODULE_INDEX = 6;
  m_PHI_INDEX_INDEX = 7;
  m_ETA_INDEX_INDEX = 8;
}

int
PLR_ID::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
  ATH_MSG_INFO("Initialize from dictionary");
  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
    return(0);
  } else {
    ATH_MSG_DEBUG("(Re)initialize");
  }

  // init base object
  if (PixelID::initialize_from_dictionary(dict_mgr)) return(1);

  // Register version of InnerDetector dictionary
  if (register_dict_tag(dict_mgr, "InnerDetector")) return(1);

  m_dict = dict_mgr.find_dictionary("InnerDetector");
  if (!m_dict) {
    ATH_MSG_FATAL("PLR_ID::initialize_from_dict - cannot access InnerDetector dictionary");
    return(1);
  }

  AtlasDetectorID::setDictVersion(dict_mgr, "InnerDetector");

  // Initialize the field indices
  if (initLevelsFromDict()) return(1);

  // save indet id
  m_pixel_id = lumi();
  if (!is_lumi(m_pixel_id)) {
    ATH_MSG_FATAL("PLR_ID::initialize_from_dict - cannot get plr id dictionary");
    return(1);
  }

  //
  // Set barrel field for testing is_barrel
  //
  int barrel_value;
  m_barrel_field.clear();
  //  barrel
  if (m_dict->get_label_value("barrel_endcap", "barrel", barrel_value)) {
    ATH_MSG_FATAL("Could not get value for label 'barrel' of field 'barrel_endcap' in dictionary " << m_dict->name());
    
    return(1);
  }
  m_barrel_field.add_value(barrel_value);
  m_barrel_field.add_value(barrel_value);
  ATH_MSG_DEBUG("PLR_ID::initialize_from_dict Set barrel field values: " << (std::string)m_barrel_field);
 

  //DBM
  //Set dbm field for testing is_dbm
  //
  // WARNING:
  //   modified to skip DBM when appropriate dictionary is not present
  //   by adding +999 or -999 to the field
  //

  int dbm_value{};
  m_dbm_field.clear();
  if (m_dict->get_label_value("barrel_endcap", "negative_dbm", dbm_value)) {
    if (m_dict->version().find("DBM") != std::string::npos) {
      ATH_MSG_WARNING("Could not get value for label 'negative_dbm' of field 'barrel_endcap' in dictionary " << m_dict->name());
     
    }
    //return (1);
    m_dbm_field.add_value(-999);
  } else {
    m_dbm_field.add_value(dbm_value);
  }
  if (m_dict->get_label_value("barrel_endcap", "positive_dbm", dbm_value)) {
    if (m_dict->version().find("DBM") != std::string::npos) {
      ATH_MSG_WARNING("Could not get value for label 'positive_dbm' of field 'barrel_endcap' in dictionary " << m_dict->name());
     
    }
    //return (1);
    m_dbm_field.add_value(999);
  } else {
    m_dbm_field.add_value(dbm_value);
  }
  ATH_MSG_DEBUG("PLR_ID::initialize_from_dict Set dbm field values: " << (std::string)m_dbm_field);
  
  //
  // Build multirange for the valid set of identifiers
  //


  // Find value for the field InnerDetector
  const IdDictDictionary* atlasDict = dict_mgr.find_dictionary("ATLAS");
  int inDetField = -1;
  if (atlasDict->get_label_value("subdet", "InnerDetector", inDetField)) {
    ATH_MSG_FATAL("Could not get value for label 'InnerDetector' of field 'subdet' in dictionary " << atlasDict->name());
   
    return(1);
  }

  // Find value for the field LuminosityDetectors
  int lumiField = -1;
  if (m_dict->get_label_value("part", "LuminosityDetectors", lumiField)) {
    ATH_MSG_FATAL("Could not get value for label 'LuminosityDetectors' of field 'part' in dictionary " << m_dict->name());
   
    return(1);
  }

  // Find value for the field PLR
  int plrField = -1;
  if (m_dict->get_label_value("PLR_or_BCM", "PLR", plrField)) {
    ATH_MSG_FATAL("Could not get value for label 'PLR' of field 'PLR_or_BCM' in dictionary " << m_dict->name());
    
    return(1);
  }

  ATH_MSG_DEBUG("PLR_ID::initialize_from_dict Found field values: InDet/LuminosityDetectors/PLR "
                << inDetField << "/" << lumiField << "/" << plrField);
 

  // Set up id for region and range prefix
  ExpandedIdentifier region_id;
  region_id.add(inDetField);
  region_id.add(lumiField);
  region_id.add(plrField);
  Range prefix;
  m_full_wafer_range = m_dict->build_multirange(region_id, prefix, "eta_module");
  m_full_pixel_range = m_dict->build_multirange(region_id, prefix);

  // Set the base identifier for PLR
  m_baseIdentifier = ((Identifier::value_type) 0);
  m_impl[kIndet].pack(indet_field_value(), m_baseIdentifier);
  m_lumi_impl.pack(lumi_field_value(), m_baseIdentifier);
  m_plr_impl.pack(plr_field_value(), m_baseIdentifier);

  // Set the base expanded identifier for PLR
  m_baseExpandedIdentifier << indet_field_value() << lumi_field_value() << plr_field_value();

  // Setup the hash tables
  if (init_hashes()) return(1);

  // Setup hash tables for finding neighbors
  if (init_neighbors()) return(1);
  ATH_MSG_DEBUG("PLR_ID::initialize_from_dict");
  ATH_MSG_DEBUG("Wafer range -> " << (std::string)m_full_wafer_range);
  ATH_MSG_DEBUG("Pixel range -> " << (std::string)m_full_pixel_range);
  
  return 0;
}


int
PLR_ID::initLevelsFromDict(void) {

  if (!m_dict) {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - dictionary NOT initialized");
    return(1);
  }

  // Find out which identifier field corresponds to each level. Use
  // names to find each field/leve.

  m_INDET_INDEX = 999;
  m_LUMI_INDEX = 999;
  m_PLR_INDEX = 999;
  m_BARREL_EC_INDEX = 999;
  m_LAYER_DISK_INDEX = 999;
  m_PHI_MODULE_INDEX = 999;
  m_ETA_MODULE_INDEX = 999;
  m_PHI_INDEX_INDEX = 999;
  m_ETA_INDEX_INDEX = 999;

  // Save index to a PIXEL region for unpacking
  ExpandedIdentifier id;
  id << indet_field_value() << lumi_field_value() << plr_field_value();
  if (m_dict->find_region(id, m_pixel_region_index)) {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find pixel region index: id, reg " << (std::string)id << " " << m_pixel_region_index);
   
    return(1);
  }

  // Get levels
  const IdDictField* field = m_dict->find_field("subdet");
  if (field) {
    m_INDET_INDEX = field->index();
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'subdet' field");
   
    return(1);
  }

  field = m_dict->find_field("part");
  if (field) {
    m_LUMI_INDEX = field->index();
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'part' field");
    return(1);
  }

  field = m_dict->find_field("PLR_or_BCM");
  if (field) {
    m_PLR_INDEX = field->index();
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'PLR_or_BCM' field");
    return(1);
  }

  field = m_dict->find_field("barrel_endcap");
  if (field) {
    m_BARREL_EC_INDEX = 3; //this will clash with another value
    if (m_BARREL_EC_INDEX != field->index()){
      ATH_MSG_INFO("Hardcoded value 3, field index " << field->index());
    }
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'barrel_endcap' field");
    return(1);
  }

  field = m_dict->find_field("layer");
  if (field) {
    m_LAYER_DISK_INDEX = 4;
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'layer' field");
    return(1);
  }

  field = m_dict->find_field("phi_module");
  if (field) {
    m_PHI_MODULE_INDEX = 5;
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'phi_module' field");
    return(1);
  }
  field = m_dict->find_field("eta_module");
  if (field) {
    
    m_ETA_MODULE_INDEX = 6;
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'eta_module' field");
    return(1);
  }
  field = m_dict->find_field("phi_index");
  if (field) {
   
    m_PHI_INDEX_INDEX = 7;
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'phi_index' field");
    return(1);
  }
  field = m_dict->find_field("eta_index");
  if (field) {
    
    m_ETA_INDEX_INDEX = 8;
  } else {
    ATH_MSG_FATAL("PLR_ID::initLevelsFromDict - unable to find 'eta_index' field");
    return(1);
  }

  // Set the field implementations: for bec, lay/disk, eta/phi mod
  // there are two kinds - shifted and non-shifted

  const IdDictRegion& region = m_dict->region(m_pixel_region_index);

  m_impl[kIndet] = region.implementation(m_INDET_INDEX);
  m_lumi_impl = region.implementation(m_LUMI_INDEX);
  m_plr_impl = region.implementation(m_PLR_INDEX);
  m_impl[kBec] = region.implementation(m_BARREL_EC_INDEX);
  m_impl[kLayDisk] = region.implementation(m_LAYER_DISK_INDEX);
  m_impl[kPhiMod] = region.implementation(m_PHI_MODULE_INDEX);
  m_impl[kEtaMod] = region.implementation(m_ETA_MODULE_INDEX);
  m_impl[kPhiIndex] = region.implementation(m_PHI_INDEX_INDEX);
  m_impl[kEtaIndex] = region.implementation(m_ETA_INDEX_INDEX);

  
  ATH_MSG_DEBUG("decode index and bit fields for each level:");
  ATH_MSG_DEBUG("indet          " << m_impl[kIndet].show_to_string());
  ATH_MSG_DEBUG("lumi           " << m_lumi_impl.show_to_string());
  ATH_MSG_DEBUG("plr            " << m_plr_impl.show_to_string());
  ATH_MSG_DEBUG("bec            " << m_impl[kBec].show_to_string());
  ATH_MSG_DEBUG("bec_shift      " << m_impl[kBecShift].show_to_string());
  ATH_MSG_DEBUG("lay_disk       " << m_impl[kLayDisk].show_to_string());
  ATH_MSG_DEBUG("lay_disk_shift " << m_impl[kLayDiskShift].show_to_string());
  ATH_MSG_DEBUG("phi_mod        " << m_impl[kPhiMod].show_to_string());
  ATH_MSG_DEBUG("phi_mod_shift  " << m_impl[kPhiModShift].show_to_string());
  ATH_MSG_DEBUG("eta_mod        " << m_impl[kEtaMod].show_to_string());
  ATH_MSG_DEBUG("eta_mod_shift  " << m_impl[kEtaModShift].show_to_string());
  ATH_MSG_DEBUG("phi_index      " << m_impl[kPhiIndex].show_to_string());
  ATH_MSG_DEBUG("eta_index      " << m_impl[kEtaIndex].show_to_string());
  ATH_MSG_DEBUG("bec_eta_mod    " << m_impl[kBecEtaMod].show_to_string());




  std::cout << "indet " << m_impl[kIndet].decode_index() << " "
            << (std::string) m_impl[kIndet].ored_field() << " "
            << std::hex << m_impl[kIndet].mask() << " "
            << m_impl[kIndet].zeroing_mask() << " "
            << std::dec << m_impl[kIndet].shift()
            << " " << m_impl[kIndet].bits() << " " << m_impl[kIndet].bits_offset() << " ";
  m_impl[kIndet].ored_field().show();
  std::cout << "lumi " << m_lumi_impl.decode_index() << " "
            << (std::string) m_lumi_impl.ored_field() << " "
            << std::hex << m_lumi_impl.mask() << " "
            << m_lumi_impl.zeroing_mask() << " "
            << std::dec << m_lumi_impl.shift()
            << " " << m_lumi_impl.bits() << " " << m_lumi_impl.bits_offset() << " ";
  m_lumi_impl.ored_field().show();
  std::cout << "plr " << m_plr_impl.decode_index() << " "
            << (std::string) m_plr_impl.ored_field() << " "
            << std::hex << m_plr_impl.mask() << " "
            << m_plr_impl.zeroing_mask() << " "
            << std::dec << m_plr_impl.shift()
            << " " << m_plr_impl.bits() << " " << m_plr_impl.bits_offset() << " ";
  m_plr_impl.ored_field().show();
  std::cout << "bec " << m_impl[kBec].decode_index() << " "
            << (std::string) m_impl[kBec].ored_field() << " "
            << std::hex << m_impl[kBec].mask() << " "
            << m_impl[kBec].zeroing_mask() << " "
            << std::dec << m_impl[kBec].shift()
            << " " << m_impl[kBec].bits() << " " << m_impl[kBec].bits_offset() << " ";
  m_impl[kBec].ored_field().show();
  std::cout << "bec_shift " << m_impl[kBecShift].decode_index() << " "
            << (std::string) m_impl[kBecShift].ored_field() << " "
            << std::hex << m_impl[kBecShift].mask() << " "
            << m_impl[kBecShift].zeroing_mask() << " "
            << std::dec << m_impl[kBecShift].shift()
            << " " << m_impl[kBecShift].bits() << " " << m_impl[kBecShift].bits_offset() << " ";
  m_impl[kBecShift].ored_field().show();
  std::cout << "lay_disk " << m_impl[kLayDisk].decode_index() << " "
            << (std::string) m_impl[kLayDisk].ored_field() << " "
            << std::hex << m_impl[kLayDisk].mask() << " "
            << m_impl[kLayDisk].zeroing_mask() << " "
            << std::dec << m_impl[kLayDisk].shift()
            << " " << m_impl[kLayDisk].bits() << " " << m_impl[kLayDisk].bits_offset() << " ";
  m_impl[kLayDisk].ored_field().show();
  std::cout << "lay_disk_shift " << m_impl[kLayDiskShift].decode_index() << " "
            << (std::string) m_impl[kLayDiskShift].ored_field() << " "
            << std::hex << m_impl[kLayDiskShift].mask() << " "
            << m_impl[kLayDiskShift].zeroing_mask() << " "
            << std::dec << m_impl[kLayDiskShift].shift()
            << " " << m_impl[kLayDiskShift].bits() << " " << m_impl[kLayDiskShift].bits_offset() << " ";
  m_impl[kLayDiskShift].ored_field().show();
  std::cout << "phi_mod " << m_impl[kPhiMod].decode_index() << " "
            << (std::string) m_impl[kPhiMod].ored_field() << " "
            << std::hex << m_impl[kPhiMod].mask() << " "
            << m_impl[kPhiMod].zeroing_mask() << " "
            << std::dec << m_impl[kPhiMod].shift()
            << " " << m_impl[kPhiMod].bits() << " " << m_impl[kPhiMod].bits_offset() << " ";
  m_impl[kPhiMod].ored_field().show();
  std::cout << "phi_mod_shift " << m_impl[kPhiModShift].decode_index() << " "
            << (std::string) m_impl[kPhiModShift].ored_field() << " "
            << std::hex << m_impl[kPhiModShift].mask() << " "
            << m_impl[kPhiModShift].zeroing_mask() << " "
            << std::dec << m_impl[kPhiModShift].shift()
            << " " << m_impl[kPhiModShift].bits() << " " << m_impl[kPhiModShift].bits_offset() << " ";
  m_impl[kPhiModShift].ored_field().show();
  std::cout << "eta_mod " << m_impl[kEtaMod].decode_index() << " "
            << (std::string) m_impl[kEtaMod].ored_field() << " "
            << std::hex << m_impl[kEtaMod].mask() << " "
            << m_impl[kEtaMod].zeroing_mask() << " "
            << std::dec << m_impl[kEtaMod].shift()
            << " " << m_impl[kEtaMod].bits() << " " << m_impl[kEtaMod].bits_offset() << " ";
  m_impl[kEtaMod].ored_field().show();
  std::cout << "eta_mod_shift " << m_impl[kEtaModShift].decode_index() << " "
            << (std::string) m_impl[kEtaModShift].ored_field() << " "
            << std::hex << m_impl[kEtaModShift].mask() << " "
            << m_impl[kEtaModShift].zeroing_mask() << " "
            << std::dec << m_impl[kEtaModShift].shift()
            << " " << m_impl[kEtaModShift].bits() << " " << m_impl[kEtaModShift].bits_offset() << " ";
  m_impl[kEtaModShift].ored_field().show();
  std::cout << "phi_index " << m_impl[kPhiIndex].decode_index() << " "
            << (std::string) m_impl[kPhiIndex].ored_field() << " "
            << std::hex << m_impl[kPhiIndex].mask() << " "
            << m_impl[kPhiIndex].zeroing_mask() << " "
            << std::dec << m_impl[kPhiIndex].shift()
            << " " << m_impl[kPhiIndex].bits() << " " << m_impl[kPhiIndex].bits_offset() << " ";
  m_impl[kPhiIndex].ored_field().show();
  std::cout << "eta_index " << m_impl[kEtaIndex].decode_index() << " "
            << (std::string) m_impl[kEtaIndex].ored_field() << " "
            << std::hex << m_impl[kEtaIndex].mask() << " "
            << m_impl[kEtaIndex].zeroing_mask() << " "
            << std::dec << m_impl[kEtaIndex].shift()
            << " " << m_impl[kEtaIndex].bits() << " " << m_impl[kEtaIndex].bits_offset() << " ";
  m_impl[kEtaIndex].ored_field().show();
  std::cout << "bec_eta_mod " << m_impl[kBecEtaMod].decode_index() << " "
            << (std::string) m_impl[kBecEtaMod].ored_field() << " "
            << std::hex << m_impl[kBecEtaMod].mask() << " "
            << m_impl[kBecEtaMod].zeroing_mask() << " "
            << std::dec << m_impl[kBecEtaMod].shift()
            << " " << m_impl[kBecEtaMod].bits() << " " << m_impl[kBecEtaMod].bits_offset() << " ";
  m_impl[kBecEtaMod].ored_field().show();


  std::cout << "PLR_ID::initLevelsFromDict - found levels " << std::endl;
  std::cout << "subdet        " << m_INDET_INDEX << std::endl;
  std::cout << "part          " << m_LUMI_INDEX << std::endl;
  std::cout << "plr           " << m_PLR_INDEX << std::endl;
  std::cout << "barrel_endcap " << m_BARREL_EC_INDEX << std::endl;
  std::cout << "layer or disk " << m_LAYER_DISK_INDEX << std::endl;
  std::cout << "phi_module    " << m_PHI_MODULE_INDEX << std::endl;
  std::cout << "eta_module    " << m_ETA_MODULE_INDEX << std::endl;
  std::cout << "phi_index     " << m_PHI_INDEX_INDEX << std::endl;
  std::cout << "eta_index     " << m_ETA_INDEX_INDEX << std::endl;

  return(0);
}
