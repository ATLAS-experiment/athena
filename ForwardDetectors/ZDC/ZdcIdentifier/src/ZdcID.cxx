/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcIdentifier/ZdcID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/RangeIterator.h"
#include <set>
#include <iostream>

//______________________________________________________
ZdcID::ZdcID()
  : AtlasDetectorID("ZdcID", "ZDC"),
    m_dict(nullptr)
{
} 
//______________________________________________________
ZdcID::~ZdcID(){

}

int
ZdcID::initialize_from_dictionary(const IdDictMgr& dict_mgr)
{
    ATH_MSG_INFO("Initialize from dictionary");
  
    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
        return (0);
    }
    else {
        ATH_MSG_DEBUG("(Re)initialize");
    }

    // init base object
    if(AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return (1);

    // Register version of ForwardDetectors dictionary 
    if (register_dict_tag(dict_mgr, "ForwardDetectors")) return(1);

    m_dict = dict_mgr.find_dictionary ("ForwardDetectors"); 
    if(!m_dict) {
        ATH_MSG_ERROR("ZdcID::initialize_from_dict - cannot access ForwardDetectors dictionary");
        return 1;
    }

    // Initialize the field indices
    if(initLevelsFromDict()) return (1);


    //
    // Build multirange for the valid set of identifiers
    //


    // Find value for the field ForwardDetectors 
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary ("ATLAS"); 
    int fwdField   = -1;
    if (atlasDict->get_label_value("subdet", "ForwardDetectors", fwdField)) {
        ATH_MSG_ERROR("Could not get value for label 'ForwardDetectors' of field 'subdet' in dictionary "
                      << atlasDict->name());
        return (1);
    }

    // Find value for the field ZDC
    int zdcField   = -1;
    if (m_dict->get_label_value("part", "ZDC", zdcField)) {
        ATH_MSG_ERROR("Could not get value for label 'ZDC' of field 'part' in dictionary " 
                      << m_dict->name());
        return (1);
    }
    ATH_MSG_DEBUG("ZdcID::initialize_from_dict "
                  << "Found field values: ZDC " << zdcField);
    
    // Set up id for region and range prefix
    ExpandedIdentifier region_id;
    region_id.add(fwdField);
    region_id.add(zdcField);
    Range prefix;
    m_full_module_range  = m_dict->build_multirange(region_id, prefix, "module");
    m_full_channel_range = m_dict->build_multirange(region_id, prefix);

    ATH_MSG_INFO("ZdcID::initialize_from_dict");
    ATH_MSG_DEBUG("Module range -> " << (std::string)m_full_module_range);
    ATH_MSG_DEBUG("Channel range -> " << (std::string)m_full_channel_range);

    // Setup the hash tables
    if(init_hashes()) return (1);
    
    return 0;
}



