/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_CALOBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRY_CALOBLUEPRINTNODEBUILDER_H

#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Utilities/Helpers.hpp"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

#include <array>
#include <memory>
#include <span>


namespace ActsTrk {


    /** @class CaloBlueprintNodeBuilder
     *  @brief Builds the Calo Blueprint Node
     */
    class CaloBlueprintNodeBuilder : public extends<AthAlgTool, IBlueprintNodeBuilder> {
    public:
        
        enum class caloRegion {DiscNegativeZ, 
                               DiscPositiveZ, 
                               BarrelCylinder, 
                               nRegions};
        friend std::ostream& operator<<(std::ostream& ostr, const caloRegion region) {
            using enum caloRegion;
            switch(region) {
              case DiscNegativeZ: ostr<<"DiscNegativeZ"; break;
              case DiscPositiveZ: ostr<<"DiscPositiveZ"; break;
              case BarrelCylinder: ostr<<"BarrelCylinder"; break;
              case nRegions: ostr<<"nRegions"; break;
            }
            return ostr;
        }
        
        using DetElVec_t = std::vector<const CaloDetDescrElement*>;
        /** @brief Vector of Detector Elements per calorimeter layer */
        using DetElementMap_t = std::array<DetElVec_t, CaloSampling::getNumberOfSamplings()>;
        
        /** @brief Vector of surfaces per calorimeter region */
        using SurfaceMap_t = std::array<std::vector<std::shared_ptr<Acts::Surface>>,
                                              Acts::toUnderlying(caloRegion::nRegions)>;

        using base_class::base_class;
        /** @copydoc IBlueprintNodeBuilder::buildBlueprintNode */
        std::shared_ptr<Acts::BlueprintNode> buildBlueprintNode(const Acts::GeometryContext& gctx,
                                                                std::shared_ptr<Acts::BlueprintNode>&& childNode) override;
        /** @copydoc AthAlgTool::initialize */
        virtual StatusCode initialize() override final;

    private:
        /** @brief Fill the detector elements per calorimeter layer and sort them in 
         *         ascending r(z) for the  barrel (endcap) layers. Remove duplicates from
         *         the list where r and z are within the precision tolerance
         *  @param detDescrMgr: The calorimeter detector description manager from which
         *                       the detector elements are retrieved */
        DetElementMap_t fillDetectorElements(const CaloDetDescrManager& detDecrMgr) const;
        /** @brief Translate all detector elements to a Cylinder and Disc surfaces. Per calorimeter
         *         layer the detector elements are combined to single or multiple surfaces and then
         *         sorted into the 
         *  @param detElements: The calorimeter detector description elements split by their
         *                      logical CaloSamplingID */
        SurfaceMap_t translateToSurfaces(const DetElementMap_t& detElements) const;

        /** @brief Create a cylinder surface from a list of Calo detector description elements. The surface is
          *        constructed such that the radius is the radial midpoint of the passed elements, the thickness
          *        is capturing the minimal and maximal passed radius and the half length completley encloses 
          *        the passed description elements.
          * @param detElements: The list of detector elements from which the envelope surface shall be translated. */
        std::shared_ptr<Acts::Surface> createCylinderSurface(std::span<const CaloDetDescrElement* const> detElements) const;

        /** @brief Create a disc surface from the passed calo description elements. The disc is placed at the central z 
         *         considering the longitudinal spread of the elements. The radius of the disc fully encapsulates the 
         *         detector description elements and the assigned thickness represents the maximum and minimum z
         * @param detElements: The list of detector elements from which the envelope surface shall be translated. */
        std::shared_ptr<Acts::Surface> createDiscSurface(std::span<const CaloDetDescrElement* const> detElements) const;
        /** @brief Create the envelope volume of the Calo Tracking Geometry. The volume encapsulates 
          *        all calorimeter surfaces taking the respective thickness into account. The inner radius
          *        is set to zero to make space for the ITk. The GeometryIdentifier is set to the central
          *        calo volume ID
          * @param tgContext: The geometry context to align the particular surfaces and volumes
          * @param surfacesPerRegion: List of the constructed calorimeter surfaces */
        std::unique_ptr<Acts::TrackingVolume> envelopeVolume(const Acts::GeometryContext& tgContext,
                                                             const SurfaceMap_t& surfacesPerRegion) const;
        /** @brief Fill the layer with extra surfaces. Via the <insertExtraSuraces> property,
         *         it can be steered how many extra surfaces are inserted between the central surface
         *         and the virtual boundaries (defined by the extra thickness). E.g. a value of one
         *         inserts one surface in front and another one behind the central surface
         *  @param tgContext: The geometry context to access the surface transforms
         *  @param centralSurf: The central surface to be duplicated */
        std::vector<std::shared_ptr<Acts::Surface>> fillLayer(const Acts::GeometryContext& tgContext,
                                                              std::shared_ptr<Acts::Surface>&& centralSurf) const;
        
        /** @brief Configure the insert of extra surfaces per each calorimeter layer  */
        Gaudi::Property<std::vector<std::uint32_t>> m_surfaceDuplicates{this, "insertExtraSurfaces",
            std::vector<std::uint32_t>(CaloSampling::Unknown, 0) };
                              
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
ACTS_OSTREAM_FORMATTER(ActsTrk::CaloBlueprintNodeBuilder::caloRegion);
#endif
