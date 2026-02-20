/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELUTILITIES_GEOALIGNMENTSTORE_H
#define GEOMODELUTILITIES_GEOALIGNMENTSTORE_H

/// Ensure that the extensions for the Vector3D are properly loaded
#include <stdexcept>
/// Ensure that the extensions for the Vector3D are properly loaded
#include "GeoPrimitives/GeoPrimitives.h"
///
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoVAlignmentStore.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoAlignableTransform.h"

#include "GeoModelUtilities/TransformMap.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

class GeoAlignmentStore : public GeoVAlignmentStore {
public:
    /// @brief Default constructor
    GeoAlignmentStore() = default;
    /// @brief  Default destructor
    virtual ~GeoAlignmentStore() override = default;

    /// @brief: Assign the alignment delta transform with a alignable GeoModel node
    /// @param alignableNode: Transform node in the GeoModel tree to align
    /// @param transform: Distortion of the node and the subsequent subtree
    virtual void setDelta(const GeoAlignableTransform* alignableNode, 
                          const GeoTrf::Transform3D& transform) override;

    /// @brief: Assign the alignment delta transform with a alignable GeoModel node
    /// @param alignableNode: Transform node in the GeoModel tree to align
    /// @param transform: Distortion of the node and the subsequent subtree
    void setDelta(const GeoAlignableTransform* alignableNode, 
                  std::shared_ptr<const GeoTrf::Transform3D> trans);

    /// @brief: Retrieve the alignment distortion associated with the alignable node. Returns a nullptr
    ///         if no distorion has been set before
    /// @param alignableNode: Pointer to the alignable node in the GeoModel tree
    virtual const GeoTrf::Transform3D* getDelta(const GeoAlignableTransform* alignableNode) const override final;

    /// @brief: Caches the absolute transform including the alignment distortions that's associated with the 
    ///         full physical volume.
    /// @param fullPhysVol: Full physical volume in the GeoModel tree to which the transforms belongs to
    /// @param physVolTrf:  Aligned placement of the physical volume in space
    virtual void setAbsPosition(const GeoNodePositioning* fullPhysVol, 
                                const GeoTrf::Transform3D& physVolTrf) override final;
    /// @brief: Returns the aligned transform associated with the full physical volume. Returns a 
    ///         nullptr if the physical volume has not been added before
    virtual const GeoTrf::Transform3D* getAbsPosition(const GeoNodePositioning* fullPhysVol) const override final;
    /// @brief: Caches the aboslute transform of the perfectly aligned physical volume.
    /// @param fullPhysVol: Full physical volume in the GeoModel tree to which the transforms belongs to
    /// @param unAlignedTrf: Nominal placement of the full physical volume.
    virtual void setDefAbsPosition(const GeoNodePositioning* fullPhysVol, 
                                  const GeoTrf::Transform3D& unAlignedTrf) override final;
    /// @brief: Returns the nominal position of the full phyiscal volume. Returns a nullptr if the
    ///         nominal position of the full physical volume has not been added before to the map
    virtual const GeoTrf::Transform3D* getDefAbsPosition(const GeoNodePositioning* fullPhysVol) const override final;

    /// @brief: Copies, the deltas, the absolute and the nominal positions of the other map
    ///         to this object. Returns false if the two maps partially overlap.
    bool append(const GeoAlignmentStore& other);
    /// @brief: Clears the position cache
    void clearPosCache();
    /// @brief: Locks the delta transform cache
    void lockDelta();
    /// @brief: Locks the position cache
    void lockPosCache();
    /** @brief Returns whether the position cache is locked */
    bool posCacheLocked() const;
    /** @brief Returns whether the delta transform cache is locked */
    bool deltaCacheLocked() const;
    
    using DeltaMap = TransformMap<GeoAlignableTransform, GeoTrf::Transform3D>;
    using DeltaMapPtr = GeoModel::TransientConstSharedPtr<DeltaMap>;

    using PositioningMap = TransformMap<GeoNodePositioning, GeoTrf::Transform3D>;
    using PositioningMapPtr = GeoModel::TransientConstSharedPtr<PositioningMap>;
private:    
    DeltaMapPtr m_deltas{std::make_unique<DeltaMap>()};
    PositioningMapPtr m_absPositions{std::make_unique<PositioningMap>()};
    PositioningMapPtr m_defAbsPositions{std::make_unique<PositioningMap>()};

public:
    DeltaMapPtr getDeltas() const;
    PositioningMapPtr getAbsPositions() const;
    PositioningMapPtr getDefAbsPositions() const;
};

inline const GeoTrf::Transform3D* GeoAlignmentStore::getDelta(const GeoAlignableTransform* axf) const { 
    return m_deltas->getTransform(axf); 
}
inline const GeoTrf::Transform3D* GeoAlignmentStore::getAbsPosition(const GeoNodePositioning* fpv) const {
    return m_absPositions->getTransform(fpv); 
}
inline const GeoTrf::Transform3D* GeoAlignmentStore::getDefAbsPosition(const GeoNodePositioning* fpv) const {
    return m_defAbsPositions->getTransform(fpv);
}

CLASS_DEF(GeoAlignmentStore, 135648236, 1)
CONDCONT_DEF(GeoAlignmentStore, 33985726);

#endif
