/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ITKBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRY_ITKBLUEPRINTNODEBUILDER_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "ActsGeometry/ActsElementVector.h"
#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"

class BeamPipeDetectorManager;


namespace ActsTrk {

    /** Helper class to build the ItkBlueprint node 
     *  It adds the system as a node to the Blueprint.
     */
    class ItkBlueprintNodeBuilder : public extends<AthAlgTool, IBlueprintNodeBuilder> {

    public:

        StatusCode initialize() override;
        using base_class::base_class;
    
        /** @brief Build the Itk Blueprint Node
         *  @param gctx Geometry context
         *  @param child The child node which is added to the itk node.*/
        std::shared_ptr<Acts::Experimental::BlueprintNode> buildBlueprintNode(const Acts::GeometryContext& gctx,
                                      std::shared_ptr<Acts::Experimental::BlueprintNode>&& child) override;
                    
    
    private:

        const InDetDD::SiDetectorManager* m_itkPixelMgr{nullptr};
        
        const InDetDD::SiDetectorManager* m_itkStripMgr{nullptr};

        const BeamPipeDetectorManager* m_beamPipeMgr{nullptr};

        std::shared_ptr<ActsElementVector> m_elementStore{nullptr};

        Gaudi::Property<bool> m_doEndcapLayerMerging{this, "doEndcapLayerMerging", true}; // Flag to control endcap layer merging

        Gaudi::Property<bool> m_buildBeamPipe{this, "buildBeamPipe", true}; // Flag to control beam pipe building

        /** @brief Build the Itk Strip Blueprint Node
         * @param gctx Geometry context
         * @param node The node to add the strip node to */
        void buildItkStripBlueprintNode(const Acts::GeometryContext& gctx,
                                      Acts::Experimental::BlueprintNode& node);

        /** @brief Build the Itk Pixel Blueprint Node
         * @param gctx Geometry context
         * @param node The node to add the pixel node to */
        void buildItkPixelBlueprintNode(const Acts::GeometryContext& gctx,
                                      Acts::Experimental::BlueprintNode& node);
                                      
        /** @brief Build the Beam Pipe Blueprint Node
         * @param gctx Geometry context
         * @param node The node to add the beam pipe node to */
        void buildBeamPipeBlueprintNode(const Acts::GeometryContext& gctx,
                                        Acts::Experimental::BlueprintNode& node);
    };

}

#endif