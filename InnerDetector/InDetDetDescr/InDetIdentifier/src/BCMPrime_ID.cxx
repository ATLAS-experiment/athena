/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/BCMPrime_ID.h"
#include "PixelOutputFormatting.h"

#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"

using InDetIdentifierPkg::formatOutput;

BCMPrime_ID::BCMPrime_ID(const std::string& name, const std::string& group)
  : PixelID(name, group)
{
  m_BARREL_EC_INDEX = 3;
  m_LAYER_DISK_INDEX = 4;
  m_PHI_MODULE_INDEX = 5;
  m_ETA_MODULE_INDEX = 6;
  m_PHI_INDEX_INDEX = 7;
  m_ETA_INDEX_INDEX = 8;
}

int BCMPrime_ID::bcm_field_value() const
{
  int value = 0;
  if (m_dict && m_dict->get_label_value("PLR_or_BCM", "BCM", value) == 0) {
    return value;
  }
  return AtlasDetectorID::invalidId;
}

int BCMPrime_ID::initialize_from_dictionary(const IdDictMgr& dict_mgr)
{
  ATH_MSG_INFO("Initialize from dictionary");
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
    return 0;
  }

  if (PixelID::initialize_from_dictionary(dict_mgr)) return 1;
  if (register_dict_tag(dict_mgr, "InnerDetector")) return 1;

  m_dict = dict_mgr.find_dictionary("InnerDetector");
  if (!m_dict) {
    ATH_MSG_FATAL("BCMPrime_ID::initialize_from_dictionary - cannot access InnerDetector dictionary");
    return 1;
  }

  AtlasDetectorID::setDictVersion(dict_mgr, "InnerDetector");
  if (initLevelsFromDict()) return 1;

  m_pixel_id = lumi();
  if (!is_lumi(m_pixel_id)) {
    ATH_MSG_FATAL("BCMPrime_ID::initialize_from_dictionary - cannot get luminosity detector dictionary");
    return 1;
  }

  int barrel_value{};
  m_barrel_field.clear();
  if (m_dict->get_label_value("barrel_endcap", "barrel", barrel_value)) {
    ATH_MSG_FATAL("Could not get value for label 'barrel' of field 'barrel_endcap' in dictionary " << m_dict->name());
    return 1;
  }
  m_barrel_field.add_value(barrel_value);
  m_barrel_field.add_value(barrel_value);

  int dbm_value{};
  m_dbm_field.clear();
  if (m_dict->get_label_value("barrel_endcap", "negative_dbm", dbm_value)) {
    m_dbm_field.add_value(-999);
  } else {
    m_dbm_field.add_value(dbm_value);
  }
  if (m_dict->get_label_value("barrel_endcap", "positive_dbm", dbm_value)) {
    m_dbm_field.add_value(999);
  } else {
    m_dbm_field.add_value(dbm_value);
  }

  const IdDictDictionary* atlasDict = dict_mgr.find_dictionary("ATLAS");
  int inDetField = -1;
  if (atlasDict->get_label_value("subdet", "InnerDetector", inDetField)) {
    ATH_MSG_FATAL("Could not get value for label 'InnerDetector' of field 'subdet' in dictionary " << atlasDict->name());
    return 1;
  }

  int lumiField = -1;
  if (m_dict->get_label_value("part", "LuminosityDetectors", lumiField)) {
    ATH_MSG_FATAL("Could not get value for label 'LuminosityDetectors' of field 'part' in dictionary " << m_dict->name());
    return 1;
  }

  int bcmField = -1;
  if (m_dict->get_label_value("PLR_or_BCM", "BCM", bcmField)) {
    ATH_MSG_FATAL("Could not get value for label 'BCM' of field 'PLR_or_BCM' in dictionary " << m_dict->name());
    return 1;
  }

  ATH_MSG_DEBUG("BCMPrime_ID::initialize_from_dictionary Found field values: InDet/LuminosityDetectors/BCM "
                << inDetField << "/" << lumiField << "/" << bcmField);

  ExpandedIdentifier region_id;
  region_id.add(inDetField);
  region_id.add(lumiField);
  region_id.add(bcmField);
  Range prefix;
  m_full_wafer_range = m_dict->build_multirange(region_id, prefix, "eta_module");
  m_full_pixel_range = m_dict->build_multirange(region_id, prefix);

  m_baseIdentifier = Identifier::value_type{0};
  m_impl[kIndet].pack(indet_field_value(), m_baseIdentifier);
  m_lumi_impl.pack(lumi_field_value(), m_baseIdentifier);
  m_bcm_impl.pack(bcm_field_value(), m_baseIdentifier);

  m_baseExpandedIdentifier << indet_field_value() << lumi_field_value() << bcm_field_value();

  if (init_hashes()) return 1;
  if (init_neighbors()) return 1;

  ATH_MSG_DEBUG("BCMPrime_ID wafer range -> " << static_cast<std::string>(m_full_wafer_range));
  ATH_MSG_DEBUG("BCMPrime_ID pixel range -> " << static_cast<std::string>(m_full_pixel_range));
  return 0;
}

