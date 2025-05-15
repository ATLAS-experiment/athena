/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RecursiveGeometryProcessor.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// STL
#include <sstream>
// Trk include
#include "TrkDetDescrTestTools/RecursiveGeometryProcessor.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/Layer.h"
#include "TrkVolumes/Volume.h"
#include "TrkSurfaces/Surface.h"

// constructor
Trk::RecursiveGeometryProcessor::RecursiveGeometryProcessor(const std::string& t, const std::string& n, const IInterface* p)
: AthAlgTool(t,n,p)
{
    declareInterface<Trk::IGeometryProcessor>(this);
}

// destructor
Trk::RecursiveGeometryProcessor::~RecursiveGeometryProcessor()
= default;


// the interface method initialize
StatusCode Trk::RecursiveGeometryProcessor::initialize()
{
    ATH_MSG_INFO( "initialize()" );
    return StatusCode::SUCCESS;
}

// the interface method finalize
StatusCode Trk::RecursiveGeometryProcessor::finalize()
{
    ATH_MSG_INFO( "finalize() successful" );
    return StatusCode::SUCCESS;
}

// Processor Action to work on TrackingGeometry
StatusCode Trk::RecursiveGeometryProcessor::process(Trk::TrackingGeometry& tgeo) const {

  ATH_MSG_VERBOSE("Start processing the TrackingGeometry recursively");
  // retrieve the highest tracking volume
  Trk::TrackingVolume* worldVolume = tgeo.highestTrackingVolume();
  if (worldVolume){
      ATH_MSG_VERBOSE("TrackingVolume '" << worldVolume->volumeName() << "' retrieved as highest level node.");
      return process(*worldVolume, 0);
  }
  // abort job
  ATH_MSG_FATAL("No highest level TrackingVolume found. Stopping recursive parsing, abort job.");
  return StatusCode::FAILURE;
}

// Processor Action to work on TrackingVolumes
StatusCode Trk::RecursiveGeometryProcessor::process(Trk::TrackingVolume& tvol, size_t level) const {

  std::stringstream displayBuffer;
  for (size_t il = 0; il < level; ++il) displayBuffer << " ";
  // formatted screen output
  ATH_MSG_VERBOSE(displayBuffer.str() << "TrackingVolume '" << tvol.volumeName() << "'");

  // create the action on the volume part of the TrackingVolume
  if (processNode(tvol, level).isFailure() ){
      ATH_MSG_FATAL("Failed to call processNode(const TrackingVolume&). Aborting.");
      return StatusCode::FAILURE;
  }

  // Process the contained layers if they exist
  Trk::LayerArray* layerArray = tvol.confinedLayers();
  if (layerArray) {
      // display output
      auto layers = layerArray->arrayObjects();
      ATH_MSG_VERBOSE(displayBuffer.str() << "--> has " << layers.size() << " confined layers." );
      for (const auto & layIter : layers){
          if (!layIter)
             ATH_MSG_WARNING("Zero-pointer found in LayerArray - indicates problem !");
          if ((layIter) && process(*layIter, level).isFailure()){
             ATH_MSG_FATAL("Failed to call process(const Layer&) on confined layers. Aborting.");
             return StatusCode::FAILURE;
          }
      }
   }

   // Process the boundary surface layers
   auto bSurfaces = tvol.boundarySurfaces();
   for (auto & bSurface : bSurfaces){
       if (bSurface->surfaceRepresentation().associatedLayer()){
           ATH_MSG_VERBOSE(displayBuffer.str() << "--> has a boundary layer." );
           if (process(
                 const_cast<Trk::Layer&>(
                   *bSurface->surfaceRepresentation().associatedLayer()),level)
                 .isFailure()) {
             ATH_MSG_FATAL("Failed to call process(const Layer&) on boundary "
                           "layer. Aborting.");
             return StatusCode::FAILURE;
           }
       }
   }


   // Process the contained TrackingVolumes (recursively) if they exist
   Trk::BinnedArray< Trk::TrackingVolume >* confinedVolumes = tvol.confinedVolumes();
   // register the next round
   if (confinedVolumes) {
       auto volumes = confinedVolumes->arrayObjects();
       for (const auto & volumesIter : volumes){
           if (!volumesIter)
              ATH_MSG_WARNING("Zero-pointer found in VolumeArray - indicates problem !");
           if (volumesIter && process(*volumesIter, ++level).isFailure() ){
               ATH_MSG_FATAL("Failed to call process(const TrackingVolume&) on confined volumes. Aborting.");
               return StatusCode::FAILURE;
           }
       }
   }

   // return
   return StatusCode::SUCCESS;

}

// Processor Action to work on Layers
StatusCode Trk::RecursiveGeometryProcessor::process(Trk::Layer& lay, size_t level) const {

    std::stringstream displayBuffer;
    for (size_t il = 0; il < level; ++il) displayBuffer << " ";
    ATH_MSG_VERBOSE(displayBuffer.str() << "   processing Layer with Index: " << lay.layerIndex() );

    // process the node itself
    if (processNode(lay, level).isFailure()){
        ATH_MSG_FATAL("Failed to call processNode(const Layer&). Aborting.");
        return StatusCode::FAILURE;
    }
    // get the subsurface array
    Trk::SurfaceArray* surfArray = lay.surfaceArray();
    if (surfArray) {
        std::span<Trk::Surface * const > layerSurfaces = surfArray->arrayObjects();
        ATH_MSG_VERBOSE(displayBuffer.str() << "   ---> has " << layerSurfaces.size() << " surfaces on the layer.");

        auto laySurfIter    = layerSurfaces.begin();
        auto laySurfIterEnd = layerSurfaces.end();
        // loop over the surfaces and draw them
        for ( ; laySurfIter != laySurfIterEnd; ++laySurfIter) {
             if (!(*laySurfIter))
                 ATH_MSG_WARNING("Zero-pointer found in SurfaceArray - indicates problem !");
             if ((*laySurfIter) && process(**laySurfIter, level).isFailure()){
                 ATH_MSG_FATAL("Failed to call process(const Surface&) on confined layer surfaces. Aborting.");
                 return StatusCode::FAILURE;
             }
        }
    }
    // return SUCCESS
    return StatusCode::SUCCESS;
}

// Processor Action to work on Surfaces
StatusCode Trk::RecursiveGeometryProcessor::process(Trk::Surface& surf, size_t level) const {
    return processNode(surf, level);
}

// Processor Action to work on TrackingVolume - to be overloaded
StatusCode Trk::RecursiveGeometryProcessor::processNode(const Trk::TrackingVolume&, size_t ) const {
    return StatusCode::SUCCESS;
}

// Processor Action to work on Layers - to be overloaded
StatusCode Trk::RecursiveGeometryProcessor::processNode(const Trk::Layer&, size_t ) const {
    return StatusCode::SUCCESS;
}
// Processor Action to work on Surfaces - to be overloaded
StatusCode Trk::RecursiveGeometryProcessor::processNode(const Trk::Surface&, size_t ) const {
    return StatusCode::SUCCESS;
}
