################################################################################
# Package: MDT_Digitization
################################################################################

# Declare the package name:
atlas_subdir( MDT_Digitization )

# External dependencies:
find_package( CLHEP )


atlas_add_library ( MDT_DigitizationLib
                    MDT_Digitization/*.h
                    INTERFACE
                    PUBLIC_HEADERS MDT_Digitization
                    INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                    LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel GeoModelUtilities GeoPrimitives 
                                   Identifier TrkEventPrimitives MuonSimEvent MdtCalibData MuonReadoutGeometry PathResolver)


# Component(s) in the package:
atlas_add_component( MDT_Digitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES}  MDT_DigitizationLib AtlasHepMCLib AthenaBaseComps AthenaKernel PileUpToolsLib StoreGateLib 
                                     Identifier xAODEventInfo GaudiKernel GeneratorObjects MdtCalibData MdtCalibInterfacesLib MuonCondInterface 
                                     MuonCondData MuonReadoutGeometry MuonDigitContainer MDT_Response MuonIdHelpersLib MuonSimData 
                                     MuonSimEvent HitManagement PathResolver TrkDetDescrUtils EventPrimitives )

