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

  return StatusCode::SUCCESS;
}
