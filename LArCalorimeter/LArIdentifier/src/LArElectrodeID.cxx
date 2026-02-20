/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdentifier/LArElectrodeID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/RangeIterator.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <set>
#include <string>


LArElectrodeID::LArElectrodeID() :
  AtlasDetectorID("LArElectrodeID", "LArElec"),
  m_larElectrodeRegion_index(999),
  m_larelectrode_index(999),
  m_atlas_index(999),
  m_configuration_index(999),
  m_detector_index(999),
  m_zside_index(999),
  m_module_index(999),
  m_hvphi_index(999),
  m_hveta_index(999),
  m_hvgap_index(999),
  m_electrode_index(999),
  m_dict(nullptr),
  m_electrodeHashMax(0)
{

}

LArElectrodeID:: ~LArElectrodeID()= default;


IdContext LArElectrodeID::electrodeContext() const
{
  ExpandedIdentifier id;
  return (IdContext(id, 0, m_electrode_index));
}


//==========================================================================
int  LArElectrodeID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
//==========================================================================
{

  ATH_MSG_INFO(" => initialize_from_dictionary()");
  
  // Check whether this helper should be reinitialized
  // -------------------------------------------------
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
    return (0);
  }
  else {
    ATH_MSG_DEBUG("(Re)initialize");
  }
  ATH_MSG_DEBUG(" => Initialization of dict_mgr done ! " << m_dict);

  // init base object
  // ----------------
  if(AtlasDetectorID::initialize_from_dictionary(dict_mgr)){
    return (1);
  }
  else{
    ATH_MSG_DEBUG(" => initialize_from_dictionary(dict_mgr) ="
                  << AtlasDetectorID::initialize_from_dictionary(dict_mgr));
  }
  m_dict = dict_mgr.find_dictionary ("LArElectrode"); 
  ATH_MSG_DEBUG(" => defined m_dict from find_dictionary(LArElectrode) = " << m_dict);
  if(!m_dict) {
    ATH_MSG_ERROR("initialize_from_dictionary - cannot access LArElectrode dictionary");
    return 1;
  }
  
  // Register version of the dictionary used
  // ---------------------------------------
  if (register_dict_tag(dict_mgr, "LArElectrode")) return(1);
  ATH_MSG_DEBUG("Register_dict_tag of LArElectrode is OK");

  // initialize dictionary version
  // -----------------------------
  AtlasDetectorID::setDictVersion(dict_mgr, "LArElectrode");
  ATH_MSG_DEBUG("setDictVersion of LArElectrode is OK");


  // Initialize the field indices
  // =========================================================================
  if(initLevelsFromDict()) return (1); 

  /* Find value for the field Calorimeter */
  const IdDictDictionary* atlasDict = dict_mgr.find_dictionary ("ATLAS"); 
  int larField   = -1;
  if (atlasDict->get_label_value("subdet", "LArElectrode", larField)) {
    ATH_MSG_ERROR("Could not get value for label 'LArElectrode' of field 'subdet' in dictionary "
                  << atlasDict->name());
    return (1);
  }
  ATH_MSG_DEBUG("[init_from_dictionary] > larField value = " << larField);


  /* Find values for the field Detector */
  int configurationValue   = 1;
  if (m_dict->get_label_value("configuration", "Atlas", configurationValue)) {
    ATH_MSG_WARNING("Could not get value for label 'configuration' in dictionary "
                    << m_dict->name());
    return (0);
  }
  ATH_MSG_DEBUG("[init_from_dictionary] > configurationValue = " << configurationValue);

  // Set up Expanded identifier for electrode range prefix
  // =========================================================
  ExpandedIdentifier reg_id;
  reg_id.add(larField);
  reg_id.add(configurationValue); 
  Range prefix;

  /*Full range for all lines */
  m_full_larelectrode_range=m_dict->build_multirange(reg_id, prefix);
  m_full_electrode_range = m_dict->build_multirange(reg_id, prefix, "electrode");
  ATH_MSG_DEBUG("[initialize_from_dictionary] >  Electrode range -> " 
                << (std::string)m_full_electrode_range);
  
  // Setup the hash tables
  // =========================================================
  if(init_hashes()) return (1);

  return 0;
}


