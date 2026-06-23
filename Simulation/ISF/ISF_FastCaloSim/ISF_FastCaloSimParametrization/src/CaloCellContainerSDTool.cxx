/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// Header include
#include "CaloCellContainerSDTool.h"

// CaloCellContainer sensitive detector
#include "ISF_FastCaloSimParametrization/CaloCellContainerBuilder.h"
#include "ISF_FastCaloSimParametrization/CaloCellContainerSD.h"

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
  ATH_CHECK(m_EmptyCellBuilderTool.retrieve());
  ATH_CHECK(m_FastHitConvertTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode CaloCellContainerSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  // Get current event context
  const EventContext& ctx = Gaudi::Hive::currentContext();

  const std::string& collectionName = m_outputCollectionNames[0];
  hitCollections.Emplace<CaloCellContainerBuilder>(collectionName);
  auto* builder = hitCollections.Find<CaloCellContainerBuilder>(collectionName);
  if (!builder) {
    ATH_MSG_ERROR("SetupEvent: Failed to create CaloCellContainerBuilder.");
    return StatusCode::FAILURE;
  }

  // Initialize cell container with empty cells.
  if(m_EmptyCellBuilderTool->process(builder->container.get(), ctx).isFailure()){
    ATH_MSG_ERROR("SetupEvent: Failed to process calo cell container with the empty cell builder tool.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode CaloCellContainerSDTool::Gather(HitCollectionMap& hitCollections)
{
  // Get current event context
  const EventContext& ctx = Gaudi::Hive::currentContext();

  const std::string& collectionName = m_outputCollectionNames[0];
  std::unique_ptr<CaloCellContainerBuilder> builder =
    hitCollections.Extract<CaloCellContainerBuilder>(collectionName);
  if (!builder || !builder->container) {
    ATH_MSG_ERROR("Gather: Failed to retrieve CaloCellContainerBuilder.");
    return StatusCode::FAILURE;
  }

  // Update the calo iterators of the calo cell container.
  builder->container->updateCaloIterators();

  // Convert FastCaloSim hits into HitCollections, taking into account sampling fractions.
  if(m_FastHitConvertTool->process(builder->container.get(), ctx).isFailure()){
    ATH_MSG_ERROR("Gather: Failed to process calo cell container with the fast hit convert tool.");
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<CaloCellContainer> handle{collectionName, ctx};
  ATH_CHECK(handle.record(std::move(builder->container)));
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* CaloCellContainerSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );

  // Create a fresh SD
  return new CaloCellContainerSD(name(), m_outputCollectionNames[0]);
}
