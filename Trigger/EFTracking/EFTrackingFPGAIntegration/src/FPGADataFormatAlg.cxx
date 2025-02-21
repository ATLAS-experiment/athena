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
  ATH_CHECK(m_xAODContainerMaker.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatAlg::execute(const EventContext &ctx) const
{
  auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);
  auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);

  std::vector<uint64_t> outputData;

  ATH_CHECK(m_FPGADataFormatTool->convertPixelHitsToFPGADataFormat(*pixelRDOHandle, outputData, ctx));

  // Report the output
  ATH_MSG_DEBUG("ITK pixel encoded data");
  int line = 0;
  for (const auto &var : outputData)
  {
    ATH_MSG_DEBUG("Line: " << line << " data: " << std::hex << var);
    line++;
  }

  outputData.clear();

  ATH_CHECK(m_FPGADataFormatTool->convertStripHitsToFPGADataFormat(*stripRDOHandle, outputData, ctx));

  // Report the output
  ATH_MSG_DEBUG("ITK Strip encoded data");
  line = 0;
  for (const auto &var : outputData)
  {
    ATH_MSG_DEBUG("Line: " << line << " data: " << std::hex << var);
    line++;
  }

  // To make the xAOD, we need the aux data
  std::unique_ptr<EFTrackingDataFormats::Metadata> metadata =
      std::make_unique<EFTrackingDataFormats::Metadata>();
  EFTrackingDataFormats::StripClusterAuxInput scAux;
  EFTrackingDataFormats::PixelClusterAuxInput pcAux;

  // Starting from here validate the conversion tool
  // First read in the test vector
  // Check if the path is set
  // If the path is set, read in the test vector and call the decode function
  EFTrackingFPGAIntegration::TVHolder pixelEDMTV("PixelEDM");
  if (m_pixelEDMRefTVPath.empty())
  {
    ATH_MSG_DEBUG("The path to the pixel EDM reference test vector is not set, skipping");
  }
  else
  {
    ATH_CHECK(m_testVectorTool->prepareTV(m_pixelEDMRefTVPath, pixelEDMTV.refTV));
    ATH_CHECK(m_outputConversionTool->decodePixelEDM(pixelEDMTV.refTV, metadata.get(), pcAux));
  }

  EFTrackingFPGAIntegration::TVHolder stripEDMTV("StripEDM");
  if (m_stripEDMRefTVPath.empty())
  {
    ATH_MSG_DEBUG("The path to the strip EDM reference test vector is not set, skipping");
  }
  else
  {
    ATH_CHECK(m_testVectorTool->prepareTV(m_stripEDMRefTVPath, stripEDMTV.refTV));
    ATH_CHECK(m_outputConversionTool->decodeStripEDM(stripEDMTV.refTV, metadata.get(), scAux));
  }

  EFTrackingFPGAIntegration::TVHolder spacePointTV("SpacePoint");
  if (m_spacePointRefTVPath.empty())
  {
    ATH_MSG_DEBUG("The path to the space point reference test vector is not set, skipping");
  }
  else
  {
    ATH_CHECK(m_testVectorTool->prepareTV(m_spacePointRefTVPath, spacePointTV.refTV));
    ATH_CHECK(m_outputConversionTool->decodeSpacePoints(spacePointTV.refTV, metadata.get()));
  }

  // Print event summary
  if (msgLvl(MSG::DEBUG))
  {
    ATH_MSG_DEBUG("===================Event Summary====================");
    ATH_MSG_DEBUG("Metadata: ");

    ATH_MSG_DEBUG("\tnumOfPixelClusters: " << metadata->numOfPixelClusters);
    ATH_MSG_DEBUG("\tpcRdoIndexSize: " << metadata->pcRdoIndexSize);
    ATH_MSG_DEBUG("Pixel Cluster RDO Index: ");
    for (unsigned int i = 0; i < metadata->pcRdoIndexSize; i++)
    {
      ATH_MSG_DEBUG("\tCluster [" << i << "] has " << metadata->pcRdoIndex[i] << " RDOs");
    }

    // print the pixel cluster rdo list
    ATH_MSG_DEBUG("Pixel Cluster RDO List: ");
    for (const auto &rdo : pcAux.rdoList)
    {
      ATH_MSG_DEBUG("\t" << rdo);
    }

    ATH_MSG_DEBUG("\tnumOfStripClusters: " << metadata->numOfStripClusters);
    ATH_MSG_DEBUG("\tscRdoIndexSize: " << metadata->scRdoIndexSize);
    ATH_MSG_DEBUG("Strip Cluster RDO Index: ");
    for (unsigned int i = 0; i < metadata->scRdoIndexSize; i++)
    {
      ATH_MSG_DEBUG("\tCluster [" << i << "] has " << metadata->scRdoIndex[i] << " RDOs");
    }

    // print the strip cluster rdo list
    ATH_MSG_DEBUG("Strip Cluster RDO List: ");
    for (const auto &rdo : scAux.rdoList)
    {
      ATH_MSG_DEBUG("\t" << rdo);
    }

    // print pcAux
    ATH_MSG_DEBUG("Pixel Cluster Aux: ");
    for (unsigned int i = 0; i < metadata->numOfPixelClusters; i++)
    {
      ATH_MSG_DEBUG("===");
      ATH_MSG_DEBUG("\tCluster [" << i << "] has id: " << pcAux.id[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has idHash: " << pcAux.idHash[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has localPosition x: " << pcAux.localPosition[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has localPosition y: " << pcAux.localPosition[i + 1]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has localCovariance xx: " << pcAux.localCovariance[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has localCovariance yy: " << pcAux.localCovariance[i + 1]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has globalPosition x: " << pcAux.globalPosition[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has globalPosition y: " << pcAux.globalPosition[i + 1]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has globalPosition z: " << pcAux.globalPosition[i + 2]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has channelsInPhi: " << pcAux.channelsInPhi[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has omegaX: " << pcAux.omegaX[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has omegaY: " << pcAux.omegaY[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has totalToT: " << pcAux.totalToT[i]);
    }

    // print scAux
    ATH_MSG_DEBUG("Strip Cluster Aux: ");
    for (unsigned int i = 0; i < metadata->numOfStripClusters; i++)
    {
      ATH_MSG_DEBUG("===");
      ATH_MSG_DEBUG("\tCluster [" << i << "] has id: " << scAux.id[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has idHash: " << scAux.idHash[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has localPosition x: " << scAux.localPosition[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has localCovariance xx: " << scAux.localCovariance[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has globalPosition x: " << scAux.globalPosition[i]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has globalPosition y: " << scAux.globalPosition[i + 1]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has globalPosition z: " << scAux.globalPosition[i + 2]);
      ATH_MSG_DEBUG("\tCluster [" << i << "] has channelsInPhi: " << scAux.channelsInPhi[i]);
    }
    ATH_MSG_DEBUG("===================End of Event Summary====================");
  }

  // Make the xAOD
  // ATH_CHECK(m_xAODContainerMaker->makePixelClusterContainer(pcAux, metadata.get(), ctx));
  // ATH_CHECK(m_xAODContainerMaker->makeStripClusterContainer(scAux, metadata.get(), ctx));

  return StatusCode::SUCCESS;
}