//=====================================================================================
int LArElectrodeID::get_expanded_id  (const HWIdentifier& id, 
				      ExpandedIdentifier& exp_id, 
				      const IdContext* context) const
//=====================================================================================
{
  // We assume that the context is >= electrode
  exp_id.clear();
  exp_id << lar_field_value()
	 << s_larconfig_value
    	 << detector(id)
    	 << zside(id)
	 << module(id)
	 << hv_phi(id)
	 << hv_eta(id)
	 << gap(id);
  if(context && context->end_index() >= m_electrode_index) {
    exp_id << electrode(id);
  }
  return (0);
}



//=============================================================================
int LArElectrodeID::initLevelsFromDict()
//=============================================================================
{
  ATH_MSG_DEBUG("[initLevelsFromDict] Entering routine...");

  if(!m_dict) {
    ATH_MSG_INFO("LArElectrodeID::initLevelsFromDict - dictionary NOT initialized");
    return (1);
  }

  ATH_MSG_DEBUG("[initLevelsFromDict] m_dict OK ...");

  // Find out which identifier field corresponds to each level.
  // ========================================================================
  m_atlas_index          = 999;
  m_configuration_index  = 999;
  m_detector_index       = 999;
  m_zside_index          = 999;
  m_module_index         = 999;
  m_hvphi_index          = 999;
  m_hveta_index          = 999;
  m_hvgap_index          = 999;
  m_electrode_index      = 999;
  ATH_MSG_DEBUG("[initLevelsFromDict] data member initialization OK ...");
  
  // Search with region name
  const IdDictRegion* reg = m_dict->find_region("EM-BARREL-ALL");
  if (reg) {
      m_larElectrodeRegion_index = reg->index();}
  else {
    ATH_MSG_WARNING("[initLevelsFromDict] - unable to find 'EM-BARREL-ALL' region");
    return (0);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] region 'EM-BARREL-ALL' found OK ...");

  // Find LArElectrode field 
  // ========================================================================
  const IdDictField* field = m_dict->find_field("subdet") ;
  if (field) {
    m_atlas_index = field->index();}
  else {
    ATH_MSG_INFO("LArElectrodeID::initLevelsFromDict - unable to find 'subdet' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'LArElectrode' found OK");

  // Find Configuration field 
  // ========================================================================
  field = m_dict->find_field("configuration") ;
  if (field) {
    m_configuration_index = field->index();}
  else {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - unable to find 'configuration' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field config=Atlas found OK");

  // Find Detector field 
  // ========================================================================
  field = m_dict->find_field("detector") ;
  if (field) {
    m_detector_index = field->index();}
  else {
    ATH_MSG_INFO("LArElectrodeID::initLevelsFromDict - unable to find 'detector' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'detector' found OK");


  // Look for Field 'zside'
  // ========================================================================
  field = m_dict->find_field("zside") ;
  if (field) {
    m_zside_index = field->index();}
  else {
    ATH_MSG_INFO("LArElectrodeID::initLevelsFromDict - unable to find 'zside' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'zside' found OK");


  // Look for Fields 'module'
  // ========================================================================
  field = m_dict->find_field("module") ;
  if (field) {
    m_module_index = field->index();
  }
  else {
    ATH_MSG_INFO("LArElectrodeID::initLevelsFromDict - unable to find 'module' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'module' found OK");

  // Look for Fields 'hvphi'
  // ========================================================================
  field = m_dict->find_field("hvphi") ;
  if (field) {
    m_hvphi_index = field->index();
  }
  else {
    ATH_MSG_INFO("initLevelsFromDict - unable to find 'hvphi' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'hvphi' found OK");

  // Look for Fields 'hveta'
  // ========================================================================
  field = m_dict->find_field("hveta") ;
  if (field) {
    m_hveta_index = field->index();
  }
  else {
    ATH_MSG_INFO("[initLevelsFromDict] - unable to find 'hveta' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'hveta' found OK");

  // Look for Fields 'hvgap'
  // ========================================================================
  field = m_dict->find_field("hvgap") ;
  if (field) {
    m_hvgap_index = field->index();
  }
  else {
    ATH_MSG_INFO("[initLevelsFromDict] - unable to find 'hvgap' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'hvgap' found OK");

  
  // Look for Fields 'electrode'
  // ========================================================================
  field = m_dict->find_field("electrode") ;
  if (field) {
    m_electrode_index = field->index();
  }
  else {
    ATH_MSG_INFO("[initLevelsFromDict] - unable to find 'electrode' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'electrode' found OK");


  // Set the field implementation
  // ========================================================================

  const IdDictRegion& region = m_dict->region(m_larElectrodeRegion_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] Found levels:");
  ATH_MSG_DEBUG("[initLevelsFromDict] > Atlas     : " << m_atlas_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > Configurat: " << m_configuration_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > detector  : " << m_detector_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > zside     : " << m_zside_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > hvphi     : " << m_hvphi_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > hveta     : " << m_hveta_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > hvgap     : " << m_hvgap_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > electrode : " << m_electrode_index);
  m_atlas_impl      = region.implementation(m_atlas_index);
  m_configuration_impl= region.implementation(m_configuration_index);
  m_detector_impl   = region.implementation(m_detector_index);
  m_zside_impl      = region.implementation(m_zside_index);
  m_module_impl     = region.implementation(m_module_index);
  m_hvphi_impl      = region.implementation(m_hvphi_index);
  m_hveta_impl      = region.implementation(m_hveta_index);
  m_hvgap_impl      = region.implementation(m_hvgap_index);
  m_electrode_impl  = region.implementation(m_electrode_index);
  
  ATH_MSG_DEBUG("[initLevelsFromDict] Decode index and bit fields for each level:");
  ATH_MSG_DEBUG("[initLevelsFromDict] > Atlas       " << m_atlas_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > larConfig  " << m_configuration_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > detector    " << m_detector_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > z-side      " << m_zside_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > module      " << m_module_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > hvphi       " << m_hvphi_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > hveta       " << m_hveta_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > hvgap       " << m_hvgap_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > hv line     " << m_electrode_impl.show_to_string());


  return(0) ;
}




//=====================================================
int  LArElectrodeID::init_hashes()
//=====================================================
{
  // tower hash
  // -----------
  m_electrodeHashMax = m_full_larelectrode_range.cardinality();
  m_electrode_vec.resize(m_electrodeHashMax);
  unsigned int nids = 0;
  std::set<HWIdentifier> ids;
  for (unsigned int i = 0; i < m_full_larelectrode_range.size(); ++i) {
    const Range& range = m_full_larelectrode_range[i];
    ConstRangeIterator rit(range);
    for (const auto & exp_id :rit) {
      HWIdentifier elec_id = ElectrodeId( 
					 exp_id[m_detector_index] ,
					 exp_id[m_zside_index] ,
					 exp_id[m_module_index] ,
					 exp_id[m_hvphi_index] ,
					 exp_id[m_hveta_index] ,
					 exp_id[m_hvgap_index] ,
					 exp_id[m_electrode_index]  
					 );
      if(!(ids.insert(elec_id)).second){
        ATH_MSG_ERROR("[init_hashes] > duplicated id for channel nb = " << nids);
        ATH_MSG_ERROR(" expanded Id= " << show_to_string(elec_id));
      }
      nids++;
    }
  }
  if(ids.size() != m_electrodeHashMax) {
    ATH_MSG_ERROR("[init_hashes] >");
    ATH_MSG_ERROR(" set size NOT EQUAL to hash max. size " << ids.size());
    ATH_MSG_ERROR(" hash max " << m_electrodeHashMax);
    return (1);
  }

  nids=0;
  std::set<HWIdentifier>::const_iterator first = ids.begin();
  std::set<HWIdentifier>::const_iterator last  = ids.end();
  for (;first != last && nids < m_electrode_vec.size(); ++first) {
    m_electrode_vec[nids] = (*first) ;
    nids++;
  }
  ATH_MSG_DEBUG("[init_hashes()] > Electrode_size= " << m_electrode_vec.size());
  return (0);                   
}



