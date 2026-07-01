/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_RawGeomAlignStore_H
#define ACTSGEOMETRYINTERFACES_RawGeomAlignStore_H

/// Put first the GeoPrimitives
#include "ActsGeometryInterfaces/TransformStore.h"
///
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"
#include "CxxUtils/CachedUniquePtr.h"

/** @brief The `DetectorAlignStore` is a cache class to hold the aligned surface local->global transforms
 *         and the both volume transforms that are associated with an ATLAS sub detector technology. The
 *         cache is written such that it can be lazily populated or fully populated at its creation.
 * 
 *         The cache is structured in three components:
 *             1) geoModelAlignment: This store contains all information to apply rigid alignment
 *                                   corrections to the ReadoutElement. All readout elements are assoicated
 *                                   with a GeoVPhysVol objects which are positioned in space via a series
 *                                   of `GeoTransforms` and `GeoAlignableTransforms`. The latter represent the
 *                                   alignment fix points from which the alignment corrections are applied. GeoModel
 *                                   then aligns all subvolumes according to the deltas
 *             2) trackingAlignment: GeoModel usally does not keep the fully assembled transforms in 
 *                                   memory. The actual caching of them is taken over by the trackingAlignment
 *                                   Clients which want to push a transform onto the cache need to draw
 *                                   a unique ticket from the store at construction. This ticket is valid for
 *                                   a given sub detector and can be used to query the cache whether a transform
 *                                   has been already pushed or to ask for the transform itself
 *            3) internalAlignment: Is an empty sub class which is meant to store conditions data to correct
 *                                  the readout geometry for surface deformations (E.g. muon b-lines, as-built) 
 * */
namespace ActsTrk {

    class DetectorAlignStore {
      public:
        using Mode = detail::TransformStore::Mode;
        /** @brief Copy constructor  */
        DetectorAlignStore(const DetectorAlignStore& other) = default;
        /** @brief Default constructor  */
        explicit DetectorAlignStore(const DetectorType type, const Mode mode): 
               detType{type}, 
               trackingAlignment{std::make_unique<detail::TransformStore>(detType, mode)} {}
      
        /** @brief Default virtual destructor */
        virtual ~DetectorAlignStore() = default;
        /** @brief Store containing the aligned GeoModel nodes  */
        std::shared_ptr<GeoAlignmentStore> geoModelAlignment{std::make_shared<GeoAlignmentStore>()};
        /** @brief The aligned detector element type  */
        DetectorType detType{DetectorType::UnDefined};
        /** @brief Pointer to the store caching the final tracking transformations  */
        using TrackingAlignStorePtr = GeoModel::TransientConstSharedPtr<detail::TransformStore>;
        TrackingAlignStorePtr trackingAlignment{};        
        /** @brief The muon system contains additional parameters such as B-lines, as-built, passivation */
        struct InternalAlignStore{};
        using InternalAlignPtr = GeoModel::TransientConstSharedPtr<InternalAlignStore>;
        InternalAlignPtr internalAlignment{};        
    };

   
}  // namespace ActsTrk

CLASS_DEF( ActsTrk::DetectorAlignStore , 167523695 , 1 );
CONDCONT_DEF( ActsTrk::DetectorAlignStore , 133556083 );
#endif
