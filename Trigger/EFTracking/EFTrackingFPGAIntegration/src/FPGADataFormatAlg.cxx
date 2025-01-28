/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */



#include "FPGADataFormatAlg.h"
#include <fstream>

StatusCode FPGADataFormatAlg::initialize()
{
  ATH_CHECK(m_pixelRDOKey.initialize());
  ATH_CHECK(m_stripRDOKey.initialize());

  ATH_CHECK(m_FPGADataFormatTool.retrieve());
  ATH_CHECK(m_testVectorTool.retrieve());
  ATH_CHECK(m_outputConversionTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatAlg::execute(const EventContext &ctx) const
{
  auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);
  auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);

  std::vector<uint64_t> outputData;

  if(!m_FPGADataFormatTool->convertPixelHitsToFPGADataFormat(*pixelRDOHandle, outputData, ctx))
  {
    return StatusCode::FAILURE;
  }


  // Report the output
  ATH_MSG_DEBUG("ITK pixel encoded data");
  int line = 0;
  for(const auto& var: outputData)
  {
    ATH_MSG_DEBUG("Line: "<<line<<" data: "<<std::hex<<var);
    line++;
  }

  outputData.clear();

  if(!m_FPGADataFormatTool->convertStripHitsToFPGADataFormat( *stripRDOHandle, outputData, ctx))
  {
    return StatusCode::FAILURE;
  }

  // Report the output
  ATH_MSG_DEBUG("ITK Strip encoded data");
  line = 0;
  for(const auto& var: outputData)
  {
    ATH_MSG_DEBUG("Line: "<<line<<" data: "<<std::hex<<var);
    line++;
  }

  // Starting from here validate the conversion tool
  // First read in the test vector
  EFTrackingFPGAIntegration::TVHolder pixelClusterTV("PixelCluster");
  ATH_CHECK(m_testVectorTool->prepareTV(m_pixelClusterRefTVPath, pixelClusterTV.refTV));

  EFTrackingFPGAIntegration::TVHolder stripClusterTV("StripCluster");
  ATH_CHECK(m_testVectorTool->prepareTV(m_stripClusterRefTVPath, stripClusterTV.refTV));

  EFTrackingFPGAIntegration::TVHolder pixelL2GTV("PixelL2G");
  ATH_CHECK(m_testVectorTool->prepareTV(m_pixelL2GRefTVPath, pixelL2GTV.refTV));

  EFTrackingFPGAIntegration::TVHolder stripL2GTV("StripL2G");
  ATH_CHECK(m_testVectorTool->prepareTV(m_stripL2GRefTVPath, stripL2GTV.refTV));

  EFTrackingFPGAIntegration::TVHolder spacePointTV("SpacePoint");
  ATH_CHECK(m_testVectorTool->prepareTV(m_spacePointRefTVPath, spacePointTV.refTV));

  // Now decode the TV
  ATH_CHECK(m_outputConversionTool->decodePixelClusters(pixelClusterTV.refTV));
  ATH_CHECK(m_outputConversionTool->decodeStripClusters(stripClusterTV.refTV));
  ATH_CHECK(m_outputConversionTool->decodePixelL2G(pixelL2GTV.refTV));
  ATH_CHECK(m_outputConversionTool->decodeStripL2G(stripL2GTV.refTV));
  ATH_CHECK(m_outputConversionTool->decodeSpacePoints(spacePointTV.refTV));

  return StatusCode::SUCCESS;
}
