# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_Analysis )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Physics Graf Gpad )

# Component(s) in the package:
atlas_add_library( HGTD_AnalysisLib
                   src/*.cxx
                   PUBLIC_HEADERS HGTD_Analysis
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel xAODTracking xAODTruth
                   ${ROOT_LIBRARIES} )

atlas_add_component( HGTD_Analysis
                     src/components/*.cxx
                     LINK_LIBRARIES HGTD_AnalysisLib GaudiKernel )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_joboptions( share/*.py )

# Plotting executables of the validation chain. They take a TEnv config file
# and turn the histograms written by HGTD_TrkTimePerformanceStudies into the
# HGTD track-time validation plots. See util/README.md.
atlas_add_executable( HGTD_effCurvesVersionsEta
                      util/effCurvesVersionsEta.cxx
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils )

atlas_add_executable( HGTD_timeResSplitCases
                      util/timeResSplitCases.cxx
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils )
