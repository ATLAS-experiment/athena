/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/***************************************************************************
   Inner Detector identifier package
   -------------------------------------------
***************************************************************************/


#include "GaudiKernel/MsgStream.h"

#include "InDetIdentifier/TRT_ID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/RangeIterator.h"
#include <set>
#include <algorithm>
#include <iostream>
#include  <cassert>


/////////////////////////////////////////////////////////////////////////////

TRT_ID::TRT_ID()
  :
  AtlasDetectorID("TRT_ID", "trt"),
  m_is_valid(true),
  m_trt_region_index(0),
  m_INDET_INDEX(0),
  m_TRT_INDEX(1),
  m_BARREL_EC_INDEX(2),
  m_PHI_MODULE_INDEX(3),
  m_LAYER_OR_WHEEL_INDEX(4),
  m_STRAW_LAYER_INDEX(5),
  m_STRAW_INDEX(6),
  m_dict(nullptr),
  m_module_hash_max(0),
  m_straw_layer_hash_max(0),
  m_straw_hash_max(0) {
  m_barrel_field.add_value(-1);
  m_barrel_field.add_value(1);
}

TRT_ID::~TRT_ID() = default;

void
TRT_ID::barrel_ec_id_checks(int barrel_ec) const {
  // Check that id is within allowed range

  // Fill expanded id
  ExpandedIdentifier id;

  id << indet_field_value() << trt_field_value() << barrel_ec;

  if (!m_full_module_range.match(id)) {  // module range check is sufficient
    MsgStream log(m_msgSvc, "TRT_ID");
    log << MSG::ERROR << " TRT_ID::barrel_ec_id result is NOT ok. ID, range "
          << (std::string) id << " "
          << (std::string) m_full_module_range << endmsg;
  }
}

void
TRT_ID::module_id_checks(int barrel_ec,
                         int phi_module,
                         int layer_or_wheel) const {
  // Check that id is within allowed range

  // Fill expanded id
  ExpandedIdentifier id;

  id << indet_field_value() << trt_field_value()
     << barrel_ec << phi_module << layer_or_wheel;

  if (!m_full_module_range.match(id)) {  // module range check is sufficient
    MsgStream log(m_msgSvc, "TRT_ID");
    log << MSG::ERROR << " TRT_ID::module_id result is NOT ok. ID, range "
          << (std::string) id << " "
          << (std::string) m_full_module_range << endmsg;
  }
}

void
TRT_ID::straw_id_checks(int barrel_ec,
                        int phi_module,
                        int layer_or_wheel,
                        int straw_layer,
                        int straw) const {
  // Check that id is within allowed range

  // Build identifier
  ExpandedIdentifier id;

  id << indet_field_value() << trt_field_value()
     << barrel_ec << phi_module << layer_or_wheel << straw_layer << straw;

  if (!m_full_straw_range.match(id)) {
    MsgStream log(m_msgSvc, "TRT_ID");
    log << MSG::ERROR << " TRT_ID::straw_id result is NOT ok. ID, range "
          << (std::string) id << " "
          << (std::string) m_full_straw_range << endmsg;
  }
}

void
TRT_ID::layer_id_checks(int barrel_ec,
                        int phi_module,
                        int layer_or_wheel,
                        int straw_layer) const {
  // Check that id is within allowed range

  // Build identifier
  ExpandedIdentifier id;

  id << indet_field_value() << trt_field_value()
     << barrel_ec << phi_module << layer_or_wheel << straw_layer;

  if (!m_full_straw_layer_range.match(id)) {
    MsgStream log(m_msgSvc, "TRT_ID");
    log << MSG::ERROR << " TRT_ID::layer_id result is NOT ok. ID, range "
          << (std::string) id << " "
          << (std::string) m_full_straw_layer_range << endmsg;
    
  }
}


