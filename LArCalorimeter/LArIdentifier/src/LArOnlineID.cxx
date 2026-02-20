/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdentifier/LArOnlineID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/RangeIterator.h"
#include "LArIdentifier/LArOnlID_Exception.h"
#include <cmath>
#include <set>
#include <string>

/* See comments in Base class */

LArOnlineID::LArOnlineID() :
  LArOnlineID_Base("LArOnlineID", "LArOnline", false)
{
}


LArOnlineID::~LArOnlineID() = default;

/* =================================================================== */
int  LArOnlineID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
/* =================================================================== */
{
    ATH_MSG_INFO("initialize_from_dictionary");

    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
        return (0);
    } else {
        ATH_MSG_DEBUG("(Re)initialize");
    }

    // init base object
    if(AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return (1);
    m_dict = dict_mgr.find_dictionary ("LArCalorimeter"); 
    if(!m_dict) {
        ATH_MSG_ERROR("initialize_from_dictionary - cannot access LArCalorimeter dictionary");
        return 1;
    }

    // Register version of the dictionary used
    if (register_dict_tag(dict_mgr, "LArCalorimeter")) return(1);

    // initialize dictionary version
    AtlasDetectorID::setDictVersion(dict_mgr, "LArCalorimeter");

    /* Initialize the field indices */
//    if(initLevelsFromDict()) return (1);
    if(LArOnlineID_Base::initLevelsFromDict(group())) return (1);


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
    m_full_laronline_range = m_dict->build_multirange( region_id , group(), prefix);
    m_full_feb_range       = m_dict->build_multirange( region_id , group(), prefix, "slot");
    m_full_feedthrough_range = m_dict->build_multirange( region_id , group(), prefix, "feedthrough");

    ATH_MSG_DEBUG("initialize_from_dictionary :");
    ATH_MSG_DEBUG("feedthrough range -> " + (std::string)m_full_feedthrough_range);
    ATH_MSG_DEBUG("feedthrough slot range -> " + (std::string)m_full_feb_range);
    ATH_MSG_DEBUG("channel range -> " + (std::string)m_full_laronline_range);

    /* Setup the hash tables */
    ATH_MSG_DEBUG("[initialize_from_dictionary] version= " << dictionaryVersion());

    if( dictionaryVersion() == "fullAtlas" ) {
        if(LArOnlineID_Base::init_hashes()) return (1);
        if(init_calib_hashes()) return (1);
    }
    if( dictionaryVersion() == "H8TestBeam" ) {
        if(init_H8Hashes()) return (1);
        if(init_calib_hashes()) return (1);
    }
    if( dictionaryVersion() == "H6TestBeam" ) {
        ATH_MSG_DEBUG("[initialze_from_dictionary] ...call init_H6hashes..");
        if(init_H6Hashes()) return (1);
        if(init_calib_hashes()) return (1);
    }
    ATH_MSG_DEBUG("initialize_from_dictionary -> calibModuleHash= " << m_calibModuleHashMax); 
    ATH_MSG_DEBUG("initialize_from_dictionary -> calibChannelHash= " << m_calibChannelHashMax); 

  
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

        // For each feedthrough we must check to see if the slot
        // values are enumerated or not. If they are enumerate we must
        // save the values in order to calculate the fed hash
        if (get_expanded_id(min, ftExpId, &ftContext)) {
          ATH_MSG_WARNING(" *****  Warning cannot get ft expanded id for " << show_to_string(min));
        }
        unsigned int nrangesFound = 0;
        for (unsigned int i = 0; i < m_full_feb_range.size(); ++i) {
            if (m_full_feb_range[i].match(ftExpId)) {
                nrangesFound += 1;
                const Range::field& slotField = m_full_feb_range[i][m_slot_index];
                if (slotField.isEnumerated()) {
                    // save values
                    hc.m_slot_values = slotField.get_values();
                }
            }
        }

        // Similarly, if there is more than one range per feedthrough,
        // this means that slot values are not a continuous range. In
        // this case, as well, we save all possible slot values
        if (nrangesFound > 1) {
            for (unsigned int i = 0; i < m_full_feb_range.size(); ++i) {
                if (m_full_feb_range[i].match(ftExpId)) {
                    const Range::field& slotField = m_full_feb_range[i][m_slot_index];
                    if (slotField.isBounded()) {
                        // save values
                        unsigned int nvalues = slotField.get_maximum() - slotField.get_minimum() + 1;
                        hc.m_slot_values.reserve(std::max(hc.m_slot_values.size()*3/2, hc.m_slot_values.size() + nvalues));
                        for (unsigned int j = 0; j < nvalues; ++j) {
                            hc.m_slot_values.push_back(j + slotField.get_minimum());
                        }
                    }
                    else {
                      ATH_MSG_WARNING(" *****  Warning feb range slot field is NOT both_bounded - id, slot mode: " 
                                      << show_to_string(min) << " enumerated");
                    }
                }
            }
        }

        
        // Set hash calculator
        m_feb_hash_calcs[m_bec_ft_impl.unpack(min)] = std::move(hc);


        if (m_bec_ft_impl.unpack(min) >= size) {
            ATH_MSG_DEBUG("Min > " << size << " " <<
                          show_to_string(min) << " " <<
                          std::to_string(m_bec_ft_impl.unpack(min)) << " " <<
                          std::to_string(min_hash));
        }
    }

    // Check feb hash calculation
    for (unsigned int i = 0; i < m_febHashMax; ++i) {
        HWIdentifier id = feb_Id(i);
        if (feb_Hash(id) != i) {
            ATH_MSG_WARNING(" *****  Warning feb ranges, id, hash, i = " <<
                            show_to_string(id) << " , " << std::to_string(feb_Hash(id)) << " , " << i);
        }
    }

    return 0;
}


