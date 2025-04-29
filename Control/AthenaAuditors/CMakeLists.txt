# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaAuditors )

# External dependencies:
find_package( gdb COMPONENTS bfd iberty sframe )
find_package( ZLIB )

# Component(s) in the package:
atlas_add_component( AthenaAuditors
   src/*.h src/*.cxx src/components/*.cxx
   DEFINITIONS PACKAGE=AthenaAuditors  # needed by bfd.h
   INCLUDE_DIRS ${GDB_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS}
   LINK_LIBRARIES ${GDB_LIBRARIES} ${ZLIB_LIBRARIES} AthenaBaseComps CxxUtils GaudiKernel )
