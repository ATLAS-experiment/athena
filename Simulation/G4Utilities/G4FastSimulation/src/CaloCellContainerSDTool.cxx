/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


// Header include
#include "CaloCellContainerSDTool.h"

// CaloCellContainer sensitive detector
#include "CaloCellContainerSD.h"
#include "CaloCellContainerBuilder.h"

#include "HitManagement/HitCollectionMap.h"
#include "StoreGate/WriteHandle.h"

#include <memory>

CaloCellContainerSDTool::CaloCellContainerSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ),
    m_EmptyCellBuilderTool("EmptyCellBuilderTool/EmptyCellBuilderTool"),
    m_FastHitConvertTool ("FastHitConvertTool/FastHitConvertTool")
{
   declareProperty ("EmptyCellBuilderTool", m_EmptyCellBuilderTool, "EmptyCellBuilderTool");
   declareProperty ("FastHitConvertTool", m_FastHitConvertTool, "FastHitConvertTool");
}

StatusCode CaloCellContainerSDTool::initialize()
{
  ATH_CHECK(SensitiveDetectorBase::initialize());
  if (m_outputCollectionNames.size() != 1 ||
      m_outputCollectionNames[0].empty()) {
    ATH_MSG_ERROR("Exactly one non-empty output collection name is required.");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_EmptyCellBuilderTool.retrieve());
  ATH_CHECK(m_FastHitConvertTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode CaloCellContainerSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  const std::string& collectionName = m_outputCollectionNames[0];
  const bool inserted =
    hitCollections.Emplace<CaloCellContainerBuilder>(collectionName).second;
  if (!inserted) {
    ATH_MSG_ERROR("SetupEvent: Collection '" << collectionName
                  << "' already exists.");
    return StatusCode::FAILURE;
  }
  auto* builder = hitCollections.Find<CaloCellContainerBuilder>(collectionName);
  if (!builder) {
    ATH_MSG_ERROR("SetupEvent: Failed to create CaloCellContainerBuilder.");
    return StatusCode::FAILURE;
  }

  if (m_EmptyCellBuilderTool->process(builder->container.get(), ctx).isFailure()) {
    ATH_MSG_ERROR("SetupEvent: EmptyCellBuilderTool failed.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode CaloCellContainerSDTool::Gather(HitCollectionMap& hitCollections)
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  const std::string& collectionName = m_outputCollectionNames[0];
  std::unique_ptr<CaloCellContainerBuilder> builder =
    hitCollections.Extract<CaloCellContainerBuilder>(collectionName);
  if (!builder || !builder->container) {
    ATH_MSG_ERROR("Gather: Failed to retrieve CaloCellContainerBuilder.");
    return StatusCode::FAILURE;
  }

  builder->container->updateCaloIterators();
  if (m_FastHitConvertTool->process(builder->container.get(), ctx).isFailure()) {
    ATH_MSG_ERROR("Gather: FastHitConvertTool failed.");
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<CaloCellContainer> handle{collectionName, ctx};
  ATH_CHECK(handle.record(std::move(builder->container)));
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* CaloCellContainerSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );

  return new CaloCellContainerSD(name(), m_outputCollectionNames[0]);
}
