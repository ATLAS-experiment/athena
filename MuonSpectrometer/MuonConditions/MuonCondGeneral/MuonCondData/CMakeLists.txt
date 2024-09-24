################################################################################
# Package: MuonCondData
################################################################################


# External dependencies:
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelHelpers )
# Declare the package name:
atlas_subdir( MuonCondData )

# Component(s) in the package:
atlas_add_library( MuonCondData
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondData
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel AthenaPoolUtilities
                   PRIVATE_LINK_LIBRARIES EventInfo MuonReadoutGeometry MuonNSWCommonDecode 
                                          MuonIdHelpersLib ${GEOMODEL_LIBRARIES} ${CLHEP_LIBRARIES})