/*========================================*/
int LArOnlineID::init_H8Hashes()
/*========================================*/
{
  /* Channel hash */
  unsigned int nids=0;
  std::set<HWIdentifier> ids;
  ATH_MSG_DEBUG("[init_H8hashes] > ChannelId : m_full_laronline_range.size() = " << m_full_laronline_range.size());
  for (unsigned int i = 0; i < m_full_laronline_range.size(); ++i) 
    {
      const Range& range = m_full_laronline_range[i];
      ConstRangeIterator rit(range);
      for (const auto & exp_id : rit) 
        {
          HWIdentifier id = this->channel_Id (exp_id[m_bec_index],
                                              exp_id[m_side_index],
                                              exp_id[m_feedthrough_index],
                                              exp_id[m_slot_index],
                                              exp_id[m_channel_in_slot_index]);
          if(!(ids.insert(id)).second)
            {
              ATH_MSG_ERROR("init_hashes : duplicated id for channel nb = "+std::to_string(nids));
              ATH_MSG_ERROR("expanded Id= "+show_to_string(id));
            }
          nids++;
        }
    }
  unsigned int nidtb=0;
  std::set<HWIdentifier>::const_iterator first = ids.begin();
  std::set<HWIdentifier>::const_iterator last  = ids.end();
  for (;first != last && nidtb < nids; ++first) 
    {
      if( is_H8(*first) )
        {
          m_channel_vec.push_back(*first);
          nidtb++;
        } 
    }
  m_channelHashMax = m_channel_vec.size();


  /* FEB hash */
  /*==========*/
  nids = 0;
  ids.clear();
  ATH_MSG_DEBUG("[init_H8hashes] > FebId : m_full_feb_range.size() = " << m_full_feb_range.size());
  for (unsigned int i = 0; i < m_full_feb_range.size(); ++i) 
    {
      const Range& range = m_full_feb_range[i];
      ConstRangeIterator rit(range);
      for (const auto & exp_id : rit) 
        {
          HWIdentifier febId = feb_Id( exp_id[m_bec_index],
                                       exp_id[m_side_index],
                                       exp_id[m_feedthrough_index],
                                       exp_id[m_slot_index] );
          if(!(ids.insert(febId)).second)
            {
              ATH_MSG_ERROR("init_hashes: duplicated id for FEB nb = "+std::to_string(nids));
              ATH_MSG_ERROR("expanded Id= "+show_to_string(febId));
            }
          nids++;
        }
    }
  nidtb=0;
  first = ids.begin();
  last  = ids.end();
  for (;first != last && nidtb < nids; ++first) 
    {
      if( is_H8(*first) )
        {
          m_feb_vec.push_back(*first);
          nidtb++;
        }
    }
  m_febHashMax = m_feb_vec.size();

  /* Feedthrough hash */
  /*=================*/
  nids = 0;
  ids.clear();
  ATH_MSG_DEBUG("[init_H8hashes] FeedthroughId: m_feedthrough_range.size() = " << m_full_feedthrough_range.size());
  for (unsigned int i = 0; i < m_full_feedthrough_range.size(); ++i) 
    {
      const Range& range = m_full_feedthrough_range[i];
      ConstRangeIterator rit(range);
      for (const auto & exp_id:rit) 
        {
          HWIdentifier feedthroughId = feedthrough_Id( exp_id[m_bec_index],
                                                       exp_id[m_side_index],
                                                       exp_id[m_feedthrough_index] );
          int test_bec = barrel_ec( feedthroughId);
          int test_pn  = pos_neg( feedthroughId);
          int test_ft = feedthrough( feedthroughId);
          ATH_MSG_VERBOSE("[init_H8hashes] in loop : [bec,pn,ft]= [" << test_bec 
                          << "," << test_pn << "," << test_ft << "]");
          if(!(ids.insert(feedthroughId)).second)
            {
              ATH_MSG_ERROR("init_hashes : duplicated id for feedthrough nb = "+std::to_string(nids));
              ATH_MSG_ERROR("expanded Id= "+show_to_string(feedthroughId));
            }
          nids++;
        }
    }
  nidtb=0;
  first = ids.begin();
  last  = ids.end();
  for (;first != last && nidtb < nids; ++first) 
    {
      if( is_H8(*first) )
        {
          m_feedthrough_vec.push_back(*first);
          nidtb++;
        }
    }
  m_feedthroughHashMax = m_feedthrough_vec.size();
  ATH_MSG_DEBUG("[init_H8hashes] final m_feedthroughHashMax = " << m_feedthroughHashMax);

  return (0);
}


