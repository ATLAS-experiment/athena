/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "HGTD_AlignCondAlg.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "AthenaKernel/IOVInfiniteRange.h"

HGTD_AlignCondAlg::HGTD_AlignCondAlg(const std::string& name,
                                   ISvcLocator* pSvcLocator)
  : AthCondAlgorithm(name, pSvcLocator)
{
}

StatusCode HGTD_AlignCondAlg::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK(m_readKey.initialize(!m_readKey.empty()));
  ATH_CHECK(m_writeKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_detManager, m_detManagerName));
  ATH_MSG_INFO("Detector manager = " << m_detManager);

  return StatusCode::SUCCESS;
}

StatusCode HGTD_AlignCondAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("execute " << name());

  SG::WriteCondHandle<GeoAlignmentStore> writeHandle{m_writeKey, ctx};

  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey()
                  << " is already valid");
    return StatusCode::SUCCESS;
  }

  auto writeCdo = std::make_unique<GeoAlignmentStore>();

  SG::ReadCondHandle<AlignableTransformContainer>
      readHandle{m_readKey, ctx};

  const AlignableTransformContainer* readCdo = *readHandle;

  if (!readCdo) {
      ATH_MSG_FATAL("Cannot retrieve " << m_readKey.key());
      return StatusCode::FAILURE;
  }

  writeHandle.addDependency(readHandle);

  ATH_CHECK(
      m_detManager->align(
          readCdo,
          writeCdo.get()));

  ATH_MSG_INFO("Created GeoAlignmentStore at "
              << writeCdo.get());

  const auto* coll = m_detManager->getDetectorElementCollection();
  for (const auto* el : *coll) {
    if (!el) continue;
    el->getMaterialGeom()->getAbsoluteTransform(writeCdo.get());
    el->getMaterialGeom()->getDefAbsoluteTransform(writeCdo.get());
  }

  writeCdo->lockDelta();
  writeCdo->lockPosCache();

  if (writeHandle.record(std::move(writeCdo)).isFailure()) {
      ATH_MSG_FATAL("Could not record "
                    << writeHandle.key()
                    << " into Conditions Store");
      return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Recorded new CDO "
              << writeHandle.key()
              << " with range "
              << writeHandle.getRange());

  return StatusCode::SUCCESS;

}

