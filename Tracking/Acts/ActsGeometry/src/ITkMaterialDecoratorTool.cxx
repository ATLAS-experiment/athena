/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkMaterialDecoratorTool.h"

#include "ActsInterop/LoggerUtils.h"
#include "PathResolver/PathResolver.h"

#include "ActsPlugins/Root/RootMaterialDecorator.hpp"
#include "ActsGeometry/MaterialMapJson.h"

namespace ActsTrk {
ITkMaterialDecoratorTool::~ITkMaterialDecoratorTool() = default;
void ITkMaterialDecoratorTool::visitSurface(Acts::Surface& surface) {
  if (m_rootMaterialDecorator) {
    m_rootMaterialDecorator->decorate(surface);
  } else {
    m_material.apply(surface);
  }
}
StatusCode ITkMaterialDecoratorTool::initialize() {
  std::string fullPath = PathResolverFindCalibFile(
      m_materialMapFolder.value() + "/" + m_materialMapFile.value());

  if (fullPath.empty()) {
    ATH_MSG_ERROR("Could not locate material map file '"
                 << m_materialMapFolder.value() << "/" << m_materialMapFile.value() << "'");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loading material map from " << fullPath);

  const Acts::Logging::Level level = ActsTrk::actsLevelVector(msg().level());

  // The Root reader only understands '.root' files; everything else
  // (in particular '.json' and '.cbor') is handled by the Json reader.
  if (fullPath.ends_with(".root")) {
    ActsPlugins::RootMaterialDecorator::Config decoratorConfig;
    decoratorConfig.fileName = std::move(fullPath);
    m_rootMaterialDecorator = std::make_unique<ActsPlugins::RootMaterialDecorator>(
        decoratorConfig, level);
  } else {
    m_material = readMaterialMapJson(fullPath, level);
  }
  return StatusCode::SUCCESS;
}
StatusCode ITkMaterialDecoratorTool::finalize() {
  m_rootMaterialDecorator.reset();
  m_material = {};
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
