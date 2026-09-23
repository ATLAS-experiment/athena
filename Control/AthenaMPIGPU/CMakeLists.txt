# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir(AthenaMPIGPU)

find_package(bmpi3 REQUIRED)
find_package(TBB REQUIRED)

atlas_add_library(AthenaMPIGPULib
   src/RemoteGPUSvc.cxx
   PUBLIC_HEADERS AthenaMPIGPU
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${TBB_LIBRARIES} bmpi3::bmpi3 AthenaBaseComps
                  AthenaKernel CxxUtils GaudiKernel)

atlas_add_component(AthenaMPIGPU
   src/components/*.cxx
   LINK_LIBRARIES AthenaMPIGPULib)

atlas_add_test(RPCFunctionEntry_test
   SOURCES test/RPCFunctionEntry_test.cxx
   LINK_LIBRARIES AthenaMPIGPULib
   POST_EXEC_SCRIPT noerror.sh)
