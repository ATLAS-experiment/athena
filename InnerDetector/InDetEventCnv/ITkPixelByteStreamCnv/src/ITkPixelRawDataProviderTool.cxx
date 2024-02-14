/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "ITkPixelRawDataProviderTool.h"

ITkPixelRawDataProviderTool::ITkPixelRawDataProviderTool( const std::string& type, const std::string& name,
  const IInterface* parent ):AthAlgTool(type, name, parent){
  declareInterface< IITkPixelRawDataProviderTool  >( this );
}

StatusCode 
ITkPixelRawDataProviderTool::initialize(){
  return StatusCode::SUCCESS;
}

StatusCode 
ITkPixelRawDataProviderTool::finalize(){
  return StatusCode::SUCCESS;
}

StatusCode 
ITkPixelRawDataProviderTool::convert( std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& /*vecRobs*/,
  IPixelRDO_Container* /*rdoIdc*/, const EventContext& /*ctx*/) const{
	return StatusCode::SUCCESS;	      
}


