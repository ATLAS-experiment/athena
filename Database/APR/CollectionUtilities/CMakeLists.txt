# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CollectionUtilities )

# External dependencies:
find_package( Boost )
find_package( CORAL COMPONENTS CoralBase )
find_package( XercesC )

# Helper macro declaring the utilities of the package:
macro( collection_utility name )
  atlas_add_executable( ${name} utilities/${name}.cpp
    INCLUDE_DIRS $${CORAL_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS}
    LINK_LIBRARIES ${CORAL_LIBRARIES} ${XERCESC_LIBRARIES} CollectionBase
    PersistentDataModel PersistencySvc FileCatalog POOLCore )
endmacro( collection_utility )

# Declare the utilities of the package:
collection_utility( coll_insertGuidToCatalog )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
