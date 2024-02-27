/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_IMUONGEOUTILITYTOOL_H
#define MUONGEOMODELR4_IMUONGEOUTILITYTOOL_H


#include "GeoPrimitives/GeoPrimitives.h"
/// Load the Eigen definitions.

#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoShape.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>
#include <GeoModelKernel/GeoAlignableTransform.h>
#include <GaudiKernel/IAlgTool.h>

class GeoShapeUnion;

namespace MuonGMR4{

class IMuonGeoUtilityTool : virtual public IAlgTool {
    public:
         /// Gaudi interface ID
        DeclareInterfaceID(IMuonGeoUtilityTool, 1, 0);

        /// Returns the next Geo alignable transform in the GeoModelTree upstream
        virtual const GeoAlignableTransform* findAlignableTransform(const PVConstLink& physVol) const = 0;
                                                             
        
        /// Navigates throughs the volume to find a Box / Prd shape
        virtual const GeoShape* extractShape(const PVConstLink& physVol) const = 0;
        virtual const GeoShape* extractShape(const GeoShape* inShape) const = 0;
    
        // Navigates through the volume to find the shifts / rotations etc. from the geo shape
        virtual Amg::Transform3D extractShifts(const PVConstLink& physVol) const = 0;
        virtual Amg::Transform3D extractShifts(const GeoShape* inShape) const = 0;

        /// Helper struct to cache a PhysVolume pointer together with the transformation to go from the volume to
        /// the given parent node in the tree
        struct physVolWithTrans{
            Amg::Transform3D transform{Amg::Transform3D::Identity()};
            PVConstLink physVol{nullptr};
        };
        /// Searches through all child volumes and collects the nodes where the logical volumes have the requested name
        /// together with the transformations to go from the node to the parent physical volume
        virtual std::vector<physVolWithTrans> findAllLeafNodesByName(const PVConstLink& physVol, const std::string& volumeName) const = 0;

        
        /// @brief Helper struct to cache a volume with its ShapeShift transformation
        struct geoShapeWithShift {
            Amg::Transform3D transform{Amg::Transform3D::Identity()};
            const GeoShape* shape{nullptr};
        };
        /// Splits a union into its building blocks. If one of the objects is an Union, then this is split again
        virtual std::vector<geoShapeWithShift> getComponents(const GeoShapeUnion* unionShape) const = 0;

        ///     
        virtual std::string dumpShape(const GeoShape* inShape) const = 0;
        ///
        virtual std::string dumpVolume(const PVConstLink& physVol) const = 0;
        /// Transforms the vertices of the Polygon shape into a std::vector consisting of Amg::Vector2D objects
        virtual std::vector<Amg::Vector2D> polygonEdges(const GeoSimplePolygonBrep& polygon) const = 0;

 
};

}
#endif