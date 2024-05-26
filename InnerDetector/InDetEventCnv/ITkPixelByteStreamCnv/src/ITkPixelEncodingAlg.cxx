/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelEncodingAlg.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetRawData/InDetRawDataContainer.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "StoreGate/ReadHandle.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

ITkPixelEncodingAlg::ITkPixelEncodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_hitSortingTool("ITkPixelHitSortingTool", this)
{
  
}


StatusCode ITkPixelEncodingAlg::initialize()
{
  
  ATH_CHECK(m_pixelRDOKey.initialize());
  
  ATH_CHECK(m_hitSortingTool.retrieve());

  return StatusCode::SUCCESS;
}


StatusCode ITkPixelEncodingAlg::execute(const EventContext& ctx) const
{

  SG::ReadHandle<PixelRDO_Container> rdoContainer(m_pixelRDOKey, ctx);

  m_hitSortingTool->sortRDOHits(rdoContainer);

  return StatusCode::SUCCESS;
}