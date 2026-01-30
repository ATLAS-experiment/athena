/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/LArMiniFCAL_ID.h"
#include "CaloIdentifier/LArID_Exception.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"
#include "PathResolver/PathResolver.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "CxxUtils/StrFormat.h"

using CxxUtils::strformat;


LArMiniFCAL_ID::LArMiniFCAL_ID()
        : 
        CaloIDHelper ("LArMiniFCAL_ID", "lar_mini_fcal"),
        m_is_initialized(false),
        m_fcal_region_index(0),
        m_LAR_INDEX(999),
        m_MiniFCAL_INDEX(999),
        m_POSNEG_INDEX(999),
        m_MODULE_INDEX(999),
        m_DEPTH_INDEX(999),
        m_ETA_INDEX(999),
        m_PHI_INDEX(999),
        m_two_sym_sides(1)
{}

LArMiniFCAL_ID::~LArMiniFCAL_ID() = default;

int LArMiniFCAL_ID::eta_min(const Identifier modId) const
{
  ExpandedIdentifier expId;
  IdContext module_cntxt = module_context();
  if(!get_expanded_id(modId, expId, &module_cntxt)) {
    int result = -999;
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
      const Range& range = m_full_channel_range[i];
      if (range.match(expId)) {
	const Range::field& eta_field = range[m_ETA_INDEX];
	if (not eta_field.empty()) {
	  int etamin = eta_field.get_minimum();
	  if (-999 == result) {
	    result = etamin;
	  }
	  else {
	    if (etamin < result) result = etamin;
	  }
	}
      }
    }
    return (result);
  }
  return (-999);
}

int LArMiniFCAL_ID::eta_max(const Identifier modId) const
{
  ExpandedIdentifier expId;
  IdContext module_cntxt = module_context();
  if(!get_expanded_id(modId, expId, &module_cntxt)) {
    int result = -999;
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
      const Range& range = m_full_channel_range[i];
      if (range.match(expId)) {
	const Range::field& eta_field = range[m_ETA_INDEX];
	if (not eta_field.empty()) {
	  int etamax = eta_field.get_maximum();
	  if (result < etamax) result = etamax;
	}
      }
    }
    return (result);
  }
  return (-999);  // default
}

int LArMiniFCAL_ID::phi_min_init(const Identifier modId) const
{
  ExpandedIdentifier expId;
  IdContext module_cntxt = module_context();
  if(!get_expanded_id(modId, expId, &module_cntxt)) {
    int result = -999;
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
      const Range& range = m_full_channel_range[i];
      if (range.match(expId)) {
	const Range::field& phi_field = range[m_PHI_INDEX];
	if (not phi_field.empty()) {
	  int phimin = phi_field.get_minimum();
	  if (-999 == result) {
	    result = phimin;
	  }
	  else {
	    if (phimin < result) result = phimin;
	  }
	}
      }
    }
    return (result);
  }
  return (-999);  // default
}

int LArMiniFCAL_ID::phi_max(const Identifier modId) const
{
  ExpandedIdentifier expId;
  IdContext module_cntxt = module_context();
  if(!get_expanded_id(modId, expId, &module_cntxt)) {
    int result = -999;
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
      const Range& range = m_full_channel_range[i];
      if (range.match(expId)) {
	const Range::field& phi_field = range[m_PHI_INDEX];
	if (not phi_field.empty()) {
	  int phimax = phi_field.get_maximum();
	  if (result < phimax) result = phimax;
	}
      }
    }
    return (result);
  }
  return (-999);  // default
}

