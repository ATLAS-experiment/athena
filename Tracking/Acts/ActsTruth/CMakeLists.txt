# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTruth )

# External dependencies:
find_package( Acts COMPONENTS Core )

# extclude src/TruthTrajectory.cxx from ActsTruth since it is needed in ActsTruthLib
file(GLOB COMPONENT_SRC src/*.cxx)
list(FILTER COMPONENT_SRC EXCLUDE REGEX "src/TruthTrajectory.cxx")

atlas_add_library(ActsTruthLib
                  ActsTruth/*.h ActsTruth/*.icc src/TruthTrajectory.cxx
                  INTERFACE
                  PUBLIC_HEADERS ActsTruth
                  LINK_LIBRARIES ActsInteropLib )


atlas_add_component( ActsTruth
                     ${COMPONENT_SRC}
                     src/components/*.cxx
                     LINK_LIBRARIES
                        ActsEventLib
                        Acts::Core
                        ActsTruthLib
                        ActsGeometryLib
                        ActsGeometryInterfacesLib
                        ActsCalibratorsLib
                        AthenaBaseComps
                        CxxUtils
                        GaudiKernel
                        GeneratorObjects
                        GeneratorModulesLib
                        Identifier
                        InDetSimData
                        MCTruthClassifierLib
                        StoreGateLib
                        TrkTruthTrackInterfaces
                        xAODInDetMeasurement
                        xAODMeasurementBase
                        xAODTruth
                        xAODMuonViews
                        TruthUtils)
                
