/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/IdentifierHash.h"
#include "LArIdentifier/LArOnlID_Exception.h"
#include <cmath>
#include <set>
#include <string>

/* See comments in Base class */

LArOnline_SuperCellID::LArOnline_SuperCellID() :
  LArOnlineID_Base("LArOnline_SuperCellID", "LArOnline_SuperCell", true)
{
}


LArOnline_SuperCellID::~LArOnline_SuperCellID() = default;

/* =================================================================== */
int  LArOnline_SuperCellID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
/* =================================================================== */
{
    ATH_MSG_INFO("initialize_from_dictionary");
  
    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
        return (0);
    }
    else {
        ATH_MSG_DEBUG("(Re)initialize");
    }

    // init base object
    if(AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return (1);
    m_dict = dict_mgr.find_dictionary ("LArCalorimeter"); 
    if(!m_dict) {
        ATH_MSG_ERROR("initialize_from_dictionary - cannot access LArCalorimeter dictionary ");
        return 1;
    }

    // Register version of the dictionary used
    if (register_dict_tag(dict_mgr, "LArCalorimeter")) return(1);

    // initialize dictionary version
    AtlasDetectorID::setDictVersion(dict_mgr, "LArCalorimeter");

    /* Initialize the field indices */
    if(LArOnlineID_Base::initLevelsFromDict(group())) return (1);

    ATH_MSG_INFO("Finished initLevelsFromDict");


    /* Find value for the field LAr Calorimeter */
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary ("ATLAS"); 
    int larField   = -1;
    if (atlasDict->get_label_value("subdet", "LArCalorimeter", larField)) {
        ATH_MSG_ERROR("Could not get value for label 'LArCalorimeter' of field 'subdet' in dictionary "
                      << atlasDict->name());
        return (1);
    }

    /* Find value for the field LArOnline */
    int larOnlineField   = -4;
    if (m_dict->get_label_value("part", "LArOnline", larOnlineField)) {
        ATH_MSG_ERROR("Could not get value for label 'LArOnline' of field 'part' in dictionary "
                      << m_dict->name());
        return (1);
    }

    /* Find value for the field calibLArOnline */
    int larOnlineCalibField   = -5;
    if (m_dict->get_label_value("part", "LArOnlineCalib", larOnlineCalibField)) {
        ATH_MSG_ERROR("Could not get value for label 'LArOnlineCalib' of field 'part' in dictionary "
                      << m_dict->name());
        return (1);
    }

    /* Set up id for Region and range prefix */
    ExpandedIdentifier region_id; 
    region_id.add(larField);
    region_id.add(larOnlineField);
    Range prefix;

    /*Full range for all channels*/
    m_full_laronline_range = m_dict->build_multirange( region_id, group(),  prefix);
    m_full_feb_range       = m_dict->build_multirange( region_id, group(), prefix, "slar_slot");
    m_full_feedthrough_range = m_dict->build_multirange( region_id , group(), prefix, "slar_feedthrough");

    ATH_MSG_DEBUG(" initialize_from_dictionary :");
    ATH_MSG_DEBUG(" feedthrough range -> " + (std::string)m_full_feedthrough_range);
    ATH_MSG_DEBUG(" feedthrough slot range -> " + (std::string)m_full_feb_range);
    ATH_MSG_DEBUG(" channel range -> " + (std::string)m_full_laronline_range);
  
    /* Setup the hash tables */
    ATH_MSG_DEBUG("[initialize_from_dictionary] version= " << dictionaryVersion());
    if( dictionaryVersion() == "fullAtlas" ) {
        if(LArOnlineID_Base::init_hashes()) return (1);
    }
  
    // Setup for hash calculation for channels (febs is further below)

    // Febs have a uniform number of channels
    // The lookup table only needs to contain the
    // hash offset for each feb
  
    // The implementation requires:
  
    //   1) a lookup table for each feb containing hash offset
    //   2) a decoder to access the "index" corresponding to the
    //      bec/side/ft/slot fields. These fields use x bits, so the
    //      vector has a length of 2**x.

    /* Create decoder for fields bec to slot */
    IdDictFieldImplementation::size_type bits = 
        m_bec_impl.bits() +
        m_side_impl.bits() +
        m_feedthrough_impl.bits() +
        m_slot_impl.bits();
    IdDictFieldImplementation::size_type bits_offset = m_bec_impl.bits_offset();
    m_bec_slot_impl.set_bits(bits, bits_offset);
    int size = (1 << bits);

    // Set up vector as lookup table for hash calculation. 
    m_chan_hash_calcs.resize(size);

    for (unsigned int i = 0; i < m_febHashMax; ++i) {

        HWIdentifier febId = feb_Id(i) ;

        HashCalc hc;
      
        HWIdentifier min = channel_Id ( febId, 0);

        IdentifierHash min_hash = channel_Hash_binary_search(min);
        hc.m_hash   = min_hash;
        m_chan_hash_calcs[m_bec_slot_impl.unpack(min)] = hc;

        if (m_bec_slot_impl.unpack(min) >= size) {
            ATH_MSG_DEBUG("Min > " << size);
            ATH_MSG_DEBUG(" " << show_to_string(min));
            ATH_MSG_DEBUG(" " << m_bec_slot_impl.unpack(min));
        }
    }

    // Check channel hash calculation
    for (unsigned int i = 0; i < m_channelHashMax; ++i) {
        HWIdentifier id = channel_Id(i);
        if (channel_Hash(id) != i) {
            ATH_MSG_ERROR(" *****  Error channel ranges, id, hash, i = " << show_to_string(id));
            ATH_MSG_ERROR(" , " << channel_Hash(id));
            ATH_MSG_ERROR(" , " << i);
        }
    }


  
    // Setup for hash calculation for febs

    // We calculate the feb hash by saving the hash of each
    // feedthrough in a HashCalc object and then adding on the slot
    // number for a particular feb
  
    // The implementation requires:
  
    //   1) a lookup table for each ft containing hash offset
    //   2) a decoder to access the "index" corresponding to the
    //      bec/side/ft fields. These fields use x bits, so the
    //      vector has a length of 2**x.

    /* Create decoder for fields bec to ft */
    bits = m_bec_impl.bits() +
        m_side_impl.bits() +
        m_feedthrough_impl.bits();
    bits_offset = m_bec_impl.bits_offset();
    m_bec_ft_impl.set_bits(bits, bits_offset);
    size = (1 << bits);

    // Set up vector as lookup table for hash calculation. 
    m_feb_hash_calcs.resize(size);

    // Get context for conversion to expanded ids
    IdContext ftContext = feedthroughContext();
    ExpandedIdentifier ftExpId;

    for (unsigned int i = 0; i < m_feedthroughHashMax; ++i) {

        HWIdentifier min = feedthrough_Id(i) ;

        HashCalcFeb hc;

        // Set the hash id for each feedthrough, and then check if one
        // needs to also save the slot values
        IdentifierHash min_hash = LArOnlineID_Base::feb_Hash_binary_search(min);
        hc.m_hash   = min_hash;

        // For each feedthrough we save the possible slot values for
        // the hash calculation.
        if (get_expanded_id(min, ftExpId, &ftContext)) {
            ATH_MSG_WARNING(" *****  Warning cannot get ft expanded id for " << show_to_string(min));
        }

        // Now and save values if we either have an
        // enumerated set of slots, or more than one FT
        for (unsigned int i = 0; i < m_full_feb_range.size(); ++i) {
            if (m_full_feb_range[i].match(ftExpId)) {
                const Range::field& slotField = m_full_feb_range[i][m_slot_index];
                if (slotField.isBounded() || slotField.isEnumerated()) {
                    // save values
                    unsigned int nvalues = slotField.get_indices();
                    hc.m_slot_values.reserve(std::max(hc.m_slot_values.size()*3/2, hc.m_slot_values.size() + nvalues));
                    for (unsigned int j = 0; j < nvalues; ++j) {
                        hc.m_slot_values.push_back(slotField.get_value_at(j));
                    }
                }
            }
        }

        // Set hash calculator
        m_feb_hash_calcs[m_bec_ft_impl.unpack(min)] = std::move(hc);


        if (m_bec_ft_impl.unpack(min) >= size) {
            ATH_MSG_DEBUG("Min > " << size << " " << show_to_string(min)
                          << " " << m_bec_ft_impl.unpack(min) << " " << min_hash);
        }
    }

    // Check feb hash calculation
    for (unsigned int i = 0; i < m_febHashMax; ++i) {
        HWIdentifier id = feb_Id(i);
        if (feb_Hash(id) != i) {
          ATH_MSG_ERROR(" *****  Error feb ranges, id, hash, i = "
                        << show_to_string(id) << " , " << feb_Hash(id) << " , " << i);
        }
    }

    return 0;
}

