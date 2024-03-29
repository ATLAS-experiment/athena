/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeoShapeConverter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRGEOMODELCNV_GEOSHAPRECONVERTER_H
#define TRKDETDESCRGEOMODELCNV_GEOSHAPRECONVERTER_H
// Trk
#include "AthenaBaseComps/AthMessaging.h"
#include "TrkGeometry/MaterialProperties.h"
// Eigen
#include "GeoPrimitives/GeoPrimitives.h"

// STL

class GeoTubs;
class GeoTube;
class GeoPcon;
class GeoBox;
class GeoShape;

namespace Trk {

class CylinderVolumeBounds;
class CuboidVolumeBounds;
class Volume;

/**
  @class GeoShapeConverter

  A Simple Helper Class that convertes the GeoShape object
  used in GeoModel full detector description to an appropriate
  Trk::VolumeBounds (factory type) object.

  @author Andreas.Salzburger@cern.ch
  generalization by Sarka.Todorova@cern.ch
  */

class GeoShapeConverter : public AthMessaging {

   public:
    GeoShapeConverter();
    /** Convert a tubs */
    static std::unique_ptr<CylinderVolumeBounds> convert(const GeoTubs* gtub);

    /** Convert a tube */
    static std::unique_ptr<CylinderVolumeBounds> convert(const GeoTube* gtub);

    /** Convert a Polygon into a CylinderVolume -> smooth it*/
    static std::unique_ptr<CylinderVolumeBounds> convert(
        const GeoPcon* gtub, std::vector<double>& zbounds);

    /** Convert a Box */
    static std::unique_ptr<CuboidVolumeBounds> convert(const GeoBox* gbox);

    /** Convert an arbitrary GeoShape into Trk::Volume */
    std::unique_ptr<Volume> translateGeoShape(
        const GeoShape* shape, const Amg::Transform3D& trf) const;

    /** Decode and dump arbitrary GeoShape for visual inspection */
    void decodeShape(const GeoShape*) const;

   private:
};

}  // end of namespace Trk

#endif
