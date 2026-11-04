/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsGeometry/MaterialMapJson.h"

#include "Acts/Material/TrackingGeometryMaterial.hpp"
#include "Acts/Utilities/Diagnostics.hpp"
#include "ActsPlugins/Json/MaterialMapJsonConverter.hpp"
#include "ActsPlugins/Json/TrackingGeometryMaterialJsonConverter.hpp"
#include "ActsPlugins/Json/detail/JsonIo.hpp"

#include <stdexcept>

namespace {
// Transitional legacy material-map compatibility, deliberately private to this
// loader. TODO: Remove this helper and its dispatch branch once the default
// material maps are migrated and the versioned codec supports volume maps.
Acts::TrackingGeometryMaterial readLegacyMaterialMapForMigration(
    const nlohmann::json& document, const std::filesystem::path& path,
    Acts::Logging::Level level) {
  if (!document.contains("Surfaces") || !document.contains("Volumes")) {
    throw std::invalid_argument(
        "Legacy material maps require both Surfaces and Volumes tables");
  }
  ACTS_PUSH_IGNORE_DEPRECATED()
  Acts::MaterialMapJsonConverter converter({}, level);
  auto material = converter.jsonToMaterialMaps(document);
  ACTS_POP_IGNORE_DEPRECATED()
  const auto logger = Acts::getDefaultLogger("MaterialMapJson", level);
  logger->log(Acts::Logging::WARNING,
              "Reading deprecated legacy material map '" + path.string() +
                  "'. This compatibility path is temporary. " +
                  (material.volumeMaterials.empty()
                       ? "Migrate it with ActsMaterialMapMigrate to .json.zst."
                       : "Volume maps require legacy support until the "
                         "versioned material codec supports them."));
  return material;
}

}  // namespace

Acts::TrackingGeometryMaterial ActsTrk::readMaterialMapJson(
    const std::filesystem::path& path, Acts::Logging::Level level) {
  // Temporary use of ACTS detail I/O to inspect the document before v0 dispatch.
  // Once legacy material maps are retired, use the versioned converter's
  // public fromFile() API directly.
  const auto document = Acts::detail::readJsonFile(path);
  if (!document.contains("header") && !document.contains("$schema") &&
      (document.contains("Surfaces") || document.contains("Volumes"))) {
    return readLegacyMaterialMapForMigration(document, path, level);
  }
  return Acts::TrackingGeometryMaterialJsonConverter{}.fromJson(document);
}