int
TRT_ID::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
  MsgStream log(m_msgSvc, "TRT_ID");
  log << MSG::INFO << "Initialize from dictionary msgSvc " << m_msgSvc << endmsg;
  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    log << MSG::INFO << "Request to reinitialize not satisfied - tags have not changed" << endmsg;
    return(0);
  } else {
      log << MSG::DEBUG << "(Re)initialize" << endmsg;
  }

  // init base object
  if (AtlasDetectorID::initialize_from_dictionary(dict_mgr)) {
    m_is_valid = false;
    return(1);
  }


  // Register version of InnerDetector dictionary
  if (register_dict_tag(dict_mgr, "InnerDetector")) {
    m_is_valid = false;
    return(1);
  }


  m_dict = dict_mgr.find_dictionary("InnerDetector");
  if (!m_dict) {
    log << MSG::ERROR << " TRT_ID::initialize_from_dict - cannot access InnerDetector dictionary "
          << endmsg;
    m_is_valid = false;
    return 1;
  }

  // Initialize the field indices
  if (initLevelsFromDict()) {
    m_is_valid = false;
    return(1);
  }


  //
  // Set barrel field for testing is_barrel
  //
  int barrel_value;
  m_barrel_field.clear();
  // negative barrel
  if (m_dict->get_label_value("barrel_endcap", "negative_barrel", barrel_value)) {
    log << MSG::ERROR << "Could not get value for label 'barrel' of field 'barrel_endcap' in dictionary "
          << m_dict->name()
          << endmsg;
    
    m_is_valid = false;
    return(1);
  }
  m_barrel_field.add_value(barrel_value);
  // negative barrel
  if (m_dict->get_label_value("barrel_endcap", "positive_barrel", barrel_value)) {
    log << MSG::ERROR << "Could not get value for label 'barrel' of field 'barrel_endcap' in dictionary "
        << m_dict->name()
          << endmsg;
   
    m_is_valid = false;
    return(1);
  }
  m_barrel_field.add_value(barrel_value);
    log << MSG::DEBUG << " TRT_ID::initialize_from_dict "
        << "Set barrel field values: "
        << (std::string) m_barrel_field
        << endmsg;
 

  //
  // Build multirange for the valid set of identifiers
  //


  // Find value for the field InnerDetector
  const IdDictDictionary* atlasDict = dict_mgr.find_dictionary("ATLAS");

  int inDetField = -1;
  if (atlasDict->get_label_value("subdet", "InnerDetector", inDetField)) {
    log << MSG::ERROR << "Could not get value for label 'InnerDetector' of field 'subdet' in dictionary "
          << atlasDict->name()
          << endmsg;
   
    m_is_valid = false;
    return(1);
  }

  // Find value for the field TRT
  int trtField = -1;
  if (m_dict->get_label_value("part", "TRT", trtField)) {
    log << MSG::ERROR << "Could not get value for label 'TRT' of field 'part' in dictionary "
          << m_dict->name()
          << endmsg;
   
    m_is_valid = false;
    return(1);
  }
  log << MSG::DEBUG << " TRT_ID::initialize_from_dict "
        << "Found field values: InDet/TRT "
        << inDetField << "/"
        << trtField
        << endmsg;
  

  // Set up id for region and range prefix
  ExpandedIdentifier region_id;
  region_id.add(inDetField);
  region_id.add(trtField);
  Range prefix;
  m_full_module_range = m_dict->build_multirange(region_id, prefix, "layer_or_wheel");
  m_full_straw_layer_range = m_dict->build_multirange(region_id, prefix, "straw_layer");
  m_full_straw_range = m_dict->build_multirange(region_id, prefix);

  init_hashes();

  log << MSG::DEBUG << " TRT_ID::initialize_from_dict "
        << endmsg;
  log << MSG::DEBUG << "Module range -> " << (std::string) m_full_module_range
        << endmsg;
  log << MSG::DEBUG << "Straw layer range -> " << (std::string) m_full_straw_layer_range
        << endmsg;
  log << MSG::DEBUG << "Straw range -> " << (std::string) m_full_straw_range
        << endmsg;
  

  return 0;
}

