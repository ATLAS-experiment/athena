atlas_subdir( HGTD_AlignGenAlgs )

find_package( CLHEP )

# Component library
atlas_add_component( HGTD_AlignGenAlgs
    src/*.cxx
    src/components/*.cxx
    INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
    LINK_LIBRARIES
        ${CLHEP_LIBRARIES}
        AthenaBaseComps
        GaudiKernel
        GeoPrimitives
        GeoModelInterfaces
        GeoModelUtilities
        HGTD_ReadoutGeometry
        HGTD_AlignGenToolsLib
)

atlas_install_python_modules( python/*.py )
atlas_install_scripts( share/*.py )