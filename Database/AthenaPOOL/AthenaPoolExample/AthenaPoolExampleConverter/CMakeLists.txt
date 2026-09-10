# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaPoolExampleConverter )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( AthenaPoolExampleConverter
                   AthenaPoolExampleConverter/*.h
                   INTERFACE
                   PUBLIC_HEADERS AthenaPoolExampleConverter
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} CxxUtils DataModelAthenaPoolLib )

atlas_add_poolcnv_library( AthenaPoolExampleConverterPoolCnv
			   FILES AthenaPoolExampleData/ExampleHitContainer.h AthenaPoolExampleData/ExampleTrackContainer.h AthenaPoolExampleData/ExampleElectron.h AthenaPoolExampleData/ExampleElectronContainer.h AthenaPoolExampleData/ExampleElectronAuxContainer.h
			   TYPES_WITH_NAMESPACE xAOD::ExampleElectron xAOD::ExampleElectronContainer xAOD::ExampleElectronAuxContainer
			   LINK_LIBRARIES AthenaPoolCnvSvcLib AthenaPoolExampleConverter AthenaPoolExampleData )

atlas_add_dictionary( AthenaPoolExampleConverterDict
                      AthenaPoolExampleConverter/AthenaPoolExampleConverterDict.h
                      AthenaPoolExampleConverter/selection.xml
                      LINK_LIBRARIES AthenaPoolExampleConverter )
