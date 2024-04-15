/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VolumeConverter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRGEOMODELCNV_VOLUMECONVERTER_H
#define TRKDETDESCRGEOMODELCNV_VOLUMECONVERTER_H

#include <cmath>    //for M_PI
#include <utility>  //for std::pair
#include <vector>

#include "AthenaBaseComps/AthMessaging.h"
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"
#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkDetDescrGeoModelCnv/VolumeIntersection.h"
#include "TrkGeometry/Material.h"  //used in typedef

class GeoVPhysVol;

namespace Trk {
class TrackingVolume;
class VolumeBounds;
}  // namespace Trk

namespace Trk {

using MaterialComponent = std::pair<Material, double>;

struct VolumeSpan {
    double phiMin{0.};
    double phiMax{2 * M_PI};
    double rMin{0.};
    double rMax{1.e5};
    double xMin{-1.e5};
    double xMax{1.e5};
    double yMin{-1.e5};
    double yMax{1.e5};
    double zMin{-1.e5};
    double zMax{1.e5};
};

struct VolumePart {
    std::vector<std::shared_ptr<Volume>> parts{};
    double sign{1.};
};
using VolumePartVec = std::vector<VolumePart>;

/**
  @class VolumeConverter

  A Simple Helper Class that collects methods for material simplification.

  @author sarka.todorova@cern.ch
  */

class VolumeConverter : public AthMessaging {

   public:
    VolumeConverter();
    /** translation of GeoVPhysVol to Trk::TrackingVolume */
    std::unique_ptr<TrackingVolume> translate(const GeoVPhysVol* gv,
                                              bool simplify, bool blend,
                                              double blendMassLimit) const;

    using VolumePair =
        std::pair<std::shared_ptr<Volume>, std::shared_ptr<Volume>>;
    using VolumePairVec = std::vector<VolumePair>;
    /** Decomposition of volume into set of non-overlapping subtractions from
     * analytically calculable volume */
    VolumePairVec splitComposedVolume(const Volume& trVol) const;

    double resolveBooleanVolume(const Volume& trVol, double tolerance) const;

    /** Estimation of the geometrical volume span */
    std::unique_ptr<VolumeSpan> findVolumeSpan(
        const VolumeBounds& volBounds, const Amg::Transform3D& transform,
        double zTol, double phiTol) const;

    /** Volume calculation : by default return analytical solution only */
    double calculateVolume(const Volume& vol, bool nonBooleanOnly = false,
                           double precision = 1.e-3) const;

    /**  the tricky part of volume calculation */
    double estimateFraction(const VolumePair& sub, double precision) const;

    /** material collection for layers */
    void collectMaterial(const GeoVPhysVol* pv, Trk::MaterialProperties& layMat,
                         double sf) const;

    /** material collection for volumes */
    void collectMaterialContent(
        const GeoVPhysVol* gv,
        std::vector<Trk::MaterialComponent>& materialContent) const;

   private:
    double leadingVolume(const GeoShape* sh) const;

    Trk::GeoShapeConverter m_geoShapeConverter;     //!< shape converter
    Trk::GeoMaterialConverter m_materialConverter;  //!< material converter
    VolumeIntersection m_intersectionHelper;        //!< overlaps

    static constexpr double s_precisionInX0 =
        1.e-3;  // tentative required precision of the material thickness
                // estimate
};

}  // end of namespace Trk

#endif
