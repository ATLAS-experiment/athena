/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/LArFCAL_Base_ID.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "PathResolver/PathResolver.h"

#include "CxxUtils/StrFormat.h"
#include <iostream>
#include <fstream>

using CxxUtils::strformat;

#define MAX_BUFFER_LEN 1024


LArFCAL_Base_ID::LArFCAL_Base_ID(const std::string& name,
                                 const std::string& group,
                                 bool supercell)
  : CaloIDHelper (name, group),
    m_slar (supercell ? 1 : 0)
{
}


bool LArFCAL_Base_ID::is_supercell (const Identifier id)const
{
  return(m_slar_impl.unpack(id)) != 0;
}

int LArFCAL_Base_ID::eta_min(const Identifier modId) const
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

int LArFCAL_Base_ID::eta_max(const Identifier modId) const
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

int LArFCAL_Base_ID::phi_min_init(const Identifier modId) const
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

int LArFCAL_Base_ID::phi_max(const Identifier modId) const
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

IdContext	
LArFCAL_Base_ID::module_context 		() const
{
  return region_context();
}

int  LArFCAL_Base_ID::initialize_base_from_dictionary (const IdDictMgr& dict_mgr,
						       const std::string& group_name)
/*=================================================================*/
{
  ATH_MSG_DEBUG("initialize_from_dictionary");

  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    ATH_MSG_DEBUG("Request to reinitialize not satisfied - tags have not changed");
    return (0);
  } else {
    ATH_MSG_DEBUG("(Re)initialize");
  }

  // init base object
  if(CaloIDHelper::initialize_base_from_dictionary(dict_mgr,
                                                   "LArCalorimeter"))
    return (1);

  // initialize dictionary version
  AtlasDetectorID::setDictVersion(dict_mgr, "LArCalorimeter");

  // Initialize the field indices
  if (initLevelsFromDict(group_name)) return (1);

  // Find value for the field LAr Calorimeter   
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary ("ATLAS"); 
    int larField   = -1;
    if (atlasDict->get_label_value("subdet", "LArCalorimeter", larField)) {
      ATH_MSG_ERROR("Could not get value for label 'LArCalorimeter' of field 'subdet' in dictionary " << atlasDict->name());
      return (1);
    }


  // Find value for the field LArFCAL
    int larFcalField   = -1;
    if (dict()->get_label_value("part", "LArFCAL", larFcalField)) {
      ATH_MSG_ERROR("Could not get value for label 'LArFCAL' of field 'part' in dictionary " << atlasDict->name());
      return (1);
    }

    // Set up id for region and range prefix

    ExpandedIdentifier region_id;
    region_id.add(larField);
    region_id.add(larFcalField);
    Range prefix;
    m_full_channel_range = dict()->build_multirange(region_id, group_name, prefix);
    m_full_module_range = dict()->build_multirange(region_id, group_name, prefix, "module");

    ATH_MSG_DEBUG(" initialize_from_dict : ");
    ATH_MSG_DEBUG(" channel range -> " << (std::string)m_full_channel_range);
    ATH_MSG_DEBUG(" module range -> " << (std::string)m_full_module_range);

    // Setup the hash tables
    if(init_hashes()) return (1);

    // initilize m_two_sym_sides
    m_two_sym_sides = ( dictionaryVersion() == "fullAtlas" );

    if (fill_vec_of_dict_regions (group_name)) return 1;

    m_vecOfPhiMin.resize(regions().hash_max());
    for (unsigned int i = 1; i < regions().hash_max(); ++i) {
      Identifier modId = module_id(i); 
      m_vecOfPhiMin[i] = phi_min_init(modId);
    }
        
  // Setup hash tables for finding neighbours
  if(m_do_neighbours) {
    if(init_neighbours(dict_mgr)) return (1);     
  }
  
  return 0;
    
}

void LArFCAL_Base_ID::module_id_checks ( int pos_neg, int module ) const
{
	
  // Check that id is within allowed range
  // Fill expanded id
  ExpandedIdentifier expId(lar_fcal_exp());
  expId << pos_neg << module;

  if (!m_full_module_range.match(expId)) { 
    std::string errorMessage = "LArFCAL_Base_ID::module_id() result is not OK: ID, range = "
      + std::string(expId) + " , " + (std::string)m_full_module_range;
    throw LArID_Exception(errorMessage , 6);
  }
}

