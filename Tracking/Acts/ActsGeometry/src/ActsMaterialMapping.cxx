/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsMaterialMapping.h"

// ATHENA
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsGeometry/IActsPropStepRootWriterSvc.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ISvcLocator.h"

// ACTS
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Definitions/Units.hpp"

// PACKAGE
#include "ActsInterop/Logger.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/IActsMaterialJsonWriterTool.h"
#include "ActsGeometryInterfaces/IActsMaterialStepConverterTool.h"
#include "ActsGeometryInterfaces/IActsMaterialTrackWriterSvc.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IActsSurfaceMappingTool.h"
#include "ActsGeometryInterfaces/IActsVolumeMappingTool.h"

// STL
#include <fstream>
#include <string>

//TEST
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"

StatusCode ActsMaterialMapping::initialize() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

  if(!m_mapSurfaces && !m_mapVolumes){
    ATH_MSG_ERROR("No element to map onto defined.");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_materialStepConverterTool.retrieve() );
  ATH_CHECK(m_materialTrackWriterSvc.retrieve() );
  if(m_mapSurfaces){
    ATH_CHECK(m_surfaceMappingTool.retrieve() );
    m_mappingState = m_surfaceMappingTool->mappingState();
  }
  if(m_mapVolumes){
    ATH_CHECK(m_volumeMappingTool.retrieve() );
    m_mappingStateVol = m_volumeMappingTool->mappingState();
  }
  ATH_CHECK(m_materialJsonWriterTool.retrieve() );
  ATH_CHECK( m_inputMaterialStepCollection.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode ActsMaterialMapping::execute() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<Trk::MaterialStepCollection> materialStepCollection(m_inputMaterialStepCollection, ctx);

  if (materialStepCollection.isValid() and not materialStepCollection->empty()) {
    Acts::RecordedMaterialTrack mTrack = m_materialStepConverterTool->convertToMaterialTrack(*materialStepCollection);

    if(m_mapSurfaces){
      auto context = m_trackingGeometryTool->getNominalGeometryContext().context();;
      m_mappingState->geoContext = std::reference_wrapper<const Acts::GeometryContext>(context);
      m_surfaceMappingTool->mapper()->mapMaterialTrack(*m_mappingState, mTrack);
    }
    if(m_mapVolumes){
      auto context = m_trackingGeometryTool->getNominalGeometryContext().context();
      m_mappingStateVol->geoContext = std::reference_wrapper<const Acts::GeometryContext>(context);
      m_volumeMappingTool->mapper()->mapMaterialTrack(*m_mappingStateVol, mTrack);
    }
    m_materialTrackWriterSvc->write(mTrack);
  }
  return StatusCode::SUCCESS;
}

StatusCode ActsMaterialMapping::finalize() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  Acts::DetectorMaterialMaps detectorMaterial;

  // Finalize all the maps using the cached state
  if(m_mapSurfaces && m_mapVolumes){
    m_surfaceMappingTool->mapper()->finalizeMaps(*m_mappingState);
    m_volumeMappingTool->mapper()->finalizeMaps(*m_mappingStateVol);
    // Loop over the state, and collect the maps for surfaces
    for (auto& [key, value] : m_mappingState->surfaceMaterial) {
      detectorMaterial.first.insert({key, std::move(value)});
    }
    // Loop over the state, and collect the maps for volumes
    for (auto& [key, value] : m_mappingStateVol->volumeMaterial) {
      detectorMaterial.second.insert({key, std::move(value)});
    }
  }
  else{
    if(m_mapSurfaces){
      m_surfaceMappingTool->mapper()->finalizeMaps(*m_mappingState);
      // Loop over the state, and collect the maps for surfaces
      for (auto& [key, value] : m_mappingState->surfaceMaterial) {
        detectorMaterial.first.insert({key, std::move(value)});
      }
      // Loop over the state, and collect the maps for volumes
      for (auto& [key, value] : m_mappingState->volumeMaterial) {
        detectorMaterial.second.insert({key, std::move(value)});
      }
    }
    if(m_mapVolumes){
      m_volumeMappingTool->mapper()->finalizeMaps(*m_mappingStateVol);
      // Loop over the state, and collect the maps for surfaces
      for (auto& [key, value] : m_mappingStateVol->surfaceMaterial) {
        detectorMaterial.first.insert({key, std::move(value)});
      }
      // Loop over the state, and collect the maps for volumes
      for (auto& [key, value] : m_mappingStateVol->volumeMaterial) {
        detectorMaterial.second.insert({key, std::move(value)});
      }
    }
  }
  
  const auto & context = m_trackingGeometryTool->getNominalGeometryContext();
  m_materialJsonWriterTool->write(context, detectorMaterial);

  return StatusCode::SUCCESS;

}
