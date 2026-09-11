# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExThinning )

# Component(s) in the package:
atlas_add_library( AthExThinningEvent
                   src_lib/AthExIParticle.cxx
                   src_lib/AthExParticle.cxx
                   src_lib/AthExParticles.cxx
                   src_lib/AthExElephantino.cxx
                   src_lib/AthExDecay.cxx
                   src_lib/AthExFatObject.cxx
                   src_lib/AthExElephantino_p1.cxx
                   src_lib/AthExDecay_p1.cxx
                   src_lib/AthExParticles_p1.cxx
                   src_lib/AthExFatObject_p1.cxx
                   PUBLIC_HEADERS AthExThinning
                   LINK_LIBRARIES AthContainers AthLinks AthenaBaseComps AthenaKernel DataModelAthenaPoolLib GaudiKernel StoreGateLib )

atlas_add_component( AthExThinningAlgs
                     src_lib/CreateData.cxx
                     src_lib/WriteThinnedData.cxx
                     src_lib/ReadThinnedData.cxx
                     src_lib/components/*.cxx
                     LINK_LIBRARIES AthExThinningEvent )

atlas_add_poolcnv_library( AthExThinningPoolCnv
                           src/*.cxx
                           FILES AthExThinning/AthExParticles.h AthExThinning/AthExIParticles.h AthExThinning/AthExDecay.h AthExThinning/AthExElephantino.h AthExThinning/AthExFatObject.h
                           LINK_LIBRARIES AthExThinningEvent AthenaPoolCnvSvcLib )

atlas_add_dictionary( AthExThinningEventDict
                      AthExThinning/AthExThinningEventDict.h
                      AthExThinning/selection.xml
                      LINK_LIBRARIES AthExThinningEvent
                      ELEMENT_LINKS AthExParticles AthExIParticles )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
atlas_add_test( AthExThinning_makeData
                SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/AthExThinning_makeData.py
                LOG_SELECT_PATTERN "^CreateData" )

atlas_add_test( WriteThinnedData
                SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/WriteThinnedData.py Exec.AlgMode="cpp" \"Input.Files=['../unitTestRun/myToThin.pool.root']\"
                DEPENDS AthExThinning_makeData
                PRIVATE_WORKING_DIRECTORY
                LOG_SELECT_PATTERN "^WriteThinnedData" )

atlas_add_test( pyWriteThinnedData
                SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/WriteThinnedData.py Exec.AlgMode="py" \"Input.Files=['../unitTestRun/myToThin.pool.root']\"
                DEPENDS AthExThinning_makeData
                PRIVATE_WORKING_DIRECTORY
                LOG_SELECT_PATTERN "^Py:WriteThinnedData" )

# Add two Reader tests for each of the two write modes
foreach( prefix "" "py")
   atlas_add_test( ${prefix}ReadThinnedData
      SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/ReadData.py \"Input.Files=['../unitTestRun_${prefix}WriteThinnedData/myUSR_0.pool.root']\"
      PRIVATE_WORKING_DIRECTORY
      DEPENDS ${prefix}WriteThinnedData
      LOG_SELECT_PATTERN "^ReadThinnedData" )

   atlas_add_test( ${prefix}ReadNonThinnedData
      SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/ReadData.py Input.Mode="NonThinned" \"Input.Files=['../unitTestRun_${prefix}WriteThinnedData/myUSR_1.pool.root']\"
      PRIVATE_WORKING_DIRECTORY
      DEPENDS ${prefix}WriteThinnedData
      LOG_SELECT_PATTERN "^Py:ReadNonThinnedData" )
endforeach()