int   LArMiniFCAL_ID::get_neighbours(const IdentifierHash id, const LArNeighbours::neighbourOption& option, std::vector<IdentifierHash>& neighbourList) const
{
  int result = 1; 

  neighbourList.clear();

  if(!m_do_neighbours) {
    ATH_MSG_WARNING("neighbours not initialized !!! returning empty list");
    return result;
  }

  if(id>=channels().hash_max()) {
    ATH_MSG_WARNING("neighbours requested for  non-existing channel -- id/max " << id << "/"
                    << channel_hash_max());
    return result;
  }

  if( dictionaryVersion() == "fullAtlas" ) {

    if ( (option & LArNeighbours::all2D) == LArNeighbours::all2D ){
      if ( !m_neighbors_2d_vec[(unsigned int)id].empty() ) { 
	neighbourList.insert(neighbourList.end(),
			     m_neighbors_2d_vec[(unsigned int)id].begin(),
			     m_neighbors_2d_vec[(unsigned int)id].end());
      }
    }
    
    if ( (option & LArNeighbours::prevInSamp) ){
      if ( !m_neighbors_3d_prev_vec[(unsigned int)id].empty() ) { 
	neighbourList.insert(neighbourList.end(),
			     m_neighbors_3d_prev_vec[(unsigned int)id].begin(),
			     m_neighbors_3d_prev_vec[(unsigned int)id].end());
      }
    }
    
    if ( (option & LArNeighbours::nextInSamp) ){
      if ( !m_neighbors_3d_next_vec[(unsigned int)id].empty() ) { 
	neighbourList.insert(neighbourList.end(),
			     m_neighbors_3d_next_vec[(unsigned int)id].begin(),
			     m_neighbors_3d_next_vec[(unsigned int)id].end());
      }
    }
    
  } else {
    ATH_MSG_WARNING(" NO MiniFCAL neighbours (yet) in the context of " << dictionaryVersion());
  }
  return result;
}

IdContext	
LArMiniFCAL_ID::module_context 		() const
{
  return region_context();
}

int  LArMiniFCAL_ID::initialize_from_dictionary (const IdDictMgr& dict_mgr)
/*=================================================================*/
{
  ATH_MSG_DEBUG("initialize_from_dictionary");

  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
    return (0);
  }
  else {
    ATH_MSG_DEBUG("(Re)initialize");
  }

  // init base object
  if(CaloIDHelper::initialize_base_from_dictionary(dict_mgr,
                                                   "LArCalorimeter"))
    return (1);
  
  // initialize dictionary version
  AtlasDetectorID::setDictVersion(dict_mgr, "LArCalorimeter");

  // Check whether dictionary has miniFCAL
  if (!dict()->find_group("lar_mini_fcal")) {
    ATH_MSG_INFO("initialize_from_dict - LArCalorimeter dictionary does NOT contain miniFCAL description. Unable to initialize LArMiniFCAL_ID.");
    return 0; // return OK
  }
      

  // Initialize the field indices
  if(initLevelsFromDict()) {
    ATH_MSG_WARNING("initialize_from_dict - cannot initialize MiniFCAL part of LArCalorimeter dictionary");
    return (1);
  }
  else {

    // Find value for the field LAr Calorimeter   
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary ("ATLAS"); 
    int larField   = -1;
    if (atlasDict->get_label_value("subdet", "LArCalorimeter", larField)) {
      ATH_MSG_ERROR("Could not get value for label 'LArCalorimeter' of field 'subdet' in dictionary " << atlasDict->name());
      return (1);
    }


    // Find value for the field LArMiniFCAL  - here the "part" has to be LArFCAL
    int larFcalField   = -1;
    if (dict()->get_label_value("part", "LArFCAL", larFcalField)) {
      ATH_MSG_ERROR("Could not get value for label 'LArMiniFCAL' of field 'part' in dictionary " << atlasDict->name());
      return (1);
    }

    // Set up id for region and range prefix

    // RDS: need to find the range for MiniFCAL module 0, which includes
    // both pos/neg endcaps
    ExpandedIdentifier region_id;
    region_id.add(larField);
    region_id.add(larFcalField);
    Range prefix;
    std::string str = "lar_mini_fcal";
    m_full_channel_range = dict()->build_multirange(region_id, str, prefix);
    m_full_module_range = dict()->build_multirange(region_id, str, prefix, "module");
    m_full_depth_range = dict()->build_multirange(region_id, str, prefix, "depth-mfcal");

    ATH_MSG_DEBUG("initialize_from_dict : ");
    ATH_MSG_DEBUG(" channel range -> " << (std::string)m_full_channel_range);
    ATH_MSG_DEBUG(" module range -> " << (std::string)m_full_module_range);
    ATH_MSG_DEBUG(" depth range -> " << (std::string)m_full_depth_range);

    // Setup the hash tables
    if(init_hashes()) return (1);

    // initilize m_two_sym_sides
    m_two_sym_sides = ( dictionaryVersion() == "fullAtlas" );

    // collect the regions to later access deta/dphi etc.
    if (fill_vec_of_dict_regions()) return 1;

    m_vecOfPhiMin.resize(regions().hash_max());
    for (unsigned int i = 1; i < regions().hash_max(); ++i) {
      Identifier modId = module_id(i); 
      m_vecOfPhiMin[i] = phi_min_init(modId);
    }

    // Setup hash tables for finding neighbors
    if(m_do_neighbours) {
      if(init_neighbors(dict_mgr)) return (1);     
    }

  }

  m_is_initialized = true;
  return 0;

}