int     
ZdcID::initLevelsFromDict()
{
    if(!m_dict) {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - dictionary NOT initialized");
        return (1);
    }
    
    // Find out which identifier field corresponds to each level. Use
    // names to find each field/level.

    m_zdc_region_index          = 999;
    m_FORWARD_INDEX             = 999;
    m_ZDC_INDEX                 = 999;
    m_SIDE_INDEX                = 999;
    m_MODULE_INDEX              = 999;
    m_TYPE_INDEX                = 999;
    m_CHANNEL_INDEX             = 999;

    // Save index to a ZDC region for unpacking
    ExpandedIdentifier id; 
    id << forward_field_value() << zdc_field_value();
    if (m_dict->find_region(id, m_zdc_region_index)) {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find zdc region index: id, reg "  
                      << (std::string)id << " " << m_zdc_region_index);
        return (1);
    }

    // Find a ZDC region
    const IdDictField* field = m_dict->find_field("subdet");
    if (field) {
        m_FORWARD_INDEX = field->index();
    }
    else {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find 'subdet' field");
        return (1);
    }
    field = m_dict->find_field("part");
    if (field) {
        m_ZDC_INDEX = field->index();
    }
    else {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find 'part' field");
        return (1);
    }
    field = m_dict->find_field("side");
    if (field) {
        m_SIDE_INDEX = field->index();
    }
    else {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find 'side' field");
        return (1);
    }
    field = m_dict->find_field("module");
    if (field) {
        m_MODULE_INDEX = field->index();
    }
    else {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find 'module' field");
        return (1);
    }
    field = m_dict->find_field("type");
    if (field) {
        m_TYPE_INDEX = field->index();
    }
    else {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find 'type' field");
        return (1);
    }
    field = m_dict->find_field("channel");
    if (field) {
        m_CHANNEL_INDEX = field->index();
    }
    else {
        ATH_MSG_ERROR("ZdcID::initLevelsFromDict - unable to find 'channel' field");
        return (1);
    }
    
    // Set the field implementations: for bec, lay/disk, eta/phi mod

    const IdDictRegion& region = m_dict->region(m_zdc_region_index);

    m_forward_impl  = region.implementation(m_FORWARD_INDEX);
    m_zdc_impl      = region.implementation(m_ZDC_INDEX);
    m_side_impl     = region.implementation(m_SIDE_INDEX);
    m_module_impl   = region.implementation(m_MODULE_INDEX);
    m_type_impl     = region.implementation(m_TYPE_INDEX);
    m_channel_impl  = region.implementation(m_CHANNEL_INDEX);

    ATH_MSG_DEBUG("decode index and bit fields for each level:");
    ATH_MSG_DEBUG("forward  " << m_forward_impl.show_to_string());
    ATH_MSG_DEBUG("zdc      " << m_zdc_impl.show_to_string());
    ATH_MSG_DEBUG("side     " << m_side_impl.show_to_string());
    ATH_MSG_DEBUG("module   " << m_module_impl.show_to_string());
    ATH_MSG_DEBUG("type     " << m_type_impl.show_to_string());
    ATH_MSG_DEBUG("channel  " << m_channel_impl.show_to_string());
    
    return (0);

}


int
ZdcID::init_hashes()
{
    //
    // create a vector(s) to retrieve the hashes for compact ids.
    //

    // module hash
    m_module_hash_max = m_full_module_range.cardinality();
    m_module_vec.resize(m_module_hash_max);
    unsigned int nids = 0;
    std::set<Identifier> ids;
    for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
	const Range& range = m_full_module_range[i];
	ConstRangeIterator rit(range);
	auto  first = rit.begin();
	auto last  = rit.end();
	for (; first != last; ++first) {
	    const ExpandedIdentifier& exp_id = (*first);
	    Identifier id = module_id(exp_id[m_SIDE_INDEX],
                                      exp_id[m_MODULE_INDEX]);
	    if(!(ids.insert(id)).second) {
                ATH_MSG_ERROR("ZDC_ID::init_hashes "
                              << " Error: duplicated id for module id. nid " << nids
                              << " compact id " << MSG::hex << id.get_compact() << MSG::dec
                              << " id " << (std::string)exp_id);
		return (1);
	    }
	    nids++;
	}
    }
    if(ids.size() != m_module_hash_max) {
        ATH_MSG_ERROR("ZDC_ID::init_hashes "
                      << " Error: set size NOT EQUAL to hash max. size " << ids.size()
                      << " hash max " << m_module_hash_max);
        return (1);
    }

    nids = 0;
    std::set<Identifier>::const_iterator first = ids.begin();
    std::set<Identifier>::const_iterator last  = ids.end();
    for (; first != last && nids < m_module_vec.size(); ++first) {
	m_module_vec[nids] = (*first);
	nids++;
    }

    // channel hash
    m_channel_hash_max = m_full_channel_range.cardinality();
    m_channel_vec.resize(m_channel_hash_max);
    nids = 0;
    ids.clear();
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
	const Range& range = m_full_channel_range[i];
	ConstRangeIterator rit(range);
	auto first = rit.begin();
	auto last  = rit.end();
	for (; first != last; ++first) {
	    const ExpandedIdentifier& exp_id = (*first);
	    Identifier id = channel_id(exp_id[m_SIDE_INDEX],
                                       exp_id[m_MODULE_INDEX], 
                                       exp_id[m_TYPE_INDEX],
                                       exp_id[m_CHANNEL_INDEX]);
            
	    if(!(ids.insert(id)).second) {
                ATH_MSG_ERROR("ZDC_ID::init_hashes "
                              << " Error: duplicated id for channel id. nid " << nids
                              << " compact id " << MSG::hex << id.get_compact() << MSG::dec
                              << " id " << (std::string)exp_id);
		return (1);
	    }
	    nids++;
	}
    }
    if(ids.size() != m_channel_hash_max) {
        ATH_MSG_ERROR("ZDC_ID::init_hashes "
                      << " Error: set size NOT EQUAL to hash max. size " << ids.size()
                      << " hash max " << m_channel_hash_max);
        return (1);
    }

    nids = 0;
    first = ids.begin();
    last  = ids.end();
    for (; first != last && nids < m_channel_vec.size(); ++first) {
	m_channel_vec[nids] = (*first);
	nids++;
    }
    return (0);
}

