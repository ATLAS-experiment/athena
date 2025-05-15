/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LayerMaterialProvider.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// STL
#include <sstream>
#include <atomic>
// Trk include
#include "TrkDetDescrTools/LayerMaterialProvider.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/LayerMaterialMap.h"
#include "TrkVolumes/Volume.h"
#include "TrkSurfaces/Surface.h"
#include "StoreGate/ReadCondHandle.h"
#include "CxxUtils/checker_macros.h"

// constructor
Trk::LayerMaterialProvider::LayerMaterialProvider(const std::string& t, const std::string& n, const IInterface* p)
: base_class(t,n,p)
{
  declareProperty ("LayerMaterialMapName", m_layerMaterialMapName,
                   "If LayerMaterialMapKey is not set, then fall back to retrieving this from the detector store.");
}

// destructor
Trk::LayerMaterialProvider::~LayerMaterialProvider()
= default;


StatusCode Trk::LayerMaterialProvider::initialize()
{
  ATH_CHECK( m_layerMaterialMapKey.initialize (SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}

// Processor Action to work on TrackingGeometry
StatusCode
Trk::LayerMaterialProvider::process(Trk::TrackingGeometry& tgeo) const
{
  const LayerMaterialMap* layerMaterialMap = nullptr;
  if (!m_layerMaterialMapKey.key().empty()) {
    SG::ReadCondHandle<LayerMaterialMap> layerMaterialMapH (m_layerMaterialMapKey);
    layerMaterialMap = *layerMaterialMapH;
  }
  else {
    ATH_CHECK( detStore()->retrieve (layerMaterialMap, m_layerMaterialMapName) );
  }
  dumpMaterialMap (*layerMaterialMap);

  ATH_MSG_VERBOSE("Start processing the TrackingGeometry recursively");
  // retrieve the highest tracking volume
  Trk::TrackingVolume* worldVolume = tgeo.highestTrackingVolume();
  // check for the world volume
  if (worldVolume){
      // TrackingVolume : confined layers
      ATH_MSG_VERBOSE("TrackingVolume '" << worldVolume->volumeName() << "' retrieved as highest level node.");
      if (process(*worldVolume, *layerMaterialMap, 0).isFailure() ) {
          ATH_MSG_FATAL("Could not load material maps for provided TrackingGeometry, abort job.");
          return StatusCode::FAILURE;
      }
      // Boundary layers
      if (!tgeo.boundaryLayers().empty()){
          ATH_MSG_VERBOSE("TrackingGeometry has " <<  tgeo.numBoundaryLayers() << " unique boundary layers, loading material.");
          for (const auto& bLayerIter : tgeo.boundaryLayers() ){
              Trk::Layer* lay = bLayerIter.first;
              int layCount = bLayerIter.second;
              int layIndex = lay->layerIndex().value();
              // only move on if layer index is different from 0
              if (layIndex){
                  StatusCode sc( process(*lay, *layerMaterialMap, 0) );
		  // @TODO Currently recoverable errors are treated as failure. Is this the intended behaviour ? Elsewhere recoverable errors are treated as recoverable
                  if (sc.isSuccess())
                      ATH_MSG_DEBUG("---[B] Boundary layer with " << layCount << " references : successfully loaded material map for layer " << layIndex );
                  // else if (sc.isRecoverable())
                  //    ATH_MSG_WARNING("Failed to call process(const Layer&) on layers - but recoverable.");
                  else {
                      ATH_MSG_FATAL("Failed to call process(const Layer&) on layer. Aborting.");
                      return StatusCode::FAILURE;
                  }
              }
           } // loop over layers
       }// we have boundary layers
       return StatusCode::SUCCESS;
  } // we have a world volume
  // abort job
  ATH_MSG_FATAL("No highest level TrackingVolume found. Stopping recursive parsing, abort job.");
  return StatusCode::FAILURE;
}

// Processor Action to work on TrackingVolumes
StatusCode
Trk::LayerMaterialProvider::process(Trk::TrackingVolume& tvol,
                                    size_t level) const
{
  const LayerMaterialMap* layerMaterialMap = nullptr;
  if (!m_layerMaterialMapKey.key().empty()) {
    SG::ReadCondHandle<LayerMaterialMap> layerMaterialMapH (m_layerMaterialMapKey);
    layerMaterialMap = *layerMaterialMapH;
  }
  else {
    ATH_CHECK( detStore()->retrieve (layerMaterialMap, m_layerMaterialMapName) );
  }
  dumpMaterialMap (*layerMaterialMap);
  ATH_CHECK( process (tvol, *layerMaterialMap, level) );
  return StatusCode::SUCCESS;
}

StatusCode
Trk::LayerMaterialProvider::process(Trk::TrackingVolume& tvol,
                                    const LayerMaterialMap& layerMaterialMap,
                                    size_t level) const
{
  std::stringstream displayBuffer;
  for (size_t il = 0; il < level; ++il) displayBuffer << " ";
  // formatted screen output
  ATH_MSG_VERBOSE(displayBuffer.str() << "TrackingVolume '" << tvol.volumeName() << "'");

  // @TODO add boundary surfaces

  // Process the contained layers
  Trk::LayerArray* layerArray = tvol.confinedLayers();
  if (layerArray) {
      // display output
      std::span<Trk::Layer * const> layers = layerArray->arrayObjects();
      ATH_MSG_VERBOSE(displayBuffer.str() << "--> has " << layers.size() << " confined layers." );
      for ( const auto & layIter : layers ){
          if (!layIter)
              ATH_MSG_WARNING("Zero-pointer found in LayerArray - indicates problem !");
          else {
            // get the layer index and only process if it's an indexed layer
            int layIndex = layIter->layerIndex().value();
            if (layIndex){
                StatusCode sc = process(*layIter, layerMaterialMap, level);
                if (sc.isSuccess())
                    ATH_MSG_DEBUG(displayBuffer.str() << "---[M] Material layer: successfully loaded material map for layer " << layIndex );
                else if (sc.isRecoverable())
                    ATH_MSG_WARNING("Failed to call process(const Layer&) on layers - but recoverable.");
                else {
                    ATH_MSG_FATAL("Failed to call process(const Layer&) on layer. Aborting.");
                    return StatusCode::FAILURE;
                }
            } else
                ATH_MSG_DEBUG(displayBuffer.str() << "---[o] Navigation layer: skipping.");
        }
      }
   }

   // Process the contained TrackingVolumes (recursively) if they exist
   Trk::BinnedArray<Trk::TrackingVolume >* confinedVolumes = tvol.confinedVolumes();
   // register the next round
   if (confinedVolumes) {
       std::span<Trk::TrackingVolume * const> volumes = confinedVolumes->arrayObjects();
       std::span<Trk::TrackingVolume * const>::iterator volumesIter = volumes.begin();
       for (; volumesIter != volumes.end(); ++volumesIter){
           if (!(*volumesIter))
              ATH_MSG_WARNING("Zero-pointer found in VolumeArray - indicates problem !");
           if ((*volumesIter) && process(**volumesIter, layerMaterialMap, ++level).isFailure() ){
               ATH_MSG_FATAL("Failed to call process(const TrackingVolume&) on confined volumes. Aborting.");
               return StatusCode::FAILURE;
           }
       }
   }

   // return
   return StatusCode::SUCCESS;
}

// Processor Action to work on Layers
StatusCode
Trk::LayerMaterialProvider::process(Trk::Layer& lay, size_t level) const
{
  const LayerMaterialMap* layerMaterialMap = nullptr;
  if (!m_layerMaterialMapKey.key().empty()) {
    SG::ReadCondHandle<LayerMaterialMap> layerMaterialMapH(
      m_layerMaterialMapKey);
    layerMaterialMap = *layerMaterialMapH;
  } else {
    ATH_CHECK(detStore()->retrieve(layerMaterialMap, m_layerMaterialMapName));
  }
  dumpMaterialMap(*layerMaterialMap);
  ATH_CHECK(process(lay, *layerMaterialMap, level));
  return StatusCode::SUCCESS;
}

StatusCode
Trk::LayerMaterialProvider::process(Trk::Layer& lay,
                                    const LayerMaterialMap& layerMaterialMap,
                                    size_t level) const
{
  // skip Layers w/o material
  if (!lay.layerMaterialProperties())
    return StatusCode::SUCCESS;

  // get the layer index for assignment
  Trk::LayerIndex lIndex = lay.layerIndex();

  // display
  std::stringstream displayBuffer;
  for (size_t il = 0; il < level; ++il)
    displayBuffer << " ";

  // find the layer and assign the material properties
  auto lmIter = layerMaterialMap.find(lIndex);
  if (lmIter != layerMaterialMap.end()) {
    ATH_MSG_VERBOSE(displayBuffer.str()
                    << "---[+] found material for Layer with Index: "
                    << lIndex.value());
    if (lay.surfaceRepresentation().isFree()) {
      ATH_MSG_VERBOSE(displayBuffer.str()
                      << "---[!] the Layer is not owned by the "
                         "TrackingGeometry, could indicate problem.");
    } else {
      ATH_MSG_VERBOSE(displayBuffer.str()
                      << "---[+] the Layer is owned by the TrackingGeometry.");
    }
    lay.assignMaterialProperties(*((*lmIter).second));
  } else {
    ATH_MSG_WARNING(displayBuffer.str()
                    << "---[!] could not find material for Layer with Index: "
                    << lIndex.value());
    return StatusCode::RECOVERABLE;
  }
  return StatusCode::SUCCESS;
}

// Processor Action to work on Surfaces
StatusCode Trk::LayerMaterialProvider::process(Trk::Surface&, size_t) const{
    return StatusCode::SUCCESS;
}


void
Trk::LayerMaterialProvider::dumpMaterialMap (const LayerMaterialMap& layerMaterialMap) const
{
    if (msgLvl (MSG::VERBOSE)) {
      static std::atomic_flag flag ATLAS_THREAD_SAFE = ATOMIC_FLAG_INIT;
      if (!flag.test_and_set()) {
        ATH_MSG_VERBOSE("Listing the layer indeces found in the loaded LayerMaterialMap");
        for ( const auto & lmIter : layerMaterialMap ){
          ATH_MSG_VERBOSE("  -> Found map for layer with index " << lmIter.first);
        }
      }
    }
}

