/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelEncodingAlg.h"

ITkPixelEncodingAlg::ITkPixelEncodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_cnvTool("ITkPixelCnvTool", this)
{
  
}


StatusCode ITkPixelEncodingAlg::initialize(){
  
  ATH_CHECK(m_pixelRDOKey.initialize());
  
  ATH_CHECK(m_cnvTool.retrieve());

  return StatusCode::SUCCESS;
}

/**
* @brief Convert PixelRDO_Container into bytestream
*/

StatusCode ITkPixelEncodingAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<PixelRDO_Container> rdoContainer(m_pixelRDOKey, ctx);

  ATH_CHECK(m_cnvTool->convertToByteStream(rdoContainer.get()));

  return StatusCode::SUCCESS;
}