void
TRT_ID::init_hashes() {
  //
  // create two vecs (module and straw layer) to retrieve the hashes
  // for compact ids
  //

  // module hash
  MsgStream log(m_msgSvc, "TRT_ID");

    log << MSG::DEBUG << "Module range -> " << (std::string) m_full_module_range
        << endmsg;
  



  m_module_hash_max = m_full_module_range.cardinality();
  m_module_vec.resize(m_module_hash_max);
  unsigned int nids = 0;
  std::set<Identifier> ids;
  for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
    const Range& range = m_full_module_range[i];
    ConstRangeIterator rit(range);
    for (const auto & exp_id: rit) {
      Identifier id = module_id(exp_id[m_BARREL_EC_INDEX],
                                exp_id[m_PHI_MODULE_INDEX],
                                exp_id[m_LAYER_OR_WHEEL_INDEX]);
      if (!(ids.insert(id)).second) {
        log << MSG::ERROR << " TRT_ID::init_hashes "
              << " Error: duplicated id for module id. nid " << nids
              << " compact id " << show_to_string(id)
              << " id " << std::string(exp_id)
              << endmsg;
        
      }
      nids++;
    }
  }
  if (ids.size() != m_module_hash_max) {
      log << MSG::ERROR << " TRT_ID::init_hashes "
          << " Error: set size NOT EQUAL to hash max. size " << ids.size()
          << " hash max " << m_module_hash_max
          << endmsg;
   
  } else {
      log << MSG::DEBUG << " TRT_ID::init_hashes "
          << " module hash max " << m_module_hash_max
          << endmsg;
    
  }

  nids = 0;
  std::set<Identifier>::const_iterator first = ids.begin();
  std::set<Identifier>::const_iterator last = ids.end();
  for (; first != last && nids < m_module_vec.size(); ++first) {
    m_module_vec[nids] = (*first);
    nids++;
  }

  // straw_layer hash
  m_straw_layer_hash_max = m_full_straw_layer_range.cardinality();
  m_straw_layer_vec.resize(m_straw_layer_hash_max);
  nids = 0;
  ids.clear();
  for (unsigned int i = 0; i < m_full_straw_layer_range.size(); ++i) {
    const Range& range = m_full_straw_layer_range[i];
    ConstRangeIterator rit(range);
    for (const auto & exp_id: rit) {
      Identifier id = layer_id(exp_id[m_BARREL_EC_INDEX],
                               exp_id[m_PHI_MODULE_INDEX],
                               exp_id[m_LAYER_OR_WHEEL_INDEX],
                               exp_id[m_STRAW_LAYER_INDEX]);
      if (!(ids.insert(id)).second) {
        log << MSG::ERROR << " TRT_ID::init_hashes "
              << " Error: duplicated id for straw layer id. nid " << nids
              << " compact id " << show_to_string(id)
              << " id " << std::string(exp_id)
              << endmsg;

        nids++;
      }
    }
  }

  if (ids.size() != m_straw_layer_hash_max) {
    log << MSG::ERROR << " TRT_ID::init_hashes "
          << " Error: set size NOT EQUAL to hash max. size " << ids.size()
          << " hash max " << m_straw_layer_hash_max
          << endmsg;
  } else {
    log << MSG::DEBUG << " TRT_ID::init_hashes "
          << " straw layer hash max " << m_straw_layer_hash_max
          << endmsg;
    
  }

  nids = 0;
  first = ids.begin();
  last = ids.end();
  for (; first != last && nids < m_straw_layer_vec.size(); ++first) {
    m_straw_layer_vec[nids] = (*first);
    nids++;
  }


  // Init vector for binary lookup
  init_straw_hash_vector();


  // Setup for hash calculation for straw ids


  // We use a HashCalc object to save the information needed to
  // calculate the hash from the decoded values of a straw id.
  //
  // The number of HashCalc objects needed are 2x the number of
  // ranges - one for each pos/neg. There are then two additional
  // lookup tables, one for each of barrel and endcap, to access the
  // index into the HashCalc table. Here we set up the HashCalc
  // table and the additional tables for accessing the index.

  // Set up vector as lookup table for hash calculation.
  unsigned int hcIndex = 0;
  unsigned int nRanges = m_full_straw_range.size();
  m_hash_calcs.resize(2 * nRanges); // allow for pos/neg

