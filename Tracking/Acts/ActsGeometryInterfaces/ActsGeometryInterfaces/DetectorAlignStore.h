/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_RawGeomAlignStore_H
#define ACTSGEOMETRYINTERFACES_RawGeomAlignStore_H

/// Put first the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"
///
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"
#include "CxxUtils/CachedUniquePtr.h"
/// The ActsFromGeoAlignStore is an adaptor to go from the GeoModel world caching the
/// rigid transformations of the detector elements to the Acts world where transformations

namespace ActsTrk {

    class DetectorAlignStore {
      public:
        
        /// @brief Copy constructor
        DetectorAlignStore(const DetectorAlignStore& other) = default;
        /// @brief Default constructor
        DetectorAlignStore(const DetectorType _type);
      
        /// @brief Default virtual destructor
        virtual ~DetectorAlignStore() = default;
        /// @brief Store containing the aligned GeoModel nodes
        std::shared_ptr<GeoAlignmentStore> geoModelAlignment{std::make_unique<GeoAlignmentStore>()};
        /// @brief Store holding the transfomations used by the Acts algorithms
        class TrackingAlignStore{
           public:
               TrackingAlignStore(const DetectorType detType);
               /** @brief Returns a unique ID to the client under which the client can store its transformation 
                *         inside the container.*/
               static unsigned int drawTicket(const DetectorType detType);
               /** @brief Returns the number of all distributed tickets */
               static unsigned int distributedTickets(const DetectorType detType);
               /** @brief */
               static void giveBackTicket(const DetectorType detType, unsigned int ticketNo);
               /** @brief Returns the transformation associated with the ticket number */
               const Amg::Transform3D* getTransform(unsigned int ticketNo) const {
                  return m_transforms[ticketNo].get();
               }
               /** @brief Caches for the given ticket number the transformation in the store and returns the 
                *         const reference to it */
               const Amg::Transform3D& setTransform(unsigned int ticketNo, Amg::Transform3D && trf) const {
                  return (*m_transforms.at(ticketNo).set(std::make_unique<Amg::Transform3D>(std::move(trf))));
               }
        
               static constexpr unsigned s_techs{static_cast<unsigned>(DetectorType::UnDefined)};
               using TicketCounterArr =  std::array<std::atomic<unsigned>, s_techs>;
               using ReturnedTicketArr = std::array<std::vector<unsigned>, s_techs>;
            private:
                static TicketCounterArr s_clientCounter ATLAS_THREAD_SAFE;
                static ReturnedTicketArr s_returnedTickets ATLAS_THREAD_SAFE;
                std::vector<CxxUtils::CachedUniquePtr<Amg::Transform3D>> m_transforms{};
        };
        /// @brief The aligned detector element type
        DetectorType detType{DetectorType::UnDefined};
        /// @brief Pointer to the store caching the final tracking transformations
        using TrackingAlignStorePtr = GeoModel::TransientConstSharedPtr<TrackingAlignStore>;
        TrackingAlignStorePtr trackingAlignment{std::make_unique<TrackingAlignStore>(detType)};        
        /// @brief The muon system contains additional parameters such as B-lines, as-built, passivation
        struct InternalAlignStore{};
        using InternalAlignPtr = GeoModel::TransientConstSharedPtr<InternalAlignStore>;
        InternalAlignPtr internalAlignment{};        
    };

   
}  // namespace ActsTrk

CLASS_DEF( ActsTrk::DetectorAlignStore , 167523695 , 1 );
CONDCONT_DEF( ActsTrk::DetectorAlignStore , 133556083 );
#endif