int BCMPrime_ID::initLevelsFromDict()
{
  if (!m_dict) {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - dictionary NOT initialized");
    return 1;
  }

  m_INDET_INDEX = 999;
  m_LUMI_INDEX = 999;
  m_BCM_INDEX = 999;
  m_BARREL_EC_INDEX = 999;
  m_LAYER_DISK_INDEX = 999;
  m_PHI_MODULE_INDEX = 999;
  m_ETA_MODULE_INDEX = 999;
  m_PHI_INDEX_INDEX = 999;
  m_ETA_INDEX_INDEX = 999;

  ExpandedIdentifier id;
  id << indet_field_value() << lumi_field_value() << bcm_field_value();
  if (m_dict->find_region(id, m_pixel_region_index)) {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find BCMPrime region index: id, reg "
                  << static_cast<std::string>(id) << " " << m_pixel_region_index);
    return 1;
  }

  const IdDictField* field = m_dict->find_field("subdet");
  if (field) {
    m_INDET_INDEX = field->index();
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'subdet' field");
    return 1;
  }

  field = m_dict->find_field("part");
  if (field) {
    m_LUMI_INDEX = field->index();
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'part' field");
    return 1;
  }

  field = m_dict->find_field("PLR_or_BCM");
  if (field) {
    m_BCM_INDEX = field->index();
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'PLR_or_BCM' field");
    return 1;
  }

  field = m_dict->find_field("barrel_endcap");
  if (field) {
    m_BARREL_EC_INDEX = 3;
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'barrel_endcap' field");
    return 1;
  }

  field = m_dict->find_field("layer");
  if (field) {
    m_LAYER_DISK_INDEX = 4;
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'layer' field");
    return 1;
  }

  field = m_dict->find_field("phi_module");
  if (field) {
    m_PHI_MODULE_INDEX = 5;
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'phi_module' field");
    return 1;
  }

  field = m_dict->find_field("eta_module");
  if (field) {
    m_ETA_MODULE_INDEX = 6;
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'eta_module' field");
    return 1;
  }

  field = m_dict->find_field("phi_index");
  if (field) {
    m_PHI_INDEX_INDEX = 7;
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'phi_index' field");
    return 1;
  }

  field = m_dict->find_field("eta_index");
  if (field) {
    m_ETA_INDEX_INDEX = 8;
  } else {
    ATH_MSG_FATAL("BCMPrime_ID::initLevelsFromDict - unable to find 'eta_index' field");
    return 1;
  }

  const IdDictRegion& region = m_dict->region(m_pixel_region_index);
  m_impl[kIndet] = region.implementation(m_INDET_INDEX);
  m_lumi_impl = region.implementation(m_LUMI_INDEX);
  m_bcm_impl = region.implementation(m_BCM_INDEX);
  m_impl[kBec] = region.implementation(m_BARREL_EC_INDEX);
  m_impl[kLayDisk] = region.implementation(m_LAYER_DISK_INDEX);
  m_impl[kPhiMod] = region.implementation(m_PHI_MODULE_INDEX);
  m_impl[kEtaMod] = region.implementation(m_ETA_MODULE_INDEX);
  m_impl[kPhiIndex] = region.implementation(m_PHI_INDEX_INDEX);
  m_impl[kEtaIndex] = region.implementation(m_ETA_INDEX_INDEX);

  if (msgLvl(MSG::DEBUG)) {
    msg() << formatOutput("indet ", m_impl[kIndet]);
    m_impl[kIndet].ored_field().show(msg());
    msg() << formatOutput("lumi ", m_lumi_impl);
    m_lumi_impl.ored_field().show(msg());
    msg() << formatOutput("bcm ", m_bcm_impl);
    m_bcm_impl.ored_field().show(msg());
  }

  return 0;
}