void LArFCAL_Base_ID::channel_id_checks ( int pos_neg, int module, int eta, int phi) const
{
	
  // Check that id is within allowed range
  // Fill expanded id
  ExpandedIdentifier expId(lar_fcal_exp());
  expId << pos_neg << module << eta << phi << m_slar ;

  if (!m_full_channel_range.match(expId)) { 
    std::string errorMessage = "LArFCAL_Base_ID::channel_id() result is not OK: ID, range = "
      + std::string(expId) + " , " + (std::string)m_full_channel_range;
    throw LArID_Exception(errorMessage , 10);
  }
}

void   LArFCAL_Base_ID::channel_id_checks   (const Identifier moduleId,
				        int eta, int phi) const
{
  // Check that id is within allowed range
  // Fill expanded id
  ExpandedIdentifier expId; 

  IdContext context = module_context();
  if (get_expanded_id(moduleId, expId, &context)) {
    std::string errorMessage = "LArFCAL_Base_ID::channel_id(modId) result is not OK: ID = "
      + show_to_string(moduleId) ;
    throw LArID_Exception(errorMessage , 10);
  }

  expId << eta << phi << m_slar ;

  if (!m_full_channel_range.match(expId)) { 
    std::string errorMessage = "LArFCAL_Base_ID::channel_id(modId) result is not OK: ID, range = "
      + std::string(expId) + " , " + (std::string)m_full_channel_range;
    throw LArID_Exception(errorMessage , 10);
  }
}

int  LArFCAL_Base_ID::get_expanded_id  (const Identifier& id, ExpandedIdentifier& exp_id, const IdContext* context) const
{
    // We assume that the context is >= region
    exp_id.clear();
    exp_id << lar_field_value()
      	   << lar_fcal_field_value()
	   << pos_neg(id)
	   << module(id);
    if(context && context->end_index() >= m_ETA_INDEX) {
	exp_id << eta(id);
	if(context->end_index() >= m_PHI_INDEX) {
	    exp_id << phi(id);
	    if ( context->end_index() >= m_SLAR_INDEX) {
              exp_id << (unsigned)is_supercell(id);
            }
	}
    }
    return (0);
}

