# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsMaterial )

# External dependencies:
find_package( Geant4 )
find_package( Acts COMPONENTS Core PluginRoot)

# Component(s) in the package:
atlas_add_library( ActsMaterial
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} AthenaBaseComps GaudiKernel G4AtlasToolsLib ActsGeometryInterfacesLib ActsEventLib ActsGeometryLib
                   Acts::Core Acts::PluginRoot
                   )
set_target_properties( ActsMaterial PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_scripts( scripts/*.py test/*.sh)

# Test(s) in the package:
atlas_add_test( MaterialStepWriter
                SCRIPT RunGeantinoMaterialTrackProduction.py --maxEvents 1
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                # Ignore spurious FPEs originating from inside Geant (G4Tubs)
                # in the clang build (due to Geant not being compiled
                # with trapping FP).
                LOG_IGNORE_PATTERN "FPE .*G4AtlasAlg"
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( MaterialStepWriterToroid
                SCRIPT testMaterialWriteToroid.sh
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                # Ignore spurious FPEs originating from inside Geant (G4Tubs)
                # in the clang build (due to Geant not being compiled
                # with trapping FP).
                LOG_IGNORE_PATTERN "FPE .*G4AtlasAlg"
                POST_EXEC_SCRIPT noerror.sh )






