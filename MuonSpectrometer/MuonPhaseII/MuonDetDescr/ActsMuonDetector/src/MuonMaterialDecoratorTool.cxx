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
#include "ActsPlugins/Root/RootMaterialDecorator.hpp"
#include "ActsPlugins/Json/JsonMaterialDecorator.hpp"
#include "ActsPlugins/Json/MaterialMapJsonConverter.hpp"



namespace MuonGMR4 {
    MuonMaterialDecoratorTool::~MuonMaterialDecoratorTool() = default;
    
    void MuonMaterialDecoratorTool::visitSurface(Acts::Surface& surface){
        bool hasMat = surface.hasMaterial(); 
        m_matDecorator->decorate(surface);
        if (hasMat != surface.hasMaterial()) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" Decorated "<<surface.geometryId()<<" with material");
        }
    }

    StatusCode MuonMaterialDecoratorTool::initialize(){
        const Acts::Logging::Level level = ActsTrk::actsLevelVector(msg().level());
        const std::string& fileName = m_materialMapFile.value();
        // Json (or Cbor) maps are read with the Json reader, everything else keeps the Root reader
        if (fileName.ends_with(".json") || fileName.ends_with(".cbor")) {
            Acts::MaterialMapJsonConverter::Config converterConfig;
            m_matDecorator = std::make_unique<Acts::JsonMaterialDecorator>(converterConfig, fileName, level);
        } else {
            ActsPlugins::RootMaterialDecorator::Config decoratorConfig;
            decoratorConfig.fileName = fileName;
            m_matDecorator = std::make_unique<ActsPlugins::RootMaterialDecorator>(decoratorConfig, level);
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonMaterialDecoratorTool::finalize(){
        m_matDecorator.reset();
        return StatusCode::SUCCESS;
    }
}