//     std::vector<bool> startHashFound(4,false);
//     m_beStartHash.resize(4);


  for (unsigned int i = 0; i < m_full_straw_range.size(); ++i) {
    const Range* range = &m_full_straw_range[i];
    ExpandedIdentifier exp_id = range->minimum();
    Identifier min = straw_id(exp_id[m_BARREL_EC_INDEX],
                              exp_id[m_PHI_MODULE_INDEX],
                              exp_id[m_LAYER_OR_WHEEL_INDEX],
                              exp_id[m_STRAW_LAYER_INDEX],
                              exp_id[m_STRAW_INDEX]);
    IdentifierHash min_neg = straw_hash_bin(min);

    // Calculate the offset in phi
    Identifier next_phi = straw_id(exp_id[m_BARREL_EC_INDEX],
                                   exp_id[m_PHI_MODULE_INDEX] + 1,
                                   exp_id[m_LAYER_OR_WHEEL_INDEX],
                                   exp_id[m_STRAW_LAYER_INDEX],
                                   exp_id[m_STRAW_INDEX]);
    IdentifierHash deltaPhi = straw_hash_bin(next_phi) - min_neg;

    // barrel or endcap
    bool isBarrel = m_barrel_field.match(exp_id[m_BARREL_EC_INDEX]);


    // both pos and neg available?
    bool two_sides = ((*range)[m_BARREL_EC_INDEX].get_indices() == 2);

    // fill lookup table with index to HashCalc
    unsigned int lay_min = (*range)[m_LAYER_OR_WHEEL_INDEX].get_minimum();
    unsigned int nlayers = (*range)[m_LAYER_OR_WHEEL_INDEX].get_indices();
    unsigned int str_lay_min = (*range)[m_STRAW_LAYER_INDEX].get_minimum();
    unsigned int nstr_layers = (*range)[m_STRAW_LAYER_INDEX].get_indices();
    unsigned int start;
    unsigned int end;

    if (isBarrel) {
      // use layer and straw layer to distinguish which HashCalc
      start = lay_min * 32 + str_lay_min;
      end = start + nstr_layers;
      if (m_hash_calcs_barrel_indexes.size() < end) m_hash_calcs_barrel_indexes.resize(end);
      for (unsigned int i = start; i < end; ++i) {
        m_hash_calcs_barrel_indexes[i].first = hcIndex;
      }

    } else {
      // use layer alone to distinguish which HashCalc
      start = lay_min;
      end = start + nlayers;
      if (m_hash_calcs_endcap_indexes.size() < end) m_hash_calcs_endcap_indexes.resize(end);
      for (unsigned int i = start; i < end; ++i) {
        m_hash_calcs_endcap_indexes[i].first = hcIndex;
      }

    }


    // Set up HashCalc objet
    HashCalc hc;
    hc.m_hash = min_neg;
//  hc.m_barrel_ec    = exp_id[m_BARREL_EC_INDEX];
    hc.m_deltaPhi = deltaPhi;
    hc.m_layerMin = (*range)[m_LAYER_OR_WHEEL_INDEX].get_minimum();
    hc.m_strLayerMin = (*range)[m_STRAW_LAYER_INDEX].get_minimum();

    hc.m_nStrawLayers = (*range)[m_STRAW_LAYER_INDEX].get_indices() *
                        (*range)[m_STRAW_INDEX].get_indices();
    hc.m_nstraws = (*range)[m_STRAW_INDEX].get_indices();


    m_hash_calcs[hcIndex] = hc;
    hcIndex++;

    // Now repeat if there are both pos/neg

    if (two_sides) {
      if (isBarrel) {
        // use layer and straw layer to distinguish which HashCalc
        start = lay_min * 32 + str_lay_min;
        end = start + nstr_layers;
        if (m_hash_calcs_barrel_indexes.size() < end) m_hash_calcs_barrel_indexes.resize(end);
        for (unsigned int i = start; i < end; ++i) {
          m_hash_calcs_barrel_indexes[i].second = hcIndex;
        }

      } else {
        // use layer alone to distinguish which HashCalc
        start = lay_min;
        end = start + nlayers;
        if (m_hash_calcs_endcap_indexes.size() < end) m_hash_calcs_endcap_indexes.resize(end);
        for (unsigned int i = start; i < end; ++i) {
          m_hash_calcs_endcap_indexes[i].second = hcIndex;
        }

      }



      // Flip sign for bec if 2 symmetric sides (ATLAS case, not TB)
      Identifier min = straw_id(-exp_id[m_BARREL_EC_INDEX],
                                exp_id[m_PHI_MODULE_INDEX],
                                exp_id[m_LAYER_OR_WHEEL_INDEX],
                                exp_id[m_STRAW_LAYER_INDEX],
                                exp_id[m_STRAW_INDEX]);
      IdentifierHash min_pos = straw_hash_bin(min);
      hc.m_hash = min_pos;
      m_hash_calcs[hcIndex] = hc;
      hcIndex++;
    }
  }

  // clear vector
  reset_straw_hash_vector();
}

