/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdentifier/LArHVLineID.h"
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


LArHVLineID::LArHVLineID() :
  AtlasDetectorID("LArHVLineID", "LArHV"),
  m_larhvRegion_index(999),
  m_atlas_index(999),
  m_configuration_index(999),
  m_partition_index(999),
  m_canline_index(999),
  m_cannode_index(999),
  m_hvline_index(999),
  m_dict(nullptr),
  m_hvlineHashMax(0)
{

}

LArHVLineID:: ~LArHVLineID()= default;


IdContext LArHVLineID::hvlineContext() const
{
  ExpandedIdentifier id;
  return (IdContext(id, 0, m_hvline_index));
}

/*
IdContext LArHVLineID::canlineContext() const
{
  ExpandedIdentifier id;
  return (IdContext(id, 0, m_canline_index));
}
*/

//==========================================================================
int  LArHVLineID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
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
    ATH_MSG_INFO(" => initialize_from_dictionary(dict_mgr) ="
                 << AtlasDetectorID::initialize_from_dictionary(dict_mgr));
  }
  m_dict = dict_mgr.find_dictionary ("LArHighVoltage"); 

  if(!m_dict) 
    {
      ATH_MSG_ERROR("initialize_from_dictionary - cannot access LArHighVoltage dictionary ");
      return 1;
    }

  // Register version of the dictionary used
  // ---------------------------------------
  if (register_dict_tag(dict_mgr, "LArHighVoltage")) return(1);
  ATH_MSG_INFO("Register_dict_tag of LArHighVoltage is OK");

  // initialize dictionary version
  // -----------------------------
  AtlasDetectorID::setDictVersion(dict_mgr, "LArHighVoltage");
  ATH_MSG_INFO("setDictVersion of LArHighVoltage is OK");


  // Initialize the field indices
  // =========================================================================
  if(initLevelsFromDict()) return (1); 

  /* Find value for the field Calorimeter */
  const IdDictDictionary* atlasDict = dict_mgr.find_dictionary ("ATLAS"); 
  int larHVValue   = -1;
  if (atlasDict->get_label_value("subdet", "LArHighVoltage", larHVValue)) {
    ATH_MSG_ERROR("Could not get value for label 'LArHighVoltage' of field 'subdet' in dictionary "
                  << atlasDict->name());
    return (1);
  }
  ATH_MSG_DEBUG("[init_from_dictionary] > larHV value = " << larHVValue);


  /* Find values for the field Configuration */
  int configurationValue   = 1;
  if (m_dict->get_label_value("configuration", "Atlas", configurationValue)) {
    ATH_MSG_WARNING("Could not get value for label 'configuration' in dictionary "
                    << m_dict->name());
    return (0);
  }
  ATH_MSG_DEBUG("[init_from_dictionary] > configurationValue = " << configurationValue);

  // Set up Expanded identifier for hvline range prefix
  // =========================================================
  ExpandedIdentifier reg_id;
  reg_id.add(larHVValue);
  reg_id.add(configurationValue); 
  Range prefix;

  /*Full range for all lines */
  m_full_atlas_highvoltage_range=m_dict->build_multirange(reg_id, prefix);
  m_full_hvline_range = m_dict->build_multirange(reg_id, prefix, "hvline");
  m_full_canline_range = m_dict->build_multirange(reg_id, prefix, "canline");
  ATH_MSG_INFO("[initialize_from_dictionary] >  HV line range -> " << (std::string)m_full_hvline_range);
  
  // Setup the hash tables
  // =========================================================
  if(init_hashes()) return (1);

  return 0;
}


//=====================================================================================
int LArHVLineID::get_expanded_id  (const HWIdentifier& id, 
				   ExpandedIdentifier& exp_id, 
				   const IdContext* context) const
//=====================================================================================
{
  // We assume that the context is >= hvline
  exp_id.clear();
  exp_id << lar_field_value()
	 << s_lar_atlas_value
    	 << partition(id)
    	 << can_line(id)
	 << can_node(id)
	 << hv_line(id);
  if(context && context->end_index() >= m_hvline_index) {
    exp_id << hv_line(id);
  }
  return (0);
}


