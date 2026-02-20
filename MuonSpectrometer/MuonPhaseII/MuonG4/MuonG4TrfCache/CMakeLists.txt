################################################################################
# Package: MuonG4TrfCache
################################################################################

# Declare the package name:
atlas_subdir( MuonG4TrfCache )

# Component(s) in the package:
atlas_add_component( MuonG4TrfCache
                     src/components/*.cxx src/*.cxx               
                     PRIVATE_LINK_LIBRARIES AthenaBaseComps ActsGeometryInterfacesLib
                                            MuonReadoutGeometryR4 )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