void
TRT_ID::reset_straw_hash_vector() {
  m_straw_vec.clear();
}

void
TRT_ID::init_straw_hash_vector() {
  // We init straw hashes separately to be able to reset the vector
  // afterwards

  MsgStream log(m_msgSvc, "TRT_ID");

  // straw hash - we do not keep a vec for the straws - too large
  m_straw_hash_max = m_full_straw_range.cardinality();
  m_straw_vec.resize(m_straw_hash_max);
  unsigned int nids = 0;
  std::set<Identifier> ids;

  for (unsigned int i = 0; i < m_full_straw_range.size(); ++i) {
    const Range& range = m_full_straw_range[i];
    ConstRangeIterator rit(range);
    for (const auto & exp_id:rit) {
      Identifier id = straw_id(exp_id[m_BARREL_EC_INDEX],
                               exp_id[m_PHI_MODULE_INDEX],
                               exp_id[m_LAYER_OR_WHEEL_INDEX],
                               exp_id[m_STRAW_LAYER_INDEX],
                               exp_id[m_STRAW_INDEX]);
      if (!(ids.insert(id)).second) {
        log << MSG::ERROR << " TRT_ID::init_hashes "
              << " Error: duplicated id for straw id. nid " << nids
              << " compact id " << show_to_string(id)
              << " id " << std::string(exp_id)
              << endmsg;
        
      }
      nids++;
    }
  }
  if (ids.size() != m_straw_hash_max) {
      log << MSG::ERROR << " TRT_ID::init_hashes "
          << " Error: set size NOT EQUAL to hash max. size " << ids.size()
          << " hash max " << m_straw_hash_max
          << endmsg;
    
  } else {
      log << MSG::DEBUG << " TRT_ID::init_hashes "
          << " straw hash max " << m_straw_hash_max
          << endmsg;
    
  }

  nids = 0;
  std::set<Identifier>::const_iterator first = ids.begin();
  std::set<Identifier>::const_iterator last = ids.end();
  for (; first != last && nids < m_straw_vec.size(); ++first) {
    m_straw_vec[nids] = (*first);
    nids++;
  }
}

