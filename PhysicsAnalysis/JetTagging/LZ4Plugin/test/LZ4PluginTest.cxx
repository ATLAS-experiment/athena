/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Test that addLz4ToPlist registers and applies the LZ4 filter without
// requiring HDF5_PLUGIN_PATH to be set.

#include "LZ4Plugin/addLz4ToPlist.h"
#include "H5Cpp.h"
#include "CxxUtils/checker_macros.h"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Build a file-access property list that keeps all HDF5 data in RAM.
// H5P_FILE_ACCESS is the property class for file-access settings.
// H5Pset_fapl_core switches the driver to the "core" (in-memory) driver:
//   increment - how much to grow the memory buffer when it fills (1 MiB)
//   backing   - 0: discard on close; no file is written to disk
// The raw hid_t is guarded by a unique_ptr so H5Pclose always runs.
// H5::FileAccPropList copies the id internally, so the raw handle can be
// closed immediately after the C++ object is constructed.
H5::FileAccPropList makeInMemoryFapl ATLAS_NOT_THREAD_SAFE () {
  hid_t raw = -1;
  auto guard = std::unique_ptr<hid_t, void(*)(hid_t*)>(
    &raw, [](hid_t* p) { if (*p >= 0) H5Pclose(*p); });
  raw = H5Pcreate(H5P_FILE_ACCESS);
  if (raw < 0) throw std::runtime_error("H5Pcreate failed");
  if (H5Pset_fapl_core(raw, /*increment=*/1 << 20, /*backing=*/0) < 0) {
    throw std::runtime_error("H5Pset_fapl_core failed");
  }
  return H5::FileAccPropList(raw);
}

// Marked not-thread-safe because it calls makeInMemoryFapl, which is.
void run_tests ATLAS_NOT_THREAD_SAFE () {
  // Open an in-memory HDF5 file so the test leaves no disk artefacts.
  H5::FileAccPropList fapl_cpp = makeInMemoryFapl();
  H5::H5File file("LZ4PluginTest.h5", H5F_ACC_TRUNC,
                  H5::FileCreatPropList(), fapl_cpp);

  // Build a creation property list with LZ4 compression.
  H5::DSetCreatPropList plist;
  hsize_t chunk = 4096;
  plist.setChunk(1, &chunk);
  addLz4ToPlist(plist);  // must not throw

  // Repetitive and large enough that LZ4 really shrinks it. A payload that
  // doesn't compress takes the memcpy fallback in both directions, so the
  // compression path would never be exercised.
  std::vector<float> written(4096);
  for (std::size_t i = 0; i < written.size(); ++i) {
    written[i] = static_cast<float>(i % 8);
  }
  hsize_t dims = written.size();
  H5::DataSpace space(1, &dims);
  H5::DataSet ds = file.createDataSet(
    "data", H5::PredType::NATIVE_FLOAT, space, plist);
  ds.write(written.data(), H5::PredType::NATIVE_FLOAT);

  // Fail loudly if the data went in uncompressed: the round trip below
  // passes either way, so without this the test can silently stop covering
  // the thing it exists to cover.
  const hsize_t raw_bytes = written.size() * sizeof(float);
  const hsize_t stored_bytes = ds.getStorageSize();
  if (stored_bytes >= raw_bytes) {
    throw std::runtime_error(
      "LZ4 filter did not compress: stored " + std::to_string(stored_bytes)
      + " bytes for " + std::to_string(raw_bytes) + " bytes of data");
  }

  // Reopen the dataset before reading. A read through the handle we just
  // wrote is served from the chunk cache and never reaches the filter.
  ds.close();
  H5::DataSet reopened = file.openDataSet("data");

  // Read back and verify equality.
  std::vector<float> readback(written.size());
  reopened.read(readback.data(), H5::PredType::NATIVE_FLOAT);
  for (std::size_t i = 0; i < written.size(); ++i) {
    if (readback[i] != written[i]) {
      throw std::runtime_error("LZ4 round-trip mismatch at index "
                               + std::to_string(i));
    }
  }
}

int main ATLAS_NOT_THREAD_SAFE () {
  try {
    run_tests();
  } catch (const H5::Exception& e) {
    // H5::Exception does not derive from std::exception, so without this
    // an HDF5 failure aborts the process instead of failing cleanly.
    std::fprintf(stderr, "error: HDF5: %s\n", e.getDetailMsg().c_str());
    return 1;
  } catch (const std::exception& e) {
    std::fprintf(stderr, "error: %s\n", e.what());
    return 1;
  }
  return 0;
}
