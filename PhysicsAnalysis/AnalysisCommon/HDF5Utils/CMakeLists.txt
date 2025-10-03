# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the project's name and version.
atlas_subdir( HDF5Utils )

# Grab HDF5 from AnalysisBaseExternals.
find_package( HDF5 1.10.1 COMPONENTS CXX HL)

# find root
find_package(ROOT COMPONENTS RIO Hist Tree Net Core TreePlayer)

# find boost
find_package( Boost 1.54.0 COMPONENTS program_options)

# Add the hdf tuple library
atlas_add_library(HDF5Utils
  Root/HdfTuple.cxx Root/common.cxx Root/H5Traits.cxx Root/CompressedTypes.cxx
  Root/DefaultMerger.cxx
  Root/H5Print.cxx
  Root/IH5Merger.cxx
  Root/MergeUtils.cxx
  PUBLIC_HEADERS HDF5Utils
  DEFINITIONS ${HDF5_CXX_DEFINITIONS}
  INCLUDE_DIRS ${HDF5_INCLUDE_DIRS}
  LINK_LIBRARIES ${HDF5_LIBRARIES})

# build a translation utility
set( _exe_sources
  util/copyRootTree.cxx
  util/getTree.cxx
  util/treeCopyOpts.cxx
  util/ttree2hdf5.cxx)

atlas_add_executable(ttree2hdf5 ${_exe_sources}
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} util ${Boost_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS}
  LINK_LIBRARIES HDF5Utils ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${HDF5_LIBRARIES} )

unset(_exe_sources)

# add the merge utility
atlas_add_executable( hdf5-merge
  util/hdf5-merge.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS}
  LINK_LIBRARIES HDF5Utils ${Boost_LIBRARIES} ${HDF5_LIBRARIES} )

atlas_install_scripts( bin/hdf5-merge-nolock )

# Services only make sense in gaudi

# we only build the algorithm stuff for Gaudi builds
if (NOT XAOD_STANDALONE)
  # interface library
  atlas_add_library(IH5GroupSvc
    INTERFACE
    PUBLIC_HEADERS HDF5Utils
    LINK_LIBRARIES GaudiKernel AthenaBaseComps
    )
  atlas_add_component(H5FileSvc
    src/H5FileSvc.cxx
    src/components/HDF5Utils_entries.cxx
    INCLUDE_DIRS ${HDF5_INCLUDE_DIRS}
    LINK_LIBRARIES IH5GroupSvc ${HDF5_LIBRARIES}
  )
endif()

# unit tests

atlas_add_executable( test-hdf5-writer
  util/test-hdf5-writer.cxx
  LINK_LIBRARIES HDF5Utils )
atlas_install_scripts( test/test-h5-output )

atlas_add_executable( test-histogram
  util/test-histogram.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES HDF5Utils ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
atlas_install_scripts( test/test-histogram-output )

atlas_add_test( test_hdf5_writer
  SCRIPT test-hdf5-writer
  POST_EXEC_SCRIPT test-h5-output)

atlas_add_test( test_histogram
  SCRIPT test-histogram
  POST_EXEC_SCRIPT test-histogram-output)


# half precision tests
atlas_add_executable( test-half-precision
  util/test-half-precision.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES HDF5Utils ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

