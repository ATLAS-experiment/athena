/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingAlg.h"
#include "StoreGate/ReadHandle.h"

ITkPixelDecodingAlg::ITkPixelDecodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_packingTool("ITkPixelDataPackingTool", this),
  m_decodingTool("ITkPixelDecodingTool", this),
  m_hitSortingTool("ITkPixelHitSortingTool", this)
{
  
}


StatusCode ITkPixelDecodingAlg::initialize()
{

  ATH_CHECK(m_EncodedStreamKey.initialize());

  ATH_CHECK(m_decodingTool.retrieve());

  ATH_CHECK(m_hitSortingTool.retrieve());

  ATH_CHECK(m_packingTool.retrieve());

  return StatusCode::SUCCESS;
}


StatusCode ITkPixelDecodingAlg::execute(const EventContext& ctx) const
{

  SG::ReadHandle<ITkPacketCollection> EncodedStreamCollection(m_EncodedStreamKey, ctx);

  std::map<ITkPixelOnlineId, ITkPixLayout<uint16_t>> EventHitMaps; 
  for(auto encodedStream : *EncodedStreamCollection){

    ITkPixelDataPackingTool::UnpackedStream unpackedStream = m_packingTool->unpack(&encodedStream);
    EventHitMaps[unpackedStream.onlineID] = *m_decodingTool->decodeStream(&unpackedStream.dataStream);

    // Add some tool to get into RDO form -- already exists?

  }

  std::shared_ptr<ITkPixelRDO_Container> pixelRDOContainer = std::make_shared<ITkPixelRDO_Container>(100); //dummy // will need to setup the container
  ATH_CHECK(m_hitSortingTool->createRDO(EventHitMaps, pixelRDOContainer.get()));
  
  //store the filled RDO container to SG

  return StatusCode::SUCCESS;
}