//=============================================================================
int LArHVLineID::initLevelsFromDict()
//=============================================================================
{
  ATH_MSG_DEBUG("[initLevelsFromDict] Entering routine...");

  if(!m_dict) {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - dictionary NOT initialized");
    return (1);
  }

  ATH_MSG_INFO("[initLevelsFromDict] m_dict OK ...");

  // Find out which identifier field corresponds to each level.
  // ========================================================================
  m_atlas_index               = 999;
  m_configuration_index       = 999;
  m_partition_index           = 999;
  m_canline_index             = 999;
  m_cannode_index             = 999;
  m_hvline_index              = 999;

  ATH_MSG_DEBUG("[initLevelsFromDict] data member initialization OK ...");
  
  // Search with region name
  const IdDictRegion* reg = m_dict->find_region("LArHV-HEC-A");
  if (reg) {
      m_larhvRegion_index = reg->index();}
  else {
    ATH_MSG_INFO("WARNING : LArHVLineID::initLevelsFromDict - unable to find 'barrel-region1' region");
    return (0);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] region 'LAr-HV-HEC-A' found OK ...");

  // Find ATLAS field 
  // ========================================================================
  const IdDictField* field = m_dict->find_field("subdet") ;
  if (field) {
    m_atlas_index = field->index();}
  else {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - unable to find 'subdet' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'LArHighVoltage' found OK");

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

  // Look for Field 'partition'
  // ========================================================================
  field = m_dict->find_field("partition") ;
  if (field) {
    m_partition_index = field->index();}
  else {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - unable to find 'partition' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'partition' found OK");


  // Look for Field 'CAN LINE'
  // ========================================================================
  field = m_dict->find_field("canline") ;
  if (field) {
    m_canline_index = field->index();}
  else {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - unable to find 'canline' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'canline' found OK");


  // Look for Fields 'CAN NODE'
  // ========================================================================
  field = m_dict->find_field("cannode") ;
  if (field) {
    m_cannode_index = field->index();
  }
  else {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - unable to find 'cannode' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'cannode' found OK");

  
  // Look for Fields 'HV_line'
  // ========================================================================
  field = m_dict->find_field("hvline") ;
  if (field) {
    m_hvline_index = field->index();
  }
  else {
    ATH_MSG_INFO("LArHVLineID::initLevelsFromDict - unable to find 'hvline' field");
    return (1);
  }
  ATH_MSG_DEBUG("[initLevelsFromDict] field 'hvline' found OK");


  // Set the field implementation
  // ========================================================================

  const IdDictRegion& region = m_dict->region(m_larhvRegion_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] Found levels:");
  ATH_MSG_DEBUG("[initLevelsFromDict] > larHV           " << m_atlas_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > larConfiguration " << m_configuration_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > CAN Node       " << m_cannode_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > HV line        " << m_hvline_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > partition      " << m_partition_index);
  ATH_MSG_DEBUG("[initLevelsFromDict] > CAN line       " << m_canline_index);


  ATH_MSG_DEBUG("[initLevelsFromDict] > ...fields implementation...");
  ATH_MSG_DEBUG("[initLevelsFromDict] > ...implementation: m_larhvcalo_index");
  m_atlas_impl        = region.implementation(m_atlas_index);
  m_configuration_impl= region.implementation(m_configuration_index);
  m_partition_impl    = region.implementation(m_partition_index);
  m_canline_impl      = region.implementation(m_canline_index);
  m_cannode_impl      = region.implementation(m_cannode_index);
  m_hvline_impl       = region.implementation(m_hvline_index);
  
  ATH_MSG_DEBUG("[initLevelsFromDict] Decode index and bit fields for each level:");
  ATH_MSG_DEBUG("[initLevelsFromDict] > larHV      " << m_atlas_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > larConfig  " << m_configuration_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > partition  " << m_partition_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > can line   " << m_canline_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > can node   " << m_cannode_impl.show_to_string());
  ATH_MSG_DEBUG("[initLevelsFromDict] > hv line    " << m_hvline_impl.show_to_string());


  return(0) ;
}




//=====================================================
int  LArHVLineID::init_hashes()
//=====================================================
{
  // tower hash
  // -----------
  m_hvlineHashMax = m_full_atlas_highvoltage_range.cardinality();
  m_hvline_vec.resize(m_hvlineHashMax);
  unsigned int nids = 0;
  std::set<HWIdentifier> ids;
  for (unsigned int i = 0; i < m_full_atlas_highvoltage_range.size(); ++i) {
    const Range& range = m_full_atlas_highvoltage_range[i];
    ConstRangeIterator rit(range);
    for (const auto & exp_id :rit) {
      HWIdentifier hv_id = HVLineId( 
				    exp_id[m_partition_index] ,
				    exp_id[m_canline_index] ,
				    exp_id[m_cannode_index] ,
				    exp_id[m_hvline_index]  
				    );
      if(!(ids.insert(hv_id)).second){
        ATH_MSG_ERROR("[init_hashes] > duplicated id for channel nb = " << nids);
        ATH_MSG_ERROR(" expanded Id= " << show_to_string(hv_id));
      }
      nids++;
    }
  }
  if(ids.size() != m_hvlineHashMax) {
    ATH_MSG_ERROR("[init_hashes] >");
    ATH_MSG_ERROR(" set size NOT EQUAL to hash max. size " << ids.size());
    ATH_MSG_ERROR(" hash max " << m_hvlineHashMax);
    return (1);
  }

  nids=0;
  std::set<HWIdentifier>::const_iterator first = ids.begin();
  std::set<HWIdentifier>::const_iterator last  = ids.end();
  for (;first != last && nids < m_hvline_vec.size(); ++first) {
    m_hvline_vec[nids] = (*first) ;
    nids++;
  }
  ATH_MSG_INFO("[init_hashes()] > Hvline_size= " << m_hvline_vec.size());
  return (0);                   
}



