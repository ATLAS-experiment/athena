################################################################################
# Package: FCALGeoPlugin
################################################################################

# Declare the package name:
atlas_subdir( FCALGeoPlugin )

# Component(s) in the package:
atlas_add_library( FCALGeoPlugin
                   src/*.cxx
                   PUBLIC_HEADERS FCALGeoPlugin
                   LINK_LIBRARIES CxxUtils
                   )