int
TRT_ID::initLevelsFromDict() {
  MsgStream log(m_msgSvc, "TRT_ID");

  if (!m_dict) {
      log << MSG::ERROR << " TRT_ID::initLevelsFromDict - dictionary NOT initialized "
          << endmsg;
    
    return(1);
  }

  // Find out which identifier field corresponds to each level. Use
  // names to find each field/leve.

  m_INDET_INDEX = 999;
  m_TRT_INDEX = 999;
  m_BARREL_EC_INDEX = 999;
  m_PHI_MODULE_INDEX = 999;
  m_LAYER_OR_WHEEL_INDEX = 999;
  m_STRAW_LAYER_INDEX = 999;
  m_STRAW_INDEX = 999;

  // Save index to a TRT region for unpacking
  ExpandedIdentifier id;
  id << indet_field_value() << trt_field_value();
  if (m_dict->find_region(id, m_trt_region_index)) {
      log << MSG::WARNING << "TRT_ID::initLevelsFromDict - unable to initialize TRT_ID helper "
          << endmsg;
      log << MSG::WARNING << "TRT_ID::initLevelsFromDict - we assume that the TRT does NOT exist for this layout "
          << endmsg;
    
    return(1);
  }

  // Find a TRT region
  IdDictField* field = m_dict->find_field("subdet");
  if (field) {
    m_INDET_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'subdet' field "
          << endmsg;
    
    return(1);
  }
  field = m_dict->find_field("part");
  if (field) {
    m_TRT_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'part' field "
          << endmsg;
    
    return(1);
  }
  field = m_dict->find_field("barrel_endcap");
  if (field) {
    m_BARREL_EC_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'barrel_endcap' field "
          << endmsg;
    

    return(1);
  }
  field = m_dict->find_field("phi_sector");
  if (field) {
    m_PHI_MODULE_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'phi_sector' field "
          << endmsg;
    

    return(1);
  }
  field = m_dict->find_field("layer_or_wheel");
  if (field) {
    m_LAYER_OR_WHEEL_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'layer' field "
          << endmsg;
    

    return(1);
  }
  field = m_dict->find_field("straw_layer");
  if (field) {
    m_STRAW_LAYER_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'straw_layer' field "
          << endmsg;
    

    return(1);
  }
  field = m_dict->find_field("straw");
  if (field) {
    m_STRAW_INDEX = field->index();
  } else {
      log << MSG::ERROR << "TRT_ID::initLevelsFromDict - unable to find 'straw' field "
          << endmsg;
    

    return(1);
  }

  // Set the field implementations: for bec, lay/disk, eta/phi mod

  const IdDictRegion& region = m_dict->region(m_trt_region_index);

  m_indet_impl = region.implementation(m_INDET_INDEX);
  m_trt_impl = region.implementation(m_TRT_INDEX);
  m_bec_impl = region.implementation(m_BARREL_EC_INDEX);
  m_phi_mod_impl = region.implementation(m_PHI_MODULE_INDEX);
  m_lay_wheel_impl = region.implementation(m_LAYER_OR_WHEEL_INDEX);
  m_str_lay_impl = region.implementation(m_STRAW_LAYER_INDEX);
  m_straw_impl = region.implementation(m_STRAW_INDEX);

    log << MSG::DEBUG << "decode index and bit fields for each level: " << endmsg;
    log << MSG::DEBUG << "indet     " << m_indet_impl.show_to_string() << endmsg;
    log << MSG::DEBUG << "trt       " << m_trt_impl.show_to_string() << endmsg;
    log << MSG::DEBUG << "bec       " << m_bec_impl.show_to_string() << endmsg;
    log << MSG::DEBUG << "phi_mod   " << m_phi_mod_impl.show_to_string() << endmsg;
    log << MSG::DEBUG << "lay_wheel " << m_lay_wheel_impl.show_to_string() << endmsg;
    log << MSG::DEBUG << "str_lay   " << m_str_lay_impl.show_to_string() << endmsg;
    log << MSG::DEBUG << "straw     " << m_straw_impl.show_to_string() << endmsg;
  
  return(0);
}

int
TRT_ID::get_straw_layer_hash_calc(const ExpandedIdentifier& id,
                                  IdentifierHash& hash_id) const {
  hash_id = m_full_straw_layer_range.cardinalityUpTo(id);
  return(0);
}

// From hash get Identifier
int
TRT_ID::get_id(const IdentifierHash& hash_id,
               Identifier& id,
               const IdContext* context) const {
  int result = 1;

  id.clear();

  size_t begin = (context) ? context->begin_index() : 0;
  // cannot get hash if end is 0:
  size_t end = (context) ? context->end_index()  : 0;
  if (0 == begin) {
    // No hashes yet for ids with prefixes
    if (m_LAYER_OR_WHEEL_INDEX == end) {
      if (hash_id < (unsigned int) (m_module_vec.end() - m_module_vec.begin())) {
        id = m_module_vec[hash_id];
        result = 0;
      }
    } else if (m_STRAW_LAYER_INDEX == end) {
      if (hash_id < (unsigned int) (m_straw_layer_vec.end() - m_straw_layer_vec.begin())) {
        id = m_straw_layer_vec[hash_id];
        result = 0;
      }
    } else if (m_STRAW_INDEX == end) {
      // Do not know how to calculate straw id from hash yet!!
      std::cout << "Do not know how to calculate straw id from hash yet!!" << std::endl;
    }
  }
  return(result);
}

