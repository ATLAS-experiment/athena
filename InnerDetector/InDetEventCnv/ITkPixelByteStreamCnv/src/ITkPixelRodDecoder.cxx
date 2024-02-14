/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelRodDecoder.h"

ITkPixelRodDecoder::ITkPixelRodDecoder(const std::string& type, 
  const std::string& name,
  const IInterface* parent):AthAlgTool(type,name,parent){
  declareInterface< IITkPixelRodDecoder  >( this );
}

StatusCode 
ITkPixelRodDecoder::initialize(){
  return StatusCode::SUCCESS;
}

StatusCode 
ITkPixelRodDecoder::finalize(){
  return StatusCode::SUCCESS;
}

StatusCode 
ITkPixelRodDecoder::fillCollection  (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment */*robFrag*/,
  IPixelRDO_Container* /*rdoIdc*/,
	std::vector<IdentifierHash>* /*vecHash*/, 
	const EventContext& /*ctx*/) const{
	return StatusCode::SUCCESS;
			
}