/*========================================*/
int LArOnlineID::init_H6Hashes()
/*========================================*/
{
  unsigned int nids=0;
  std::set<HWIdentifier> ids;
  ATH_MSG_DEBUG("[init_H6hashes] : ChannelId : m_full_laronline_range.size() = " 
                << m_full_laronline_range.size());
  for (unsigned int i = 0; i < m_full_laronline_range.size(); ++i) 
    {
      const Range& range = m_full_laronline_range[i];
      ConstRangeIterator rit(range);
      for (const auto & exp_id:rit) 
        {
          HWIdentifier id = this->channel_Id (exp_id[m_bec_index],
                                              exp_id[m_side_index],
                                              exp_id[m_feedthrough_index],
                                              exp_id[m_slot_index],
                                              exp_id[m_channel_in_slot_index]);
          if(!(ids.insert(id)).second)
            {
              ATH_MSG_ERROR("init_hashes: duplicated id for channel nb = "+std::to_string(nids));
              ATH_MSG_ERROR("expanded Id= "+show_to_string(id));
            }
          nids++;
        }
    }
  unsigned int nidtb=0;
  std::set<HWIdentifier>::const_iterator first = ids.begin();
  std::set<HWIdentifier>::const_iterator last  = ids.end();
  for (;first != last && nidtb < nids; ++first) 
    {
      if( is_H6(*first) )
        {
          m_channel_vec.push_back(*first);
          nidtb++;
        } 
    }
  m_channelHashMax = m_channel_vec.size();

  /* FEB hash */
  /*==========*/
  nids = 0;
  ids.clear();
  ATH_MSG_DEBUG("[init_H6hashes] > FebId : m_full_feb_range.size() = " << m_full_feb_range.size());
  for (unsigned int i = 0; i < m_full_feb_range.size(); ++i) 
    {
      const Range& range = m_full_feb_range[i];
      ConstRangeIterator rit(range);
      for (const auto & exp_id : rit) 
        {
          HWIdentifier febId = feb_Id( exp_id[m_bec_index],
                                       exp_id[m_side_index],
                                       exp_id[m_feedthrough_index],
                                       exp_id[m_slot_index] );
          if(!(ids.insert(febId)).second)
            {
              ATH_MSG_ERROR("init_hashes : duplicated id for FEB nb = "+std::to_string(nids));
              ATH_MSG_ERROR("expanded Id= "+show_to_string(febId));
            }
          nids++;
        }
    }
  nidtb=0;
  first = ids.begin();
  last  = ids.end();
  for (;first != last && nidtb < nids; ++first) 
    {
      if( is_H6(*first) )
        {
          m_feb_vec.push_back(*first);
          nidtb++;
        }
    }
  m_febHashMax = m_feb_vec.size();


  /* Feedthrough hash */
  /*=================*/
  nids = 0;
  ids.clear();
  // AL-->
  ATH_MSG_DEBUG("[init_H6hashes] FeedthroughId: m_feedthrough_range.size() = " << m_full_feedthrough_range.size());
  for (unsigned int i = 0; i < m_full_feedthrough_range.size(); ++i) 
    {
      const Range& range = m_full_feedthrough_range[i];
      ConstRangeIterator rit(range);
      for (const auto & exp_id:rit) 
        {
          HWIdentifier feedthroughId = feedthrough_Id( exp_id[m_bec_index],
                                                       exp_id[m_side_index],
                                                       exp_id[m_feedthrough_index] );
          ATH_MSG_DEBUG("[init_H6hashes] m_bec_index= " << m_bec_index 
                        << "m_side_index= " << m_bec_index 
                        << "m_feedthrough_index= " << m_bec_index 
                        << "m_slot_index= " << m_bec_index);
          int test_bec = barrel_ec( feedthroughId);
          int test_pn  = pos_neg( feedthroughId);
          int test_ft = feedthrough( feedthroughId);
          ATH_MSG_DEBUG("[init_H6hashes] in loop : [bec,pn,ft]= [" << test_bec 
                        << "," << test_pn << "," << test_ft << "]");
          if(!(ids.insert(feedthroughId)).second)
            {
              ATH_MSG_ERROR("init_hashes : duplicated id for feedthrough nb = "+std::to_string(nids));
              ATH_MSG_ERROR("expanded Id= "+show_to_string(feedthroughId));
            }
          nids++;
        }
    }
  nidtb=0;
  first = ids.begin();
  last  = ids.end();
  for (;first != last && nidtb < nids; ++first) 
    {
      if( is_H6FT( *first ))
        {
          ATH_MSG_DEBUG("[init_H6hashes] filling m_feedthrough_vec IS-H6 !!");
          m_feedthrough_vec.push_back(*first);
          nidtb++;
        }
    }
  m_feedthroughHashMax = m_feedthrough_vec.size();
  ATH_MSG_DEBUG("[init_H6hashes] final m_feedthroughHashMax = " << m_feedthroughHashMax);

  return (0);
}