void
TRT_ID::get_expanded_id(const Identifier& id,
                        ExpandedIdentifier& exp_id,
                        const IdContext* context) const {
  exp_id.clear();
  exp_id << indet_field_value()
         << trt_field_value()
         << barrel_ec(id)
         << phi_module(id)
         << layer_or_wheel(id);
  if (!context || context->end_index() >= m_STRAW_LAYER_INDEX) {
    exp_id << straw_layer(id);
  }
  if (!context || context->end_index() == m_STRAW_INDEX) {
    exp_id << straw(id);
  }
}

int
TRT_ID::get_hash(const Identifier& id,
                 IdentifierHash& hash_id,
                 const IdContext* context) const {
  // Get the hash code from either a vec (for modules and straw
  // layers) or calculate it (straws). For the former, we convert to
  // compact and call get_hash again. For the latter, we calculate
  // the hash from the Identifier.

  int result = 1;

  hash_id = 0;
  size_t begin = (context) ? context->begin_index() : 0;
  size_t end = (context) ? context->end_index()  : 0;

  if (0 == begin) {
    // No hashes yet for ids with prefixes
    if (m_LAYER_OR_WHEEL_INDEX == end) {
      hash_id = module_hash(id);
      if (hash_id.is_valid()) result = 0;
    } else if (m_STRAW_LAYER_INDEX == end) {
      hash_id = straw_layer_hash(id);
      if (hash_id.is_valid()) result = 0;
    } else if (context && context->end_index() == m_STRAW_INDEX) {
      // Must calculate for straw hash
      ExpandedIdentifier new_id;
      get_expanded_id(id, new_id);
      hash_id = m_full_straw_range.cardinalityUpTo(new_id);
      result = 0;
    }
  }
  return(result);
}

int
TRT_ID::straw_layer_max(const Identifier& id) const {
  // get max from dictionary
  ExpandedIdentifier expId;
  IdContext module_context1 = module_context();

  get_expanded_id(id, expId, &module_context1);
  int result = 0;
  for (unsigned int i = 0; i < m_full_straw_layer_range.size(); ++i) {
    const Range& range = m_full_straw_layer_range[i];
    if (range.match(expId)) {
      const Range::field& field = range[m_STRAW_LAYER_INDEX];
      if (not field.empty()) {
        int max = field.get_maximum();
        result = result > max ? result : max;
      }
    }
  }
  return(result);
}

int
TRT_ID::straw_max(const Identifier& id) const {
  // get max from dictionary
  ExpandedIdentifier expId;
  IdContext straw_layer_context1 = straw_layer_context();

  get_expanded_id(id, expId, &straw_layer_context1);
  for (unsigned int i = 0; i < m_full_straw_range.size(); ++i) {
    const Range& range = m_full_straw_range[i];
    if (range.match(expId)) {
      const Range::field& field = range[m_STRAW_INDEX];
      if (not field.empty()) {
        return(field.get_maximum());
      }
    }
  }
  return(-999);  // default
}

