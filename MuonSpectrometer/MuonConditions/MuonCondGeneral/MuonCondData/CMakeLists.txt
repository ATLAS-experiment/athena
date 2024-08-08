################################################################################
# Package: MuonCondData
################################################################################


# External dependencies:
find_package( CLHEP )
# Declare the package name:
atlas_subdir( MuonCondData )

# Component(s) in the package:
atlas_add_library( MuonCondData
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondData
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel 
                                   AthenaPoolUtilities MuonIdHelpersLib MuonTesterTreeLib 
                   PRIVATE_LINK_LIBRARIES EventInfo MuonReadoutGeometry MuonNSWCommonDecode )

