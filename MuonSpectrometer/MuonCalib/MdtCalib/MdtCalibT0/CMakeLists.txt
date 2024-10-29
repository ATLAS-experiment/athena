# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MdtCalibT0 )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Graf Hist MathCore MathMore Minuit RIO Tree )

# Component(s) in the package:
atlas_add_library( MdtCalibT0
                   src/*.cxx
                   PUBLIC_HEADERS MdtCalibT0
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaKernel GaudiKernel MdtCalibData MdtCalibInterfacesLib MuonCalibStandAloneBaseLib
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils MuonCalibEventBase MuonCalibIdentifier )

atlas_add_dictionary( MdtCalibT0Dict
                      MdtCalibT0/ReflexHeaders.h
                      MdtCalibT0/selection.xml
                      LINK_LIBRARIES MdtCalibT0 )
