################################################################################
# Package: TGC_Digitization
################################################################################

# Declare the package name:
atlas_subdir( TGC_Digitization )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( TGC_Digitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel PileUpToolsLib GeoPrimitives Identifier
                                    GaudiKernel MuonSimEvent HitManagement xAODEventInfo GeneratorObjects MuonCondData
                                    MuonReadoutGeometry MuonDigitContainer MuonIdHelpersLib MuonSimData PathResolver RDBAccessSvcLib )

atlas_install_runtime( share/*.dat )
