/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HDF5Utils/Merger.h"
#include "H5Cpp.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
  // Create a group chain (e.g. "a/b/c") under the file's root and write a
  // chunked, extensible 1D int dataset at the end of it.
  void writeNestedDataset(
      H5::H5File& file,
      const std::string& groupChain,
      const std::string& dsName,
      int firstValue,
      hsize_t nRows)
  {
    H5::Group cur = file.openGroup("/");
    std::istringstream chain(groupChain);
    std::string part;
    while (std::getline(chain, part, '/'))
      cur = cur.createGroup(part);

    hsize_t dims[1] = {nRows};
    hsize_t maxDims[1] = {H5S_UNLIMITED};
    H5::DataSpace space(1, dims, maxDims);
    H5::DSetCreatPropList props;
    hsize_t chunk[1] = {std::max<hsize_t>(nRows, 1)};
    props.setChunk(1, chunk);
    H5::DataSet ds = cur.createDataSet(
        dsName, H5::PredType::NATIVE_INT, space, props);

    std::vector<int> data(nRows);
    for (hsize_t i = 0; i < nRows; ++i) data.at(i) = firstValue + int(i);
    ds.write(data.data(), H5::PredType::NATIVE_INT);
  }

  hsize_t readExtent(H5::H5File& file, const std::string& dsPath)
  {
    H5::DataSet ds = file.openDataSet(dsPath);
    hsize_t dims[1];
    ds.getSpace().getSimpleExtentDims(dims);
    return dims[0];
  }
}

// Regression test for a bug where merging a source group into a target that
// doesn't yet have a matching child group would merge that child twice: once
// while creating it, once more right after. Every dataset under a
// newly-created group ended up with its rows duplicated.
int main() {
  const std::string groupChain = "a/b/c";
  const std::string dsPath = "/a/b/c/dataset";
  const hsize_t rows = 5;

  {
    H5::H5File src("merge_test_src1.h5", H5F_ACC_TRUNC);
    writeNestedDataset(src, groupChain, "dataset", 0, rows);
  }
  {
    H5::H5File src("merge_test_src2.h5", H5F_ACC_TRUNC);
    writeNestedDataset(src, groupChain, "dataset", 100, rows);
  }

  H5::H5File target("merge_test_out.h5", H5F_ACC_TRUNC);
  H5Utils::Merger merger;

  // Merging into a target with none of "a/b/c" yet: this is the path that
  // recursively creates new groups, and used to trigger the double merge.
  {
    H5::H5File src1("merge_test_src1.h5", H5F_ACC_RDONLY);
    merger.merge(target, src1);
  }
  hsize_t afterFirst = readExtent(target, dsPath);
  if (afterFirst != rows) {
    std::cerr << "FAIL: merging into a brand-new nested group produced "
              << afterFirst << " rows, expected " << rows
              << " (double-merge bug?)" << std::endl;
    return 1;
  }

  // Merging a second source into the now-existing "a/b/c" group: this is
  // the ordinary "found" path and should still merge exactly once.
  {
    H5::H5File src2("merge_test_src2.h5", H5F_ACC_RDONLY);
    merger.merge(target, src2);
  }
  hsize_t afterSecond = readExtent(target, dsPath);
  if (afterSecond != 2 * rows) {
    std::cerr << "FAIL: merging into an existing nested group produced "
              << afterSecond << " rows, expected " << 2 * rows << std::endl;
    return 1;
  }

  std::cout << "test-hdf5-merge: OK" << std::endl;
  return 0;
}
