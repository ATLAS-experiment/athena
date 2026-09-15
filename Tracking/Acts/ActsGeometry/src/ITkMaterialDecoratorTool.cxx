/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkMaterialDecoratorTool.h"

#include "ActsInterop/LoggerUtils.h"
#include "PathResolver/PathResolver.h"

namespace ActsTrk {
ITkMaterialDecoratorTool::~ITkMaterialDecoratorTool() = default;
void ITkMaterialDecoratorTool::visitSurface(Acts::Surface& surface) {
  m_matDecorator->decorate(surface);
}
StatusCode ITkMaterialDecoratorTool::initialize() {
  ActsPlugins::RootMaterialDecorator::Config decoratorConfig;

  std::string fullPath = PathResolverFindCalibFile(
      m_materialMapFolder.value() + "/" + m_materialMapFile.value());

  decoratorConfig.fileName = fullPath;
  ATH_MSG_INFO("Loading material map from " << fullPath);
  m_matDecorator = std::make_unique<ActsPlugins::RootMaterialDecorator>(
      decoratorConfig, ActsTrk::actsLevelVector(msg().level()));
  return StatusCode::SUCCESS;
}
StatusCode ITkMaterialDecoratorTool::finalize() {
  m_matDecorator.reset();
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
