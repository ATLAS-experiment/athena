/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "HGTDAlignCondAlg.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "AthenaKernel/IOVInfiniteRange.h"

HGTDAlignCondAlg::HGTDAlignCondAlg(const std::string& name,
                                   ISvcLocator* pSvcLocator)
  : AthCondAlgorithm(name, pSvcLocator)
{
}

StatusCode HGTDAlignCondAlg::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK(m_writeKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_detManager, m_detManagerName));

  return StatusCode::SUCCESS;
}

StatusCode HGTDAlignCondAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("execute " << name());

  SG::WriteCondHandle<GeoAlignmentStore> writeHandle{m_writeKey, ctx};

  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey()
                  << " is already valid");
    return StatusCode::SUCCESS;
  }

  auto writeCdo = std::make_unique<GeoAlignmentStore>();

  if (writeHandle.record(IOVInfiniteRange::infiniteMixed(),
                         std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record " << writeHandle.key()
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("recorded new CDO " << writeHandle.key()
               << " with range " << writeHandle.getRange());

  return StatusCode::SUCCESS;
}
