# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetRawData )

find_library(ATLAS_ATOMIC_LIB NAMES atomic
   HINTS ENV LD_LIBRARY_PATH
   DOC "Path to the atomic library to use during the build")
mark_as_advanced(ATLAS_ATOMIC_LIB)
set(ATLAS_ATOMIC_LIBS)
if (ATLAS_ATOMIC_LIB)
   set(ATLAS_ATOMIC_LIBS ${ATLAS_ATOMIC_LIB})
endif()

# Component(s) in the package:
atlas_add_library( InDetRawData
                   src/*.cxx
                   PUBLIC_HEADERS InDetRawData
                   LINK_LIBRARIES ${ATLAS_ATOMIC_LIBS} AthenaKernel AthContainers AthLinks Identifier EventContainers
                   PRIVATE_LINK_LIBRARIES GaudiKernel )

atlas_add_dictionary( InDetRawDataDict
                      InDetRawData/InDetRawDataCLASS_DEF.h
                      InDetRawData/selection.xml
                      LINK_LIBRARIES InDetRawData )

atlas_add_test( PhaseIIInDetRawDataContainer_test
   SOURCES test/PhaseIIInDetRawDataContainer_test.cxx
   LINK_LIBRARIES InDetRawData
   POST_EXEC_SCRIPT nopost.sh )
