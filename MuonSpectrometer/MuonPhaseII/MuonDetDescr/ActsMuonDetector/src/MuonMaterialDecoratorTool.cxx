/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonMaterialDecoratorTool.h"

#include "ActsInterop/LoggerUtils.h"
#include "PathResolver/PathResolver.h"
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
        std::string fullPath = std::format("{:}/{:}",  m_materialMapFolder.value() , m_materialMapFile.value());
        if (fullPath.size() && fullPath[0] != '/') fullPath = PathResolverFindCalibFile(fullPath);

        if(fullPath.empty()){
            ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" Could not find material map file: "
                <<m_materialMapFolder.value() + "/" + m_materialMapFile.value());
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO("Loading material map from " << fullPath);

        // Json (or Cbor) maps are read with the Json reader, everything else keeps the Root reader
        if (fullPath.ends_with(".json") || fullPath.ends_with(".cbor")) {
            Acts::MaterialMapJsonConverter::Config converterConfig;
            m_matDecorator = std::make_unique<Acts::JsonMaterialDecorator>(converterConfig, fullPath, level);
        } else {
            ActsPlugins::RootMaterialDecorator::Config decoratorConfig;
            decoratorConfig.fileName = fullPath;
            m_matDecorator = std::make_unique<ActsPlugins::RootMaterialDecorator>(decoratorConfig, level);
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonMaterialDecoratorTool::finalize(){
        m_matDecorator.reset();
        return StatusCode::SUCCESS;
    }
}
