/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonMaterialDecoratorTool.h"

#include "ActsInterop/LoggerUtils.h"
#include "PathResolver/PathResolver.h"
#include "ActsGeometry/MaterialMapJson.h"



namespace MuonGMR4 {
    MuonMaterialDecoratorTool::~MuonMaterialDecoratorTool() = default;
    
    void MuonMaterialDecoratorTool::visitSurface(Acts::Surface& surface){
        bool hasMat = surface.hasMaterial(); 
        m_material.apply(surface);
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
        if (fullPath.ends_with(".root")) {
            ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" Root material maps are not supported any more ('"
                <<fullPath<<"'): provide a Json/Cbor map, optionally zstd compressed");
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO("Loading material map from " << fullPath);

        // Json or Cbor, optionally zstd compressed: detected from the file contents
        m_material = ActsTrk::readMaterialMapJson(fullPath, level);
        return StatusCode::SUCCESS;
    }

    StatusCode MuonMaterialDecoratorTool::finalize(){
        m_material = {};
        return StatusCode::SUCCESS;
    }
}