int         LArFCAL_Base_ID::initLevelsFromDict(const std::string& /*group_name*/)
{
  if(!dict()) {
    ATH_MSG_ERROR("initLevelsFromDict - dictionary NOT initialized ");
    return (1);
  }

  // Find out which identifier field corresponds to each level.

  m_fcal_region_index = 999 ;
  m_LAR_INDEX        = 999 ;
  m_FCAL_INDEX       = 999 ;
  m_POSNEG_INDEX     = 999 ;
  m_MODULE_INDEX     = 999 ;
  m_ETA_INDEX        = 999 ;
  m_PHI_INDEX        = 999 ;
  m_SLAR_INDEX       = 999 ;

  // Save index to a FCAL region for unpacking
  ExpandedIdentifier id(lar_fcal_exp());
  if (dict()->find_region(id,m_fcal_region_index)){
    ATH_MSG_ERROR("initLevelsFromDict - unable to find fcal region index: id, reg " << id << m_fcal_region_index);
    return (1);
  }

  const IdDictField* field = dict()->find_field("subdet") ;
  if (field) {
    m_LAR_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'subdet' field ");
    return (1);
  }

  field = dict()->find_field("part") ;
  if (field) {
    m_FCAL_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'part' field ");
    return (1);
  }

  field = dict()->find_field("barrel-endcap") ;
  if (field) {
    m_POSNEG_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'barrel-endcap' field ");
    return (1);
  }
  
  field = dict()->find_field("module") ;
  if (field) {
    m_MODULE_INDEX = field->index();
  }
  else {
    if(dictionaryVersion() != "H8TestBeam" ) {
      ATH_MSG_ERROR("initLevelsFromDict - unable to find 'module' field ");
    }
    return (1);
  }

  field = dict()->find_field("eta-fcal") ;
  if (field) {
    m_ETA_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'eta' field ");
    return (1);
  }
  
  field = dict()->find_field("phi-fcal") ;
  if (field) {
    m_PHI_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'phi' field ");
    return (1);
  }
  
  field = dict()->find_field("is-slar-fcal") ;
  if (field) {
    m_SLAR_INDEX = field->index();
  }
  else {
    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'is-slar-fcal' field");
    return (1);
  }

  // Set the field implementations

  const IdDictRegion& region = dict()->region(m_fcal_region_index);

  /*
  std::cout << "LArFCAL_Base_ID::initLevelsFromDict - found levels " << std::endl ;
  std::cout << "part           " << m_FCAL_INDEX     << std::endl ;
  std::cout << "pos-neg        " << m_POSNEG_INDEX   << std::endl ;
  std::cout << "module         " << m_MODULE_INDEX   << std::endl ;
  std::cout << "eta            " << m_ETA_INDEX      << std::endl ;
  std::cout << "phi            " << m_PHI_INDEX      << std::endl ;
  */

  m_lar_impl      = region.implementation(m_LAR_INDEX);
  m_fcal_impl     = region.implementation(m_FCAL_INDEX);
  m_pn_impl       = region.implementation(m_POSNEG_INDEX);
  m_module_impl   = region.implementation(m_MODULE_INDEX);
  m_eta_impl      = region.implementation(m_ETA_INDEX);
  m_phi_impl      = region.implementation(m_PHI_INDEX);
  m_slar_impl     = region.implementation(m_SLAR_INDEX);

  ATH_MSG_DEBUG("decode index and bit fields for each level:");
  ATH_MSG_DEBUG("lar  "  << m_lar_impl.show_to_string());
  ATH_MSG_DEBUG("fcal "  << m_fcal_impl.show_to_string());
  ATH_MSG_DEBUG("pn   "  << m_pn_impl.show_to_string());
  ATH_MSG_DEBUG("mod  "  << m_module_impl.show_to_string());
  ATH_MSG_DEBUG("eta  "  << m_eta_impl.show_to_string());
  ATH_MSG_DEBUG("phi  "  << m_phi_impl.show_to_string());
  ATH_MSG_DEBUG("is-slar  "  << m_slar_impl.show_to_string());

  return(0) ;
}

int         LArFCAL_Base_ID::init_hashes()
{
  if (channels().init (*this, "channels",
                       m_full_channel_range,
                       &LArFCAL_Base_ID::channel_id,
                       m_SLAR_INDEX))
    return 1;
  if (regions().init (*this, "regions",
                      m_full_module_range,
                      &LArFCAL_Base_ID::module_id,
                      m_MODULE_INDEX))
    return 1;

  return (0);
}

int   LArFCAL_Base_ID::get_neighbours(const IdentifierHash id, const LArNeighbours::neighbourOption& option, 
				 std::vector<IdentifierHash>& neighbourList) const
{
  int result = 1; 

  neighbourList.clear();

  if(!m_do_neighbours) {
    ATH_MSG_WARNING("neighbours not initialized !!! returning empty list");
    return result;
  }

  if(id>=channel_hash_max()) {
    ATH_MSG_WARNING("neighbours requested for non-existing channel -- id/max " << id << "/" << channel_hash_max());
    return result;
  }

  if( dictionaryVersion() == "fullAtlas" || dictionaryVersion() == "H6TestBeam" ) {

    if ( (option & LArNeighbours::all2D) == LArNeighbours::all2D ){
      if ( !m_neighbours_2d_vec[(unsigned int)id].empty() ) { 
	neighbourList.insert(neighbourList.end(),
			     m_neighbours_2d_vec[(unsigned int)id].begin(),
			     m_neighbours_2d_vec[(unsigned int)id].end());
      }
    }
    
    if ( (option & LArNeighbours::prevInSamp) ){
      if ( !m_neighbours_3d_prev_vec[(unsigned int)id].empty() ) { 
	neighbourList.insert(neighbourList.end(),
			     m_neighbours_3d_prev_vec[(unsigned int)id].begin(),
			     m_neighbours_3d_prev_vec[(unsigned int)id].end());
      }
    }
    
    if ( (option & LArNeighbours::nextInSamp) ){
      if ( !m_neighbours_3d_next_vec[(unsigned int)id].empty() ) { 
	neighbourList.insert(neighbourList.end(),
			     m_neighbours_3d_next_vec[(unsigned int)id].begin(),
			     m_neighbours_3d_next_vec[(unsigned int)id].end());
      }
    }
    
  } else {
    ATH_MSG_WARNING(" NO FCAL neighbours (yet) in the context of " << dictionaryVersion());
  }
  return result;
}

