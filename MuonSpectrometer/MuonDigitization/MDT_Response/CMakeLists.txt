# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MDT_Response )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS RIO Core Tree MathCore Hist pthread )

# Component(s) in the package:
atlas_add_library( MDT_Response
                    src/Amplifier.cxx
                    src/MDT_Response.cxx
                    PUBLIC_HEADERS MDT_Response
                    LINK_LIBRARIES
                    CLHEP::CLHEP
                    PRIVATE_LINK_LIBRARIES
                    ROOT::Core ROOT::RIO ROOT::Tree ROOT::MathCore ROOT::Hist
                   )

atlas_add_executable( MDT_ResponseTest
                    src/MDT_ResponseTest.cxx
                    LINK_LIBRARIES
                    MDT_Response
                    CxxUtils           
                    CLHEP::CLHEP
                    ROOT::Core ROOT::RIO ROOT::Tree ROOT::MathCore ROOT::Hist 
)
