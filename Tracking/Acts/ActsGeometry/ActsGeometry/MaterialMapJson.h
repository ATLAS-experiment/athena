/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_MATERIALMAPJSON_H
#define ACTSGEOMETRY_MATERIALMAPJSON_H

#include "Acts/Material/TrackingGeometryMaterial.hpp"
#include "Acts/Utilities/Logger.hpp"

#include <filesystem>

namespace ActsTrk {
/// Load versioned material assignments without applying them to geometry.
/// JSON/CBOR and optional zstd compression are detected from the file contents.
/// Legacy material maps are accepted temporarily, with a migration warning.
/// Invalid versioned documents are rejected without falling back to legacy parsing.
Acts::TrackingGeometryMaterial readMaterialMapJson(
    const std::filesystem::path& path,
    Acts::Logging::Level level = Acts::Logging::INFO);
}

#endif
