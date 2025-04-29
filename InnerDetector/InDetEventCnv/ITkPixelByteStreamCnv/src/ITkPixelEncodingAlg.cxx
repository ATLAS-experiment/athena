/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelEncodingAlg.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include <bitset>

ITkPixelEncodingAlg::ITkPixelEncodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_hitSortingTool("ITkPixelHitSortingTool", this),
  m_encodingTool("ITkPixelEncodingTool", this),
  m_packingTool("ITkPixelDataPackingTool", this)
{
  
}


StatusCode ITkPixelEncodingAlg::initialize()
{
  
  ATH_CHECK(m_pixelRDOKey.initialize());
  
  ATH_CHECK(m_hitSortingTool.retrieve());

  ATH_CHECK(m_encodingTool.retrieve());

  ATH_CHECK(m_packingTool.retrieve());

  ATH_CHECK(m_EncodedStreamKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode ITkPixelEncodingAlg::execute(const EventContext& ctx) const
{

  SG::ReadHandle<ITkPixelRDO_Container> rdoContainer(m_pixelRDOKey, ctx);

  const std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> EventHitMaps = m_hitSortingTool->sortRDOHits(rdoContainer.get());

  SG::WriteHandle<std::vector<uint32_t>> EncodedStreamCollection = SG::makeHandle(m_EncodedStreamKey, ctx);

  std::vector<uint32_t> streams;
  streams.reserve(10000);

  for (auto iter = EventHitMaps.begin(); iter != EventHitMaps.end(); ++iter) {

    std::vector<uint32_t> encodedStream = m_encodingTool->encodeFE(iter->second);

    ATH_MSG_DEBUG("Encoded stream from chip " << iter->first);
    for (auto& i : encodedStream) ATH_MSG_DEBUG("   " << std::hex << i);

    m_packingTool->pack(&(iter->first), &encodedStream);
    streams.insert(streams.end(), encodedStream.begin(), encodedStream.end());
  }


  ATH_MSG_DEBUG("Storing to SG " << streams.size() << " streams");
  
  ATH_CHECK(EncodedStreamCollection.record(std::make_unique<std::vector<uint32_t>>(streams)));

  return StatusCode::SUCCESS;
}