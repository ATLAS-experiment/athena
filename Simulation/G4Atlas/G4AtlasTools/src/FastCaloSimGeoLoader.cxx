/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4AtlasTools/FastCaloSimGeoLoader.h"

#include <array>
#include <atomic>
#include <filesystem>
#include <stdexcept>

#include <unistd.h>  // getpid

#include <ROOT/RDataFrame.hxx>

#include "FCALGeoPlugin/FCAL.h"
#include "FastCaloSim/Geometry/CaloGeo.h"

namespace FastCaloSimGeo {

std::unique_ptr<CaloGeo> loadCaloGeo(const std::string& geoFileBasePath,
                                     const std::string& geoTag,
                                     std::size_t rtreeCacheSize)
{
  // Fail with a clear message rather than letting RDataFrame throw a generic
  // one (typical cause: geometry tag with no matching folder in the calib area)
  const std::string cellsFile =
      geoFileBasePath + "/" + geoTag + "/CaloCells.root";
  if (!std::filesystem::exists(cellsFile)) {
    throw std::runtime_error(
        "FastCaloSimGeo::loadCaloGeo: geometry file not found: " + cellsFile);
  }

  // Read the calorimeter cell geometry
  ROOT::RDataFrame df("caloDetCells", cellsFile);

  auto geo = std::make_unique<CaloGeo>();

  // Scratch dir for the RTree/cell-store files. These back an mmap held for
  // the geometry's lifetime. The per-call counter keeps multiple CaloGeo
  // instances in one process (e.g. several tool instances) from overwriting
  // each other's still-mapped files.
  static std::atomic<unsigned int> geoInstanceCount{0};
  const std::filesystem::path tmp_dir_path =
      std::filesystem::temp_directory_path()
      / ("fcs_geo_" + std::to_string(::getpid()) + "_"
         + std::to_string(geoInstanceCount++));
  std::filesystem::create_directories(tmp_dir_path);

  // Build the geometry and write it to disk, then load it back
  geo->build(df, tmp_dir_path.string());
  geo->load(tmp_dir_path.string(), rtreeCacheSize);

  // Create the alternative geometry handler for the FCal
  auto fcal_geo = std::make_shared<FCALGeo::FCal>();

  const std::array<std::string, 3> FCAL_ELECTRODE_FILES = {
      geoFileBasePath + "/FCal1-electrodes.sorted.HV.09Nov2007.dat",
      geoFileBasePath + "/FCal2-electrodes.sorted.HV.April2011.dat",
      geoFileBasePath + "/FCal3-electrodes.sorted.HV.09Nov2007.dat"};

  // Load the FCal geometry from the files
  fcal_geo->load(FCAL_ELECTRODE_FILES);
  fcal_geo->set_geo(geo.get());

  // Set the alternative geometry handler for the FCal layers (21 - 23)
  geo->set_alt_geo_handler(21, 23, fcal_geo);

  return geo;
}

}  // namespace FastCaloSimGeo