bool LArOnline_SuperCellID::isHECchannel(const HWIdentifier id) const
/*========================================================*/
{
   int ft = feedthrough(id);
   return ( barrel_ec(id)==1 
        && 
        ( ft==3 || ft==10 || ft==16 || ft==22 )
        );
}


//pos_neg :     0 = negative eta side (C side)
//              1 = positive eta side (A side)

bool LArOnline_SuperCellID::isEMECIW(const HWIdentifier id) const {
  /*======================================================*/
  // 
  int bec= barrel_ec(id);
  int ft = feedthrough(id);
  int sl = slot(id);
  int ch = channel(id);
  bool sideCondition= (pos_neg(id)==1 && ch>95) || (pos_neg(id)==0 && ch<64);
 
  return (bec==1 && sl==2 && sideCondition && (ft==2  || ft==9 || 
			     ft==15 || ft==21)); 
}

bool LArOnline_SuperCellID::isEMECOW(const HWIdentifier id) const {
  /*======================================================*/
  // 
  int bec= barrel_ec(id);
  int ft = feedthrough(id);
  int sl = slot(id);
  int ch = channel(id);
  bool sideCondition=(pos_neg(id)==1 && ch<=95) || (pos_neg(id)==0 && ch>=64);
 
  return (bec == 1 && 
          ((sl == 1 &&  
              (ft == 0 || ft == 1 || ft == 2 || ft == 4 || ft == 5 || ft == 7 || ft == 8 || ft == 9 || ft == 11 || ft == 12 || ft == 13 || ft == 14 || ft == 15 ||
	       ft == 17 || ft == 18 || ft == 19 || ft == 20 || ft == 21 || ft == 23 || ft == 24)) || 
           (sl==2 && sideCondition && (ft==2 ||ft==9 || ft==15 || ft==21))));
}

bool LArOnline_SuperCellID::isEMECchannel(const HWIdentifier id) const {
  int bec= barrel_ec(id);
  int ft = feedthrough(id);
  return (bec == 1 && !( ft==3 || ft==10 || ft==16 || ft==22 || ft==6));
}
