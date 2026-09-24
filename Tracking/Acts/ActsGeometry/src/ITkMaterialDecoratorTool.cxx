/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkMaterialDecoratorTool.h"

#include "ActsInterop/LoggerUtils.h"
#include "PathResolver/PathResolver.h"

#include "ActsPlugins/Root/RootMaterialDecorator.hpp"
#include "ActsPlugins/Json/JsonMaterialDecorator.hpp"

namespace ActsTrk {
ITkMaterialDecoratorTool::~ITkMaterialDecoratorTool() = default;
void ITkMaterialDecoratorTool::visitSurface(Acts::Surface& surface) {
  m_matDecorator->decorate(surface);
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
    decoratorConfig.fileName = fullPath;
    m_matDecorator = std::make_unique<ActsPlugins::RootMaterialDecorator>(
        decoratorConfig, level);
  } else {
    Acts::MaterialMapJsonConverter::Config converterConfig;
    m_matDecorator = std::make_unique<Acts::JsonMaterialDecorator>(
        converterConfig, fullPath, level);
  }
  return StatusCode::SUCCESS;
}
StatusCode ITkMaterialDecoratorTool::finalize() {
  m_matDecorator.reset();
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
