/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_CALOBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRY_CALOBLUEPRINTNODEBUILDER_H

#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

using caloSampleSurfaceMap_t = std::map<CaloCell_ID::CaloSample, std::vector<std::shared_ptr<Acts::Surface> > >;
using caloSampleDDEElementsMap_t = std::map<CaloCell_ID::CaloSample, std::vector<const CaloDetDescrElement*> >;
using caloDimensionMap_t = std::map<std::string, double>;

namespace ActsTrk {

    /** @class CaloBlueprintNodeBuilder
     *  @brief Builds the Calo Blueprint Node
     */
    class CaloBlueprintNodeBuilder : public extends<AthAlgTool, IBlueprintNodeBuilder> {
    public:
        StatusCode initialize() override;
        StatusCode finalize() override;
        using base_class::base_class;

        /** @brief Build the Itk Blueprint Node
         *  @param gctx Geometry context
         *  @param child The child node which is added to the itk node.*/
        std::shared_ptr<Acts::Experimental::BlueprintNode> buildBlueprintNode(const Acts::GeometryContext& gctx,
                                      std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) override;

    private:

        /** fillMaps fills two maps.
        ** The first maps each calo sampling to a vector of surfaces (initially empty)
        ** The second maps each calo sampling to a vector of CaloDetDescrElements
        ** The second map is filled by looping over all DDE in the CaloDetDescrManager
        ** and adding each DDE to the vector corresponding to its sampling in the map
        */
        void fillMaps(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const;
        /** fillCaloDimensionsMap fills a map of calorimeter dimensions for each sampling layer.
        ** The map contains minR, maxR, minZ, maxZ and halfLengthZ for each sampling layer.
        */
        void fillCaloDimensionsMap(caloDimensionMap_t& caloDimensionsMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const;

        /** generateCylinderSurfaces generates cylindrical surfaces for each calo sampling.
        ** It does this for cylindrical layers by scanning in Z, for each Z finding the average radius of the cells in a phi ring
        ** If the average radius changes by more than a tolerance value (m_radiusTolerance), a new cylinder surface is created.
        ** The surfaces are added to the relevant vector of surfaces in the caloSampleSurfaceMap.    
        */
        void generateCylinderSurfaces(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const;

        /** generateCylinderSurface generates a cylindrical surface for a given set of parameters.
        ** To do this it calculates the radius and length of the cylinder, then shifts it in Z to the midpoint of the Z values used to build it.
        ** It then creates the Acts::CylinderSurface and returns it via a shared pointer.
        */
        std::shared_ptr<Acts::CylinderSurface> generateCylinderSurface(const double& maxLArBRadius, const double& minLArBRadius, const double& lowZLarB, const double& highZLarB) const;

        /** addCylindricalTrackingVolumeToCaloNode adds a cylindrical tracking volume to the calo node.
        ** It takes as input the container node, the calo dimensions map, the name of the volume and the vector of surfaces to be added to the volume.
        ** It creates a new CylinderContainerBlueprintNode in the container node, then creates a new Acts::TrackingVolume with the appropriate dimensions.
        ** Finally it adds the Acts::CylinderSurface to that Acts::TrackingVolume, then adds the tracking volume to the container node.
        */
        void addCylindricalTrackingVolumeToCaloNode(Acts::Experimental::CylinderContainerBlueprintNode& containerNode, caloDimensionMap_t& caloDimensionMap, const std::string& volumeName,const std::vector<std::shared_ptr<Acts::Surface>>& surfaces) const;

        std::unique_ptr<CaloDetDescrManager> m_caloDetSecrMgr;   

        //create lists from all possible calo samplings that tracks could hit
        //The first list is for disc shaped samples and the second for cylindrical shaped samples
        //Note that TileGap3 is the barrel, but is disk shaped. 
        std::vector<CaloCell_ID::CaloSample> m_caloDiscSampleList{CaloCell_ID::PreSamplerE, CaloCell_ID::EME1, CaloCell_ID::EME2, 
        CaloCell_ID::EME3, CaloCell_ID::HEC0, CaloCell_ID::HEC1, CaloCell_ID::HEC2, CaloCell_ID::HEC3, CaloCell_ID::TileGap3};

        std::vector<CaloCell_ID::CaloSample> m_caloCylinderSampleList{CaloCell_ID::PreSamplerB, CaloCell_ID::EMB1, CaloCell_ID::EMB2, 
        CaloCell_ID::EMB3,CaloCell_ID::TileBar0, CaloCell_ID::TileBar1, CaloCell_ID::TileBar2, 
        CaloCell_ID::TileGap1, CaloCell_ID::TileGap2, CaloCell_ID::TileExt0, CaloCell_ID::TileExt1, 
        CaloCell_ID::TileExt2};

        Gaudi::Property<double> m_radiusTolerance { this
        , "RadiusTolerance"
        , 2.0
        , "Tolerance for determining if a ring of cells in phi has changed the radius w.r.t to the previous ring in phi" };

        Gaudi::Property<double> m_zTolerance { this
        , "ZTolerance"
        , 2.0
        , "Tolerance for determining if a ring of cells in phi has changed the z w.r.t to the previous ring in phi" };

    };

}
#endif