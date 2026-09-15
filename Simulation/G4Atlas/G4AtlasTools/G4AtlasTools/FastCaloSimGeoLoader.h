/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_FASTCALOSIMGEOLOADER_H
#define G4ATLASTOOLS_FASTCALOSIMGEOLOADER_H

#include <memory>
#include <string>

class CaloGeo;

/// Shared helpers for loading the external FastCaloSim geometry.
namespace FastCaloSimGeo {

/// Load CaloGeo and its FCal handler from the geometry files.
std::unique_ptr<CaloGeo> loadCaloGeo(const std::string& geoFileBasePath,
                                     const std::string& geoTag,
                                     std::size_t rtreeCacheSize = 5 * 1024 * 1024);

}  // namespace FastCaloSimGeo

#endif  // G4ATLASTOOLS_FASTCALOSIMGEOLOADER_H