void
ZdcID::test_packing    () const
{
    if (m_dict) {
	
	int nids = 0;
	int nerr = 0;
        ExpandedIdentifier id;
	const_id_iterator first = m_module_vec.begin();
	const_id_iterator last  = m_module_vec.end();
	for (; first != last; ++first, ++nids) {
	    Identifier id = (*first);
	    Identifier new_id = module_id(side(id),
                                          module(id));
	    if (id != new_id) {
            ATH_MSG_ERROR("ZDC_ID::test_packing: new and old compacts not equal. New/old/expanded ids "
                          << show_to_string(id) << " " << show_to_string(new_id));
		nerr++;
		continue;
	    }
            IdentifierHash idHash = module_hash(id);
	    Identifier new_id_1   = module_id(idHash);
	    if (id != new_id_1) {
            ATH_MSG_ERROR("ZDC_ID::test_packing: new and old compacts from hash not equal. New/old/expanded ids. hash " << idHash << " "
                          << show_to_string(id) << " " << show_to_string(new_id_1));
		nerr++;
		continue;
	    }
	}

	ATH_MSG_DEBUG("ZDC_ID::test_packing: tested module ids. nids, errors "
	              << nids << " " << nerr);

        first = m_channel_vec.begin();
        last  = m_channel_vec.end();
	for (; first != last; ++first, ++nids) {
	    Identifier id = (*first);
	    Identifier new_id = channel_id(side(id),
                                           module(id),
                                           type(id),
                                           channel(id));
	    if (id != new_id) {
            ATH_MSG_ERROR("ZDC_ID::test_packing: new and old compacts not equal. New/old/expanded ids "
                          << show_to_string(id) << " " << show_to_string(new_id));
		nerr++;
		continue;
	    }
            IdentifierHash idHash = channel_hash(id);
	    Identifier new_id_1   = channel_id(idHash);
	    if (id != new_id_1) {
            ATH_MSG_ERROR("ZDC_ID::test_packing: new and old compacts from hash not equal. New/old/expanded ids hash " << idHash << " "
                          << show_to_string(id) << " " << show_to_string(new_id_1));
		nerr++;
		continue;
	    }
	}

	ATH_MSG_DEBUG("ZDC_ID::test_packing: tested channel ids. nids, errors "
	              << nids << " " << nerr);
	
	ATH_MSG_DEBUG("ZDC_ID::test_packing: Successfully tested "
	              << nids << " ids.");
    }
    else {
        ATH_MSG_ERROR("ZDC_ID::test_packing: Unable to test module/channel is packing - no dictionary has been defined.");
    }
}

ZdcID::const_id_iterator   ZdcID::modules_begin () const
{
    return (m_module_vec.begin());
}

ZdcID::const_id_iterator   ZdcID::modules_end   () const
{
    return (m_module_vec.end());
}

ZdcID::const_id_iterator   ZdcID::channels_begin () const
{
    return (m_channel_vec.begin());
}

ZdcID::const_id_iterator   ZdcID::channels_end   () const
{
    return (m_channel_vec.end());
}