void LArMiniFCAL_ID::module_id_checks ( int pos_neg, int /*module*/ ) const
{
	
  // Check that id is within allowed range
  // Fill expanded id
  ExpandedIdentifier expId(lar_fcal_exp());
  //expId << pos_neg << module;
  expId << pos_neg << 0;

  if (!m_full_module_range.match(expId)) { 
    std::string errorMessage = "LArMiniFCAL_ID::module_id() result is not OK: ID, range = "
      + std::string(expId) + " , " + (std::string)m_full_module_range;
    throw LArID_Exception(errorMessage , 6);
  }
}

void LArMiniFCAL_ID::channel_id_checks ( int pos_neg, int /*module*/, int depth, int eta, int phi) const
{
	
  // Check that id is within allowed range
  // Fill expanded id
  ExpandedIdentifier expId(lar_fcal_exp());
  //expId << pos_neg << module << depth << eta << phi ;
  expId << pos_neg << 0 << depth << eta << phi ;

  if (!m_full_channel_range.match(expId)) { 
    std::string errorMessage = "LArMiniFCAL_ID::channel_id() result is not OK: ID, range = "
      + std::string(expId) + " , " + (std::string)m_full_channel_range;
    throw LArID_Exception(errorMessage , 10);
  }
}

void   LArMiniFCAL_ID::channel_id_checks   (const Identifier moduleId, int depth,
				        int eta, int phi) const
{
  // Check that id is within allowed range
  // Fill expanded id
  ExpandedIdentifier expId; 

  IdContext context = module_context();
  if (get_expanded_id(moduleId, expId, &context)) {
    std::string errorMessage = "LArMiniFCAL_ID::channel_id(modId) result is not OK: ID = "
      + show_to_string(moduleId) ;
    throw LArID_Exception(errorMessage , 10);
  }

  expId << depth << eta << phi ;

  if (!m_full_channel_range.match(expId)) { 
    std::string errorMessage = "LArMiniFCAL_ID::channel_id(modId) result is not OK: ID, range = "
      + std::string(expId) + " , " + (std::string)m_full_channel_range;
    throw LArID_Exception(errorMessage , 10);
  }
}

int  LArMiniFCAL_ID::get_expanded_id  (const Identifier& id, ExpandedIdentifier& exp_id, const IdContext* context) const
{
    // We assume that the context is >= region
    exp_id.clear();
    exp_id << lar_field_value()
      	   << lar_fcal_field_value()
	   << pos_neg(id)
      	   << module(id);
    if (context && context->end_index() >= m_DEPTH_INDEX) {
      exp_id << depth(id);
      if (context->end_index() >= m_ETA_INDEX) {
        exp_id << eta(id);
        if (context->end_index() >= m_PHI_INDEX) {
          exp_id << phi(id);
        }
      }
    }
    return 0;
}

