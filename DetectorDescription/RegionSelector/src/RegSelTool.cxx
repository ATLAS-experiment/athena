/**
 **   @file   RegSelTool.cxx         
 **            
 **           Implmentation of a local regionselector tool            
 **            
 **   @author sutt
 **   @date   Sun 22 Sep 2019 10:21:50 BST
 **
 **
 **   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 **/


#include "RegSelLUT/RegSelSiLUT.h"

#include "RegSelTool.h"



//! Constructor
RegSelTool::RegSelTool( const std::string& type, const std::string& name, const IInterface*  parent )
  :  base_class( type, name, parent ),
     m_initialised(false),
     m_dumpTable(false)
{
  //! Declare properties
  declareProperty( "WriteTable",  m_dumpTable,          "write out maps to files for debugging" );
  declareProperty( "Initialised", m_initialised=false,  "flag to determine whether the corresponding subsystem is initilised" );
}


//! Standard destructor
RegSelTool::~RegSelTool() { }



StatusCode RegSelTool::initialize() {
  ATH_MSG_DEBUG( "Initialising RegSelTool " << name() << "\ttable: " << m_tableKey );
  if ( !m_initialised ) { 
    ATH_MSG_WARNING( "Lookup table will not be initialised " << name() << "\tkey " << m_tableKey );
  } 
  ATH_CHECK( m_tableKey.initialize(m_initialised) );
  return StatusCode::SUCCESS;
}



const IRegSelLUT* RegSelTool::lookup( const EventContext& ctx ) const {
  if ( !m_initialised ) return nullptr; 
  SG::ReadCondHandle<IRegSelLUTCondData> table_handle( m_tableKey, ctx ); 
  return (*table_handle)->payload();
}





