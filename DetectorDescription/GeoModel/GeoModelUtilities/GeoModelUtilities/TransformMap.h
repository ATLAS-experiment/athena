/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELUTILITIES_TRANSFORMMAP_H
#define GEOMODELUTILITIES_TRANSFORMMAP_H

#include <CxxUtils/ConcurrentToValMap.h>
#include <CxxUtils/SimpleUpdater.h>

#include <memory>
#include <unordered_map>
#include <shared_mutex>

/**
 *  Generic class to outsource the cache of  transformations, Vectors, etc, from the 
 *  core class instance. The pointer to the class instance serves as access key to the map.
 *  The cache can be filled on-the fly and hence does not require an initialization step.  
 *    
*/
template <typename T, typename X> class TransformMap {
public:
    /** @brief: Default constructor*/
    TransformMap() =default;
    /*** @brief: Copy constructor from an existing map*/
    TransformMap(const TransformMap& other) = default;
    /** @brief: Copy assignment operator*/
    TransformMap& operator=(const TransformMap& other) = default;

    ~TransformMap() = default;
    /** @brief: Adds a transform object to the map. Returns false if a transform has been already
     *          associated before with the object. The object is turned into a shared pointer to allow
     *          sharing of the same memory in cases where multiple TransformMaps are around in memory and the
     *          transform remains constant across the maps.
     * @param obj: Pointer to the class instance to which the transform shall be associated.
     * @param xf:  Transform object to cache
    */
    bool setTransform(const T *obj, const X &xf) const;
    bool setTransform(const T* obj, const X& xf);
    /** @brief: Adds a transform object to the map. Returns false if a transform has been already
     *          associated before with the object
     * @param obj: Pointer to the class instance to which the transform shall be associated.
     * @param xf:  Shared pointer of the transform object to cache
    */
    bool setTransform(const T *obj, std::shared_ptr<const X> xf) const;
    bool setTransform(const T* obj, std::shared_ptr<const X> xf);    
    
    /** @brief: Returns the transform that's associated with input object. If the object has not been
     *          added via the setTransform method, a nullptr is returned
     *  @param obj: Pointer to the class instance which associated transform shall be fetched
    */
    const X *getTransform(const T *obj) const;

    /** @brief: Copies all key value pairs from the external map and appends it to this map instance. Returns false
     *          if the second map partially overlaps with this map.
     *  @param other: Other map in memory to copy from
    */
    bool append(const TransformMap &other);
    /** @brief: Clear the transform cache map*/
    void clear();
    /** @brief: Lock the map which switches off the mutex protection mechanism. 
     *          Any attempt to const modify the container afterwards leads to exceptions. */
    void lock();
    /** @brief: Returns a vector of all objects that have been parsed to the map */
    std::vector<const T*> getStoredKeys() const;

private:
    using ConCurrentMap_t = CxxUtils::ConcurrentToValMap<const T*, std::shared_ptr<const X>, CxxUtils::SimpleUpdater>;
    using ConCurrentMap_ptr = std::unique_ptr<ConCurrentMap_t>;
    using CanonicalMap_t = std::unordered_map<const T*, std::shared_ptr<const X>>;
    using CanonicalMap_ptr = std::unique_ptr<CanonicalMap_t>;
    

    mutable CanonicalMap_t m_container ATLAS_THREAD_SAFE{};
    mutable bool m_locked ATLAS_THREAD_SAFE{false};
    mutable std::shared_mutex m_mutex ATLAS_THREAD_SAFE{};
    mutable ConCurrentMap_ptr m_mutableCont ATLAS_THREAD_SAFE{
                                std::make_unique<ConCurrentMap_t>(typename ConCurrentMap_t::Updater_t())};


};
#include <GeoModelUtilities/TransformMap.icc>
#endif