void
TRT_ID::test_trt_ids() {
  MsgStream log(m_msgSvc, "TRT_ID");
  int nids = 0;
  int nidsFailed = 0;

  init_straw_hash_vector();


  // Check straw hashes
  //bool debug = false;
  for (unsigned int i = 0; i < m_straw_hash_max; ++i, ++nids) {
    Identifier id = m_straw_vec[i];
    IdentifierHash h1 = straw_hash_bin(id);
    IdentifierHash h2 = straw_hash(id);
    //IdentifierHash h2 = straw_hash(id, debug);
    if (h1 != h2) {
      std::cout << "test_trt_ids - bad match: id, bin hash, hash: "
                << nids << " "
                << show_to_string(id) << " "
                << h1 << " "
                << h2 << std::endl;
      nidsFailed++;
      //debug = true;
    }
    Identifier id1 = straw_id(h1);
    if (id != id1) {
      std::cout << "test_trt_ids - bad match: id, bin hash, hash: "
                << nids << " "
                << show_to_string(id) << " "
                << show_to_string(id1) << " "
                << h1 << " "
                << h2 << std::endl;
      nidsFailed++;
    }
    if (i % 10000 == 5) {
      std::cout << "test_trt_ids: id, bin hash, hash: "
                << nids << " "
                << show_to_string(id) << " "
                << h1 << " "
                << h2 << std::endl;
    }
  }

  std::cout << "Checked hash calculation for " << nids << " hashes and found "
            << nidsFailed << " failures to match between binary lookup and calculation "
            << std::endl;

  reset_straw_hash_vector();

  nids = 0;
  IdContext context = module_context();
  const_id_iterator first = m_module_vec.begin();
  const_id_iterator last = m_module_vec.end();
  for (; first != last; ++first, ++nids) {
    Identifier id = (*first);
    ExpandedIdentifier exp_id;
    get_expanded_id(id, exp_id, &context);
    Identifier new_id = module_id(exp_id[m_BARREL_EC_INDEX],
                                  exp_id[m_PHI_MODULE_INDEX],
                                  exp_id[m_LAYER_OR_WHEEL_INDEX]);
    if (id != new_id) {
        log << MSG::ERROR << "TRT_ID::test_trt_ids: module new and old compacts not equal. New/old/expanded ids "
            << nids << " "
            << show_to_string(new_id) << " " << show_to_string(id) << " "
            << (std::string) exp_id << endmsg;
     
    }

    IdentifierHash hashId;
    if (get_hash(id, hashId, &context)) {
        log << MSG::ERROR << "Unable to set trt hash id for det elem "
            << show_to_string(id) << " " << nids
            << endmsg;
      
    }
    new_id = module_id(hashId);

    if (id != new_id) {
        log << MSG::ERROR << "TRT_ID::test_trt_ids: module new and old compacts not equal. New/old/hash ids "
            << nids << " "
            << show_to_string(new_id) << " "
            << show_to_string(id) << " " << MSG::hex
            << hashId << MSG::dec
            << endmsg;
      
    }
  }

  nids = 0;
  context = straw_layer_context();
  first = m_straw_layer_vec.begin();
  last = m_straw_layer_vec.end();
  for (; first != last; ++first, ++nids) {
    Identifier id = (*first);
    ExpandedIdentifier exp_id;
    get_expanded_id(id, exp_id, &context);
    Identifier new_id = layer_id(exp_id[m_BARREL_EC_INDEX],
                                 exp_id[m_PHI_MODULE_INDEX],
                                 exp_id[m_LAYER_OR_WHEEL_INDEX],
                                 exp_id[m_STRAW_LAYER_INDEX]);
    if (id != new_id) {
        log << MSG::ERROR << "TRT_ID::test_trt_ids: straw layer new and old compacts not equal. New/old/expanded ids "
            << show_to_string(new_id) << " " << show_to_string(id) << " "
            << (std::string) exp_id << endmsg;
      
    }

    IdContext strcontext = straw_context();

    // test straw ids
    int nstraws = straw_max(id);
    for (int i = 0; i < nstraws; ++i) {
      id = straw_id(id, i);
      get_expanded_id(id, exp_id, &strcontext);
      new_id = straw_id(exp_id[m_BARREL_EC_INDEX],
                        exp_id[m_PHI_MODULE_INDEX],
                        exp_id[m_LAYER_OR_WHEEL_INDEX],
                        exp_id[m_STRAW_LAYER_INDEX],
                        exp_id[m_STRAW_INDEX]);
      if (id != new_id) {
          log << MSG::ERROR << "TRT_ID::test_trt_ids: straw new and old compacts not equal. New/old/expanded ids "
              << show_to_string(new_id) << " " << show_to_string(id) << " "
              << (std::string) exp_id << endmsg;
        
      }
    }
  }

  log << MSG::INFO << "TRT_ID::test_trt_ids: Successful tested "
        << nids << " ids. "
        << endmsg;
  
}
