# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigDFEmulator )

# External dependencies:
find_package( Boost )
find_package( tdaq-common COMPONENTS eformat eformat_write DataReader DataWriter RawFileName EventStorageRecords)

# Component(s) in the package:
atlas_add_component( TrigDFEmulator 
                     src/*.h src/*.cxx 
                     INCLUDE_DIRS
                     ${TDAQ-COMMON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}  
                     LINK_LIBRARIES
                     ${TDAQ-COMMON_LIBRARIES}  
                     TrigKernel
                     )
