/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "SimpleCylinderDetBuilderTool.h"

#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Detector/GeometryIdGenerator.hpp"
#include "Acts/Detector/DetectorVolume.hpp"
#include "Acts/Detector/PortalGenerators.hpp"
#include "Acts/Navigation/DetectorVolumeFinders.hpp"
#include "Acts/Navigation/InternalNavigation.hpp"
#include "Acts/Navigation/NavigationDelegates.hpp"
#include "Acts/Navigation/NavigationState.hpp"
#include "Acts/Navigation/DetectorVolumeFinders.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

namespace ActsTrk {

    SimpleCylinderDetBuilderTool::SimpleCylinderDetBuilderTool( const std::string& type, const std::string& name, const IInterface* parent ):
        AthAlgTool(type, name, parent){
            declareInterface<IDetectorVolumeBuilderTool>(this);
    }   

    Acts::Experimental::DetectorComponent SimpleCylinderDetBuilderTool::construct(const Acts::GeometryContext& context) const {
        const ActsGeometryContext* gctx = context.get<const ActsGeometryContext* >();

        auto cylinderBounds = std::make_unique<Acts::CylinderVolumeBounds>(0, m_radiusMax, m_outerZ);
        auto cylinderDetectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                    Acts::Experimental::defaultPortalGenerator(), gctx->context(), "EnvelopeSimple", 
                    Acts::Transform3::Identity(), std::move(cylinderBounds), Acts::Experimental::tryAllPortalsAndSurfaces());
        cylinderDetectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(1));

        if(msgLvl(MSG::VERBOSE)){
            Acts::ObjVisualization3D helper;
            Acts::GeometryView3D::drawDetectorVolume(helper, *cylinderDetectorVolume, gctx->context());
            helper.write("detectorSimple.obj");
            helper.clear();
        }       

        Acts::Experimental::DetectorComponent::PortalContainer portalContainer;
        for (auto [ip, p] : Acts::enumerate(cylinderDetectorVolume->portalPtrs())) {
            portalContainer[ip] = p;
        }

        return Acts::Experimental::DetectorComponent{
        {cylinderDetectorVolume},
        std::move(portalContainer),
        {{cylinderDetectorVolume}, Acts::Experimental::tryAllSubVolumes()}};
    }
}
