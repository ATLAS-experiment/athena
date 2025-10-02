/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include <GeoModelHelpers/getChildNodesWithTrf.h>
#include <GaudiKernel/IAlgTool.h>

class GeoShapeUnion;

namespace MuonGMR4{

class IMuonGeoUtilityTool : virtual public IAlgTool {
    public:
         /// Gaudi interface ID
        DeclareInterfaceID(IMuonGeoUtilityTool, 1, 0);

        /** @brief Returns the first alignable transform in the root subtree upstream the volume
         *         the volume must not be shared, otherwise exception is thrown
         *  @param physVol: Reference to the GeoVPhysVol to find the associated alignable trf */
        virtual const GeoAlignableTransform* findAlignableTransform(const PVConstLink& physVol) const = 0;
        /** @brief Navigates through the volume shape hiearchy to return the first shape 
         *         which is neither a shift or in case of subtractions, the overall envelope shape
         *  @param physVol: Reference to the GeoVPhysVol to extract the shape from */
        virtual const GeoShape* extractShape(const PVConstLink& physVol) const = 0;
        /** @brief Navigates through the shape hiarchy to return the first shape which is neither a shift
         *         or in case of a subtraction, the overall envelope shape
         *  @param inShape: Pointer to the shape to navigate through */
        virtual const GeoShape* extractShape(const GeoShape* inShape) const = 0;

        /// Helper struct to cache a PhysVolume pointer together with the transformation to go from the volume to
        /// the given parent node in the tree
        using physVolWithTrans = GeoChildNodeWithTrf;
        /// Searches through all child volumes and collects the nodes where the logical volumes have the requested name
        /// together with the transformations to go from the node to the parent physical volume
        virtual std::vector<physVolWithTrans> findAllLeafNodesByName(const PVConstLink& physVol, const std::string& volumeName) const = 0;
        /// Splits a boolean shape into its building blocks
        virtual std::vector<const GeoShape*> getComponents(const GeoShape* booleanShape) const = 0;

        ///     
        virtual std::string dumpShape(const GeoShape* inShape) const = 0;
        ///
        virtual std::string dumpVolume(const PVConstLink& physVol) const = 0;
        /// Transforms the vertices of the Polygon shape into a std::vector consisting of Amg::Vector2D objects
        virtual std::vector<Amg::Vector2D> polygonEdges(const GeoSimplePolygonBrep& polygon) const = 0;

        /// Returns the edge points of the polygon like GeoShapes
        virtual std::vector<Amg::Vector3D> shapeEdges(const GeoShape* shape,
                                                      const Amg::Transform3D& volTrf) const = 0;

 
};

}
#endif