int         LArMiniFCAL_ID::initLevelsFromDict()
{
  if(!dict()) {
    ATH_MSG_ERROR("initLevelsFromDict - dictionary NOT initialized");
    return (1);
  }

  // Find out which identifier field corresponds to each level.

  m_fcal_region_index = 999 ;
  m_LAR_INDEX         = 999 ;
  m_MiniFCAL_INDEX    = 999 ;
  m_POSNEG_INDEX      = 999 ;
  m_MODULE_INDEX      = 999 ;
  m_DEPTH_INDEX       = 999 ;
  m_ETA_INDEX         = 999 ;
  m_PHI_INDEX         = 999 ;

  const IdDictField* field = dict()->find_field("subdet") ;
  if (field) {
    m_LAR_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'subdet' field");
    return (1);
  }

  field = dict()->find_field("part") ;
  if (field) {
    m_MiniFCAL_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'part' field");
    return (1);
  }

  field = dict()->find_field("barrel-endcap") ;
  if (field) {
    m_POSNEG_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'barrel-endcap' field");
    return (1);
  }
  
  field = dict()->find_field("module") ;
  if (field) {
    m_MODULE_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'module' field");
    return (1);
  }

  
  field = dict()->find_field("depth-mfcal") ;
  if (field) {
    m_DEPTH_INDEX = field->index();
  }
  else {
    ATH_MSG_WARNING("initLevelsFromDict - unable to find 'depth' field");
    return (1);
  }
  
  
  field = dict()->find_field("eta-mfcal") ;
  if (field) {
    m_ETA_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'eta' field");
    return (1);
  }
  
  field = dict()->find_field("phi-mfcal") ;
  if (field) {
    m_PHI_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'phi' field");
    return (1);
  }
  
  // Save index to a MiniFCAL region for unpacking

  // Fill expanded identifier with id for module 0 of the MiniFCAL,
  // i.e. add on module == 0 to the id below

  ExpandedIdentifier id(lar_fcal_exp());
  if (dict()->find_region(id,m_fcal_region_index)){  
    ATH_MSG_ERROR("initLevelsFromDict - unable to find fcal region index: id, reg " << id << " " << m_fcal_region_index);
    return (1);
  }

   // Set the field implementations


  const IdDictRegion& region = *dict()->find_region ("", "lar_mini_fcal");

  m_lar_impl      = region.implementation(m_LAR_INDEX);
  m_fcal_impl     = region.implementation(m_MiniFCAL_INDEX);
  m_pn_impl       = region.implementation(m_POSNEG_INDEX);
  m_module_impl   = region.implementation(m_MODULE_INDEX);
  m_depth_impl    = region.implementation(m_DEPTH_INDEX);
  m_eta_impl      = region.implementation(m_ETA_INDEX);
  m_phi_impl      = region.implementation(m_PHI_INDEX);

  ATH_MSG_DEBUG("decode index and bit fields for each level: ");
  ATH_MSG_DEBUG("lar   " << m_lar_impl.show_to_string());
  ATH_MSG_DEBUG("fcal  " << m_fcal_impl.show_to_string());
  ATH_MSG_DEBUG("pn    " << m_pn_impl.show_to_string());
  ATH_MSG_DEBUG("mod   " << m_module_impl.show_to_string());
  ATH_MSG_DEBUG("depth " << m_depth_impl.show_to_string());
  ATH_MSG_DEBUG("eta   " << m_eta_impl.show_to_string());
  ATH_MSG_DEBUG("phi   " << m_phi_impl.show_to_string());

  return(0) ;
}

