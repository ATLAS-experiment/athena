# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FileCatalog )

# Component(s) in the package:
link_directories( ${GAUDI_LIBRARY_DIR} )
atlas_add_library( FileCatalog
                   src/*.cpp
                   PUBLIC_HEADERS FileCatalog
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel POOLCore
                   PRIVATE_LINK_LIBRARIES PersistentDataModel )


#atlas_add_executable( FCdeleteEntry
#                      utilities/FCdeleteEntry.cpp
#                      LINK_LIBRARIES FileCatalog )
#
#atlas_add_executable( FCdeletePFN
#                      utilities/FCdeletePFN.cpp
#                      LINK_LIBRARIES FileCatalog )
#
#atlas_add_executable( FClistGUID
#                      utilities/FClistGUID.cpp
#                      LINK_LIBRARIES FileCatalog )
#
#atlas_add_executable( FClistLFN
#                      utilities/FClistLFN.cpp
#                      LINK_LIBRARIES FileCatalog )
#
#atlas_add_executable( FClistPFN
#                      utilities/FClistPFN.cpp
#                      LINK_LIBRARIES FileCatalog )
#
#atlas_add_executable( FCregisterLFN
#                      utilities/FCregisterLFN.cpp
#                      LINK_LIBRARIES FileCatalog )
#
#atlas_add_executable( FCregisterPFN
#                      utilities/FCregisterPFN.cpp
#                      LINK_LIBRARIES FileCatalog )

#atlas_add_executable( FCrenamePFN
#                      utilities/FCrenamePFN.cpp
#                      LINK_LIBRARIES FileCatalog )

