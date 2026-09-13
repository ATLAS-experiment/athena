# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( ActsGPUEvent )

# External dependencies.
find_package(traccc)
find_package(Boost COMPONENTS unit_test_framework)

# Component(s) in the package.
atlas_add_library(ActsGPUEventLib
   ActsGPUEvent/*.h src/*.cxx
   PUBLIC_HEADERS ActsGPUEvent
   LINK_LIBRARIES traccc::core AthenaKernel)

atlas_generate_cliddb( ActsGPUEventLib )

# Install files from the package.
atlas_install_joboptions(share/*.txt)

# Test(s) in the package.
function(_add_test name)
   atlas_add_test(${name}
      SOURCES "test/${name}.cxx"
      LINK_LIBRARIES Boost::unit_test_framework CxxUtils StoreGateLib
                     TestTools ActsGPUEventLib
      POST_EXEC_SCRIPT noerror.sh)
endfunction(_add_test)

_add_test(BufferToView_test)
_add_test(BufferToConstView_test)
_add_test(DataToView_test)
_add_test(DataToConstView_test)
_add_test(ConstDataToConstView_test)
_add_test(HostToConstData_test)
_add_test(HostToConstView_test)
_add_test(ViewToConstView_test)
