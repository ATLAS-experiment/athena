/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#include "ActsVolumeIdToDetectorElementCollectionMappingAlg.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

// ATHENA
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "StoreGate/WriteCondHandle.h"

#include "ActsGeometry/ActsDetectorElement.h"
#include "Acts/Geometry/TrackingGeometry.hpp"

namespace ActsTrk {
ActsVolumeIdToDetectorElementCollectionMappingAlg::ActsVolumeIdToDetectorElementCollectionMappingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

ActsVolumeIdToDetectorElementCollectionMappingAlg::~ActsVolumeIdToDetectorElementCollectionMappingAlg() = default;

StatusCode ActsVolumeIdToDetectorElementCollectionMappingAlg::initialize() {
    ATH_CHECK(m_volumeIdToDetectorElementCollMapKey.initialize());
    ATH_CHECK(m_detEleCollKeys.initialize());
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode ActsVolumeIdToDetectorElementCollectionMappingAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<ActsTrk::ActsVolumeIdToDetectorElementCollectionMap>
       volumeIdTodetectorElementCollMap{m_volumeIdToDetectorElementCollMapKey, ctx};
    if (volumeIdTodetectorElementCollMap.isValid()) {
       return StatusCode::SUCCESS;
    }

    volumeIdTodetectorElementCollMap.addDependency (IOVInfiniteRange::infiniteTime());

    const Acts::TrackingGeometry *acts_tracking_geometry=m_trackingGeometryTool->trackingGeometry().get();
    ATH_CHECK( acts_tracking_geometry != nullptr);

    std::unique_ptr<ActsTrk::ActsVolumeIdToDetectorElementCollectionMap>
       volume_id_to_detector_element_collection_map = std::make_unique<ActsTrk::ActsVolumeIdToDetectorElementCollectionMap>();


    std::unordered_map<unsigned long long, unsigned int> detector_element_to_volume_id;
    createDetectorElementToVolumeIdMap(*acts_tracking_geometry,
                                       detector_element_to_volume_id);

    for (const SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> &det_ele_col_key : m_detEleCollKeys) {
       SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> det_ele_col(det_ele_col_key,ctx);
       ATH_CHECK(det_ele_col.isValid());
       volumeIdTodetectorElementCollMap.addDependency(det_ele_col);
       for (const InDetDD::SiDetectorElement *det_ele : *(det_ele_col.cptr())) {
          unsigned int vol_id = detector_element_to_volume_id.at(det_ele->identify().get_compact());
          volume_id_to_detector_element_collection_map->registerCollection(vol_id, det_ele_col.cptr());
       }
    }
    if (msgLvl(MSG::DEBUG)) {
       unsigned int vol_i=0;
       for (unsigned char col_i : volume_id_to_detector_element_collection_map->collecionMap()) {
          if (col_i>0) {
             ATH_MSG_DEBUG("Mapping " << vol_i << " -> " << static_cast<unsigned int>(col_i)
                           << " : " << static_cast<const void *>(volume_id_to_detector_element_collection_map->collection(vol_i)));
          }
          ++vol_i;
       }
    }

    ATH_CHECK( volumeIdTodetectorElementCollMap.record( std::move(volume_id_to_detector_element_collection_map) ) );

    return StatusCode::SUCCESS;
}

void
ActsVolumeIdToDetectorElementCollectionMappingAlg::createDetectorElementToVolumeIdMap(const Acts::TrackingGeometry &acts_tracking_geometry,
                                                                                      std::unordered_map<unsigned long long,
                                                                                                         unsigned int> &detector_element_to_volume_id)
const
{
   using Counter = struct { unsigned int n_detector_elements, n_missing_detector_elements, n_wrong_type; };
   Counter counter {0u,0u,0u};
   acts_tracking_geometry.visitSurfaces([&counter, &detector_element_to_volume_id](const Acts::Surface *surface_ptr) {
      if (!surface_ptr) return;
      const Acts::Surface &surface = *surface_ptr;
      const Acts::DetectorElementBase*detector_element = surface.associatedDetectorElement();
      if (detector_element) {
         const ActsDetectorElement *acts_detector_element = dynamic_cast<const ActsDetectorElement*>(detector_element);
         if (acts_detector_element) {
            const auto*trk_detector_element  = dynamic_cast<const Trk::TrkDetElementBase*>(acts_detector_element->upstreamDetectorElement());
            if(trk_detector_element  != nullptr) {
               detector_element_to_volume_id.insert( std::make_pair( trk_detector_element->identify().get_compact(), surface.geometryId().volume()));
            }
            else {
               ++counter.n_wrong_type;
            }
         }
         else {
            ++counter.n_wrong_type;
         }
         ++counter.n_detector_elements;
      }
      else {
         ++counter.n_missing_detector_elements;
      }
   }, true /*sensitive surfaces*/);

   ATH_MSG_DEBUG( "Surfaces without associated detector elements " << counter.n_missing_detector_elements
                  << " (with " << counter.n_detector_elements << ")" );
   if (counter.n_detector_elements==0) {
      ATH_MSG_ERROR( "No surface with associated detector element" );
   }
   if (counter.n_wrong_type>0) {
      ATH_MSG_WARNING( "Surfaces associated to detector elements not of type Trk::TrkDetElementBase :" << counter.n_wrong_type);
   }
}

}