int         LArFCAL_Base_ID::init_neighbours_from_file(const std::string& filename, std::vector<std::set<IdentifierHash> > & vec)
{
  ATH_MSG_DEBUG("init_neighbours_from_file");
  // Find the full path to filename:
  std::string file = PathResolver::find_file (filename, "DATAPATH");
  ATH_MSG_DEBUG("Reading file " << file);
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

  vec.resize(channel_hash_max());
  
  char aLine[MAX_BUFFER_LEN];
  std::string sLine;
  bool isComment = true;
  char AorC,dot;
  int isamp,iphi,ieta;

  while( isComment ) {
    sLine.resize( 0 );
    do {  
      fin.getline(aLine,sizeof(aLine)-1);
      sLine = std::string(aLine);
    } while (sLine.empty() && !fin.eof());
    isComment = ( sLine.find('#') != std::string::npos );
  }
  do {
    unsigned int ic, inext;
    Identifier thisCell,nextCell;
    while ( sLine.empty() && !fin.eof()) {
      fin.getline(aLine,sizeof(aLine)-1);
      sLine = std::string(aLine);
    }
    std::istringstream es( sLine.c_str() );

    if ( es >> AorC >> isamp >> dot >> iphi >> dot >> ieta ) {
      thisCell = channel_id((AorC=='A'||AorC=='S'?2:-2),isamp,ieta,iphi);
      ic = channel_hash(thisCell);
      while ( es >> AorC >> isamp >> dot >> iphi >> dot >> ieta ) {
	nextCell = channel_id((AorC=='A'||AorC=='S'?2:-2),isamp,ieta,iphi);
	inext = channel_hash(nextCell);
	vec[ic].insert(vec[ic].end(),inext);
      }
      sLine.resize(0);
    } 
  }while (!fin.eof()) ;
  fin.close();

  return 0;
}

int
LArFCAL_Base_ID::init_neighbours(const IdDictMgr& dict_mgr) 
{
    
    ATH_MSG_DEBUG("init_neighbours");

    int status;
    std::string f2d,f3dnext,f3dprev;

    if ( m_slar ) {
      f2d     = "FCalSuperCells2DNeighborsNew-April2014.txt";
      f3dnext = "FCalSuperCells3DNeighborsNextNew-April2014.txt"; 
      f3dprev = "FCalSuperCells3DNeighborsPrevNew-April2014.txt"; 
    }
    else {
      f2d     = dict_mgr.find_metadata("FCAL2DNEIGHBORS");
      f3dnext = dict_mgr.find_metadata("FCAL3DNEIGHBORSNEXT");
      f3dprev = dict_mgr.find_metadata("FCAL3DNEIGHBORSPREV");
    }
    if (f2d.empty() || f3dnext.empty() || f3dprev.empty()) {
        ATH_MSG_ERROR("init_neighbours: cannot find neighbours files: f2d: " << f2d << " f3dnext: " << f3dnext << " f3dprev: " << f3dprev);
        throw std::runtime_error("LArFCAL_Base_ID::init_neighbours: Cannot find the FCAL Neighbour file names");
    }
    
    status = init_neighbours_2d(f2d);
    if ( status == 0 ) 
        status = init_neighbours_3d_next(f3dnext);
    if ( status == 0 ) 
        status = init_neighbours_3d_prev(f3dprev);

    ATH_MSG_DEBUG("init_neighbours status: " << status);
    
    if ( status == 0 ) 
        return (0);
    else 
        return (1);
}