bool LArOnlineID::isHECchannel(const HWIdentifier id) const
/*========================================================*/
{
   int ft = feedthrough(id);
   return ( barrel_ec(id)==1 
        && 
        ( ft==3 || ft==10 || ft==16 || ft==22 )
        &&
        slot(id) > 2 );
}

bool LArOnlineID::isEMECIW(const HWIdentifier id) const {
  /*======================================================*/
  // 
  int bec= barrel_ec(id);
  int ft = feedthrough(id);
  int sl = slot(id);
  return (bec==1 && sl<3 && (ft==3  || ft==10 || 
			     ft==16 || ft==22)); 
}

bool LArOnlineID::isEMECOW(const HWIdentifier id) const {
  /*======================================================*/
  // 
  int bec= barrel_ec(id);
  int ft = feedthrough(id);
  return (bec==1 && 
	  (ft==0 || ft==1 ||
	   ft==2 || ft==4 ||
	   ft==5 || ft==7 ||
	   ft==8 || ft==9 ||
	   ft==11|| ft==12||
	   ft==13|| ft==14||
	   ft==15|| ft==17||
	   ft==18|| ft==19||
	   ft==20|| ft==21||
	   ft==23|| ft==24 )
	  );
}

bool LArOnlineID::isEMECchannel(const HWIdentifier id) const
/*========================================================*/
{/* redefinition with isEMECIW and isEMECOW */
  return (LArOnlineID::isEMECOW(id) || LArOnlineID::isEMECIW(id));
}