int         LArMiniFCAL_ID::init_hashes()
{
  if (channels().init (*this, "channels",
                       m_full_channel_range,
                       &LArMiniFCAL_ID::channel_id,
                       m_PHI_INDEX))
    return 1;
  if (regions().init (*this, "regions",
                      m_full_module_range,
                      &LArMiniFCAL_ID::module_id,
                      m_MODULE_INDEX))
    return 1;

  return (0);
}


int         LArMiniFCAL_ID::init_neighbors_from_file(const std::string& filename, std::vector<std::set<IdentifierHash> > & vec)
{
  ATH_MSG_DEBUG("init_neighbors_from_file");
  // Find the full path to filename:
  std::string file = PathResolver::find_file (filename, "DATAPATH");
  ATH_MSG_INFO("Reading file " << file);

  std::ifstream fin;
  if (!file.empty()) {
    fin.open(file.c_str());
  }
  else {
    ATH_MSG_ERROR("Could not find input file " << filename);
    return 1;
  }
  if (fin.bad()) {
    ATH_MSG_ERROR("Could not open file " << file);
    return 1;
  }

  //
  // Parse the input file
  //

  vec.resize(channels().hash_max());
  
  char aLine[1024];
  std::string sLine;
  bool isComment = true;
  char AorC,dot;
  int isamp,idepth, iphi,ieta;

  while( isComment ) {
    sLine.resize( 0 );
    do {  
      fin.getline(aLine,sizeof(aLine)-1);
      sLine = std::string(aLine);
    } while (sLine.empty() && !fin.eof());
    isComment = ( sLine.find('#') != std::string::npos );
  }
  do {
    while ( sLine.empty() && !fin.eof()) {
      fin.getline(aLine,sizeof(aLine)-1);
      sLine = std::string(aLine);
    }
    std::istringstream es( sLine.c_str() );

    if ( es >> AorC >> isamp >> dot >> idepth >> dot >> iphi >> dot >> ieta ) {
      Identifier thisCell = channel_id((AorC=='A'?2:-2),isamp,idepth,ieta,iphi);
      IdentifierHash ic = channel_hash(thisCell);
      while ( es >> AorC >> isamp >> dot >> idepth >> dot >> iphi >> dot >> ieta ) {
        if (!ic.is_valid()) continue;
	Identifier nextCell = channel_id((AorC=='A'?2:-2),isamp,idepth,ieta,iphi);
	IdentifierHash inext = channel_hash(nextCell);
	vec[ic].insert(vec[ic].end(),inext);
      }
      sLine.resize(0);
    } 
  }while (!fin.eof()) ;
  fin.close();

  return 0;
}

int         LArMiniFCAL_ID::init_neighbors(const IdDictMgr& dict_mgr) 
{
    ATH_MSG_DEBUG("init_neighbors");

    int status;
    std::string f2d,f3dnext,f3dprev;

    f2d     = dict_mgr.find_metadata("FCAL2DNEIGHBORS");
    f3dnext = dict_mgr.find_metadata("FCAL3DNEIGHBORSNEXT");
    f3dprev = dict_mgr.find_metadata("FCAL3DNEIGHBORSPREV");
    if (f2d.empty() || f3dnext.empty() || f3dprev.empty()) {
        ATH_MSG_ERROR("init_neighbours: cannot find neighbours files: "
                << " f2d: " << f2d << " f3dnext: " << f3dnext << " f3dprev: " << f3dprev);
        throw std::runtime_error("LArMiniFCAL_ID::init_neighbours: Cannot find the FCAL Neighbour file names");
    }
    
    status = init_neighbors_2d(f2d);
    if ( status == 0 ) 
        status = init_neighbors_3d_next(f3dnext);
    if ( status == 0 ) 
        status = init_neighbors_3d_prev(f3dprev);
    
    if ( status == 0 ) 
        return (0);
    else 
        return (1);
}



