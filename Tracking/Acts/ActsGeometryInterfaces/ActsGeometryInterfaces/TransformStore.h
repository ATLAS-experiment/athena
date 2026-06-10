/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRYINTERFACES_TrackingAlignStore_H
#define ACTSGEOMETRYINTERFACES_TrackingAlignStore_H

/// Put first the GeoPrimitives
#include "ActsGeometryInterfaces/GeometryDefs.h"

#include "CxxUtils/CachedUniquePtr.h"
#include "CxxUtils/checker_macros.h"

#include <array>
#include <variant>
#include <cassert>
#include <iostream>

namespace ActsTrk::detail{
    /** @brief The `TransformStore` provides a central place to outsource the alignable transforms
     *         of the sensitive detector elements and the alignable volumes. For each subdetector, 
     *         a cache of size N is provided (C.f. the @ref TrfStoreTicketCounter how the cache 
     *         size is managed). The transform store can be operated either in a LazyFilling mode
     *         where the memory is only assigned if the cache slot is actually needed. In this mode,
     *         the setTransform mehtod can be called on a const instance. Alternatively, the Store
     *         can be operated in Block mode, where tne the memory for N Transforms is allocated 
     *         at instantiation and can only be filled if a non-const reference is passed */
    class TransformStore {
        public:
            /** @brief Enum describing the operation mode of the cache */
            enum class Mode: std::uint8_t {
                LazyFill,   /// Transforms can be set later during the event processing
                Block       /// Transforms can only be set if the Store is created and 
                            /// non-const.
            };
            /** @brief Define the to string operator */
            static std::string toString(const Mode m);
            inline friend std::ostream& operator<<(std::ostream&ostr, const Mode m){
                return ostr<<toString(m);
            }
            /** @brief Define the to string ostream operator */
            inline friend std::ostream& operator<<(std::ostream&ostr, const TransformStore& store){
                return store.print(ostr);
            }
            /**  @brief Constructor
             *   @param detType: Sub detector type to retrieve the information about
             *                   how much memory needs to be allocated
             *   @param m: Storage mode  */
            TransformStore(const DetectorType detType, const Mode m);
            /** @brief Define the move constructor */
            TransformStore(TransformStore&& other) noexcept = default;
            /** @brief Define the move assignment operator */
            TransformStore& operator=(TransformStore&& other) noexcept = default;
            /** @brief Define the copy constructor */
            TransformStore(const TransformStore& other) noexcept;
            /** @brief Define the copy assignment operator */
            TransformStore& operator=(const TransformStore& other) noexcept;
            /** @brief Returns the pointer to the transfomation cached in the store
             *         (Might be a nullptr if LazyFill is used)
             *  @param ticketNo: Unique ticket number drawn by the client indicating
             *                   the index inside the storegae vector*/
            const Amg::Transform3D* getTransform(const unsigned ticketNo) const;
            /** @brief Stores a transform at the index of the passed ticket number.
             *         (Only works if the LazyFill mode option is activated)
             *  @param ticketNo: Number of the ticket under which the transform should be cached
             *  @param trf: Transform object to be stored 
             *  @returns The reference to the transform to which the passed transform was assigned to */
            const Amg::Transform3D& setTransform(const unsigned int ticketNo, Amg::Transform3D && trf) const;
            /** @brief Stores a transform at the index of the passed ticket number.
             *  @param ticketNo: Number of the ticket under which the transform should be cached
             *  @param trf: Transform object to be stored 
             *  @returns The reference to the transform to which the passed transform was assigned to */
            const Amg::Transform3D& setTransform(const unsigned int ticketNo, Amg::Transform3D && trf);
            /** @brief Returns the mode with which the store has been instantiated */
            Mode mode() const;
            /** @brief Returns the detector type */
            DetectorType detectorType() const;
            /** @brief Return the size (maximum capacity) of the current store */
            std::size_t size() const;
            /** @brief Returns the number of filled transforms */
            std::size_t filled() const;
        private:
            /** @brief Abrivation of the backend in the lazy filling mode */
            using LazyStorage_t = std::vector<CxxUtils::CachedUniquePtr<Amg::Transform3D>>;
            /** @brief Abrivate the transform vector */
            using TrfVec_t = std::vector<Amg::Transform3D>;
            /** @brief Abrivate the char vector */
            using CheckVec_t = std::vector<char>;
            /** @brief Abrivation of the continous vector storage */
            using BlockStorage_t = std::pair<TrfVec_t, CheckVec_t>;
            /** @brief Use the std::variant to dynamically switch between the two types */
            using Storage_t = std::variant<LazyStorage_t, BlockStorage_t>;
            Storage_t m_storage{};
            /** @brief The associated detector type  */
            DetectorType m_detType{ActsTrk::DetectorType::UnDefined};
            /** @brief Helper method to instantiate the storage for a given detector type 
             *  @param dType: Subdetector system for which the transform storage needs to
             *                be instantiated
             *  @param mode: The mode in which the storage shall be instantiated */
            static Storage_t initStorage(const DetectorType dType,
                                         const Mode mode) noexcept;
            
            std::ostream& print(std::ostream& ostr) const;

    };

    /** @brief In order that the `TransformStore` is able to provide enough memory to store
     *         the transform centrally, it needs to know how many clients are instantiated
     *         for a certain detector type. Each client which wants to use the TansformStore
     *         to outsource its transform caching needs to draw a ticket when it is instantiated.
     *         As the life time of certain instances may actually be short, the client is supposed
     *         to return its ticket to the counter when destructed. In this way, the cache size
     *         matches exactly the memory needs. */
    class TrfStoreTicketCounter {
        public:
            /** @brief Returns a unique ID to the client under which the client 
             *         can store its transfomrm inside the container.*/
            static unsigned int drawTicket(const DetectorType detType);
            /** @brief Returns the number of all distributed tickets */
            static unsigned int distributedTickets(const DetectorType detType);
            /** @brief Return back a ticket for the specified detector type such that
             *         its slot can be used by another instance
             *  @param detType: The ATLAS sub detector type for which the ticket is returned
             *  @param ticketNo: The number of the drawn ticket to return */
            static void giveBackTicket(const DetectorType detType, unsigned int ticketNo);


            static constexpr unsigned s_techs{static_cast<unsigned>(DetectorType::UnDefined)};
            using TicketCounterArr =  std::array<std::atomic<unsigned>, s_techs>;
            using ReturnedTicketArr = std::array<std::vector<char>, s_techs>;
            using ReturnedHintArr = std::array<int, s_techs>;

        private:
            static TicketCounterArr s_clientCounter ATLAS_THREAD_SAFE;
            static ReturnedTicketArr s_returnedTickets ATLAS_THREAD_SAFE;
            static ReturnedHintArr s_returnedHints ATLAS_THREAD_SAFE;
    };


    inline const Amg::Transform3D* TransformStore::getTransform(const unsigned ticketNo) const {
        return std::visit([&](auto& store) -> const Amg::Transform3D* {
            using Store_t = std::decay_t<decltype(store)>;
            if constexpr(std::is_same_v<Store_t, LazyStorage_t>) {
                assert(ticketNo < store.size());
                return store[ticketNo].get();
            } else if constexpr(std::is_same_v<Store_t, BlockStorage_t>) {
                assert(ticketNo < store.first.size());
                return store.second[ticketNo] ? &store.first[ticketNo] : nullptr;
            } else {
                return nullptr;
            }
        }, m_storage);
    }
}

#ifndef SIMULATIONBASE 
ACTS_OSTREAM_FORMATTER(ActsTrk::detail::TransformStore::Mode);
#endif
#endif