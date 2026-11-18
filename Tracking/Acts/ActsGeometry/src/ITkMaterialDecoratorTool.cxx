/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkMaterialDecoratorTool.h"

#include "ActsInterop/LoggerUtils.h"
#include "PathResolver/PathResolver.h"

#include "ActsGeometry/MaterialMapJson.h"

namespace ActsTrk {
ITkMaterialDecoratorTool::~ITkMaterialDecoratorTool() = default;
void ITkMaterialDecoratorTool::visitSurface(Acts::Surface& surface) {
  m_material.apply(surface);
}
StatusCode ITkMaterialDecoratorTool::initialize() {
  std::string fullPath = PathResolverFindCalibFile(
      m_materialMapFolder.value() + "/" + m_materialMapFile.value());

  if (fullPath.empty()) {
    ATH_MSG_ERROR("Could not locate material map file '"
                 << m_materialMapFolder.value() << "/" << m_materialMapFile.value() << "'");
    return StatusCode::FAILURE;
  }
  if (fullPath.ends_with(".root")) {
    ATH_MSG_ERROR("Root material maps are not supported any more ('" << fullPath
                  << "'): provide a Json/Cbor map, optionally zstd compressed");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loading material map from " << fullPath);

  const Acts::Logging::Level level = ActsTrk::actsLevelVector(msg().level());

  // Json or Cbor, optionally zstd compressed: detected from the file contents
  m_material = readMaterialMapJson(fullPath, level);
  return StatusCode::SUCCESS;
}
StatusCode ITkMaterialDecoratorTool::finalize() {
  m_material = {};
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
