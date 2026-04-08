/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonMaterialDecoratorTool.h"

#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Geometry/DiamondVolumeBounds.hpp"
#include "ActsInterop/LoggerUtils.h"
#include "ActsGeoUtils/VolumePlacement.h"
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include <ActsPlugins/GeoModel/GeoModelMaterialConverter.hpp>
#include "GeoModelValidation/GeoMaterialHelper.h"



namespace MuonGMR4 {
    MuonMaterialDecoratorTool::~MuonMaterialDecoratorTool() = default;
    
    void MuonMaterialDecoratorTool::visitSurface(Acts::Surface& surface){
         m_matDecorator->decorate(surface);
    }

    StatusCode MuonMaterialDecoratorTool::initialize(){
        ActsPlugins::RootMaterialDecorator::Config decoratorConfig{};
        decoratorConfig.fileName = m_materialMapFile;
        m_matDecorator = std::make_unique<ActsPlugins::RootMaterialDecorator>(decoratorConfig,
                                                                             ActsTrk::actsLevelVector(msg().level()));                                                                           
        return StatusCode::SUCCESS;
    }

    StatusCode MuonMaterialDecoratorTool::finalize(){
        m_matDecorator.reset();
        return StatusCode::SUCCESS;
